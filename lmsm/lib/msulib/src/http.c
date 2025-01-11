#define BT_IMPL
#include "msulib/http.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>

#define BT_NAME http_headers_tree
#define BT_KEY const msu_str_t *
#define BT_VALUE list_of_msu_strs_t *
#define BT_HASHFUNC(x) msu_str_hash(x, 42)
#define BT_EQFUNC msu_str_eq
#define BT_FREE_KEY msu_str_free
#define BT_FREE_VALUE(x) list_of_msu_strs_free(x, true)
#include "templates/btree.h"

const msu_str_t **kv_store_get_one(kv_store_t *me, const msu_str_t *str) {
    list_of_msu_strs_t **values = kv_store_getv(me, str);
    if (!values || (*values)->len != 1) return NULL;
    return list_of_msu_strs_get_ref(*values, 0);
}

typedef enum conn_state {
    CONN_STATE_INIT = 0,
    CONN_STATE_INFO = 1,
    CONN_STATE_HEADERS = 2,
    CONN_STATE_BODY = 3,
} conn_state_t;

struct httpheaders {
    http_headers_tree_t *table;
};

struct httpconn {
    bool done_reading;
    tcp_stream_t *stream;
    socket_addr_t *peer_addr;
    conn_state_t recv_state;
    http_res_t *response;

    msu_str_builder_t buffer;
    size_t buffer_offset;
};

const size_t MAX_BUF_SIZE = 64 * 1024 * 1024;

bool httpconn_read(http_conn_t *conn, http_error_t *error) {
    if (conn->done_reading) return false;
    const size_t bufsize = BUFSIZ;
    uint8_t buf[bufsize];
    tcp_error_t err;
    size_t read = tcpstream_read(conn->stream, buf, bufsize, &err);
    if (err) {
        *error = HTTP_ERROR_READ;
        return false;
    }
    if (conn->buffer->len + read >= MAX_BUF_SIZE) {
        *error = HTTP_ERROR_REQ_TOO_BIG;
        return false;
    }
    msu_str_builder_pushn(conn->buffer, buf, read);
    if (read < bufsize) {
        conn->done_reading = true;
    }
    return true;
}

bool httpconn_buffer_find(http_conn_t *conn, const char *s, size_t *idx) {
    size_t slen = strlen(s);
    if (0 == slen || slen > conn->buffer->len) {
        return false;
    }

    size_t i;
    for (i = conn->buffer_offset; i < conn->buffer->len - slen + 1; i++) {
        if (0 == strncmp(s, conn->buffer->src + i, slen)) {
            if (idx) *idx = i;
            return true;
        }
    }
    return false;
}

bool httpconn_read_until(http_conn_t *conn, size_t read_limit, const char *s, size_t *idx, http_error_t *err) {
    size_t start = conn->buffer_offset;
    while (!httpconn_buffer_find(conn, s, idx)) {
        if (!httpconn_read(conn, err)) {
            return false;
        }
        if (conn->buffer->len - start >= read_limit) {
            return false;
        }
    }
    return true;
}

http_info_t httpconn_recv_info(http_conn_t *conn, http_error_t *errout) {
    assert(conn->recv_state == CONN_STATE_INIT && "info already received");
    *errout = HTTP_ERROR_NONE;

    size_t start = conn->buffer_offset;
    size_t end;
    if (!httpconn_read_until(conn, 1024*1024, "\r\n", &end, errout)) {
        if (!*errout) *errout = HTTP_ERROR_BAD_REQUEST;
        conn->recv_state = CONN_STATE_INIT;
        return (http_info_t) {0};
    }

    const msu_str_t *infoline = msu_str_builder_substring(conn->buffer, start, end);
    conn->buffer_offset = end + 2;

    list_of_msu_strs_t *parts = msu_str_splitwhite(infoline);
    if (parts->len != 3) {
        *errout = HTTP_ERROR_BAD_REQUEST;
        list_of_msu_strs_free(parts, true);
        msu_str_free(infoline);
        return (http_info_t) {0};
    }

    const msu_str_t *method = list_of_msu_strs_get(parts, 0);
    const msu_str_t *url = list_of_msu_strs_get(parts, 1);
    const msu_str_t *version = list_of_msu_strs_get(parts, 2);

    if (!msu_str_eqs(version, "HTTP/1.1")) {
        *errout = HTTP_ERROR_BAD_VERSION;
        list_of_msu_strs_free(parts, true);
        msu_str_free(infoline);
        return (http_info_t){0};
    }

    http_info_t out;
    out.method = msu_str_clone(method);
    out.url = msu_str_clone(url);
    out.version = msu_str_clone(version);

    list_of_msu_strs_free(parts, true);
    msu_str_free(infoline);

    conn->recv_state = CONN_STATE_INFO;
    return out;
}

bool http_parse_header(const msu_str_t *header, http_header_t *out) {
    size_t colon_idx = 0;
    if (!msu_str_find_char(header, ':', &colon_idx)) {
        return false;
    }

    const msu_str_t *tmp;

    tmp = msu_str_substring(header, 0, colon_idx);
    const msu_str_t *name = msu_str_trim_ws(tmp);
    msu_str_free(tmp);
    tmp = name;
    name = msu_str_lower(tmp);
    msu_str_free(tmp);

    tmp = msu_str_substring(header, colon_idx + 1, msu_str_len(header));
    const msu_str_t *value = msu_str_trim_ws(tmp);
    msu_str_free(tmp);

    out->name = name;
    out->value = value;
    return true;
}

const msu_str_t *http_url_decode(const msu_str_t *str, http_error_t *err) {
    if (!str) return EMPTY_STRING;
    *err = HTTP_ERROR_NONE;

    msu_str_builder_t sb = msu_str_builder_new();
    for (size_t i = 0; i < msu_str_len(str); i++) {
        char c = msu_str_at(str, i);
        if (c != '%') {
            msu_str_builder_push(sb, c);
            continue;
        }

        if (i + 2 >= msu_str_len(str)) {
            *err = HTTP_ERROR_URL_DECODE;
            return NULL;
        }

        char s[3];
        memcpy(s, msu_str_data(str) + i + 1, sizeof(char) * 2);
        s[2] = 0;

        errno = 0;
        char *end;
        unsigned long value = strtoul(s, &end, 16);
        if (errno || *end != 0 || value > 255) {
            fprintf(stderr, "%d %s\n", errno, strerror(errno));
            *err = HTTP_ERROR_URL_DECODE;
            return false;
        }
        msu_str_builder_push(sb, (char) value);
        i += 2;
    }

    return msu_str_builder_into_string_and_free(sb);
}

kv_store_t *http_form_data_decode(const msu_str_t *str, http_error_t *err) {
    kv_store_t *out = kv_store_new();
    *err = HTTP_ERROR_NONE;

    list_of_msu_strs_t *pairs = msu_str_split(str, STRLIT("&"));
    for (size_t i = 0; i < pairs->len; i++) {
        const msu_str_t *pair = list_of_msu_strs_get(pairs, i);
        list_of_msu_strs_t *kv = msu_str_split(pair, STRLIT("="));
        if (kv->len != 2) {
            list_of_msu_strs_free(kv, true);
            *err = HTTP_ERROR_FORM_DATA_DECODE;
            goto defer;
        }

        const msu_str_t *name = http_url_decode(list_of_msu_strs_get(kv, 0), err);
        if (*err) {
            list_of_msu_strs_free(kv, true);
            goto defer;
        }

        const msu_str_t *value = http_url_decode(list_of_msu_strs_get(kv, 1), err);
        if (*err) {
            list_of_msu_strs_free(kv, true);
            goto defer;
        }

        kv_store_entry_t *entry = kv_store_get(out, name);
        if (!entry) {
            list_of_msu_strs_t *values = list_of_msu_strs_new();
            list_of_msu_strs_append(values, value);
            kv_store_insert(out, msu_str_clone(name), values);
        } else {
            list_of_msu_strs_t *values = kv_store_entry_value(entry);
            list_of_msu_strs_append(values, value);
        }

        msu_str_free(name); // value is always stored, name is always discarded
    }

defer:
    list_of_msu_strs_free(pairs, true);
    return out;
}

http_headers_t *httpconn_recv_headers(http_conn_t *conn, http_error_t *errout) {
    assert(conn->recv_state == CONN_STATE_INFO);
    *errout = HTTP_ERROR_NONE;
    const size_t max_header_size = 4 * 1024 * 1024;
    size_t start = conn->buffer_offset;

    http_headers_t *out = httpheaders_new();
    while (1) {
        size_t line_begin = conn->buffer_offset;
        size_t line_end;
        if (!httpconn_read_until(conn, -1, "\r\n", &line_end, errout)) {
            if (!*errout) *errout = HTTP_ERROR_BAD_REQUEST;
            httpheaders_free(out);
            return NULL;
        }

        if (line_end == line_begin) {
            conn->buffer_offset = line_end + 2;
            break; // found \r\n\r\n
        }

        if (conn->buffer->len - start >= max_header_size) {
            *errout = HTTP_ERROR_REQ_TOO_BIG;
            httpheaders_free(out);
            return NULL;
        }

        http_header_t header;
        const msu_str_t *field_line = msu_str_builder_substring(conn->buffer, line_begin, line_end);
        conn->buffer_offset = line_end + 2;
        if (!http_parse_header(field_line, &header)) {
            *errout = HTTP_ERROR_BAD_REQUEST;
            httpheaders_free(out);
            return NULL;
        }

        httpheaders_add(out, header.name, header.value);
        msu_str_free(header.name);
        msu_str_free(header.value);
        msu_str_free(field_line);
    }

    conn->recv_state = CONN_STATE_HEADERS;
    return out;
}

const msu_str_t *httpconn_recv_body_string(http_conn_t *conn, http_error_t *errout) {
    assert(conn->recv_state == CONN_STATE_HEADERS);
    *errout = HTTP_ERROR_NONE;

    const size_t max_body_size = 48 * 1024 * 1024;

    size_t start = conn->buffer_offset;
    while (httpconn_read(conn, errout)) {
        // hi
        if (conn->buffer->len - start >= max_body_size) {
            *errout = HTTP_ERROR_REQ_TOO_BIG;
            return NULL;
        }
    }

    conn->recv_state = CONN_STATE_BODY;
    return msu_str_builder_substring(conn->buffer, start, conn->buffer->len);
}

void httpcon_recv_request(http_conn_t *conn, http_req_t *out, http_error_t *errout) {
    assert(conn->recv_state == CONN_STATE_INIT);
    *errout = HTTP_ERROR_NONE;

    http_info_t info = httpconn_recv_info(conn, errout);
    if (*errout) return;

    http_headers_t *headers = httpconn_recv_headers(conn, errout);
    if (*errout) return;

    const msu_str_t *body = httpconn_recv_body_string(conn, errout);
    if (*errout) return;

    http_req_t req = {
        .info = info,
        .headers = headers,
        .body = body
    };
    memcpy(out, &req, sizeof(http_req_t));

    msu_str_builder_free(conn->buffer);
    conn->buffer = NULL;
    conn->buffer_offset = 0;
}

http_res_t *httpcon_make_response(http_conn_t *conn) {
    assert(conn->response == NULL);

    http_res_t *res = malloc(sizeof(http_res_t));
    assert(res && "out of memory!\n");
    res->headers = httpheaders_new();
    res->status_code = 200;
    res->body = NULL;
    conn->response = res;
    return res;
}

const char *status_code_name(http_status_t sc) {
    if (sc == 200) return "OK";
    if (sc == 201) return "CREATED";
    if (sc == 400) return "BAD REQUEST";
    if (sc == 404) return "NOT FOUND";
    if (sc == 500) return "INTERNAL SERVER ERROR";
    return "I'm a teapot";
}

void httpres_free(http_res_t *res);
void httpconn_send_response(http_conn_t *conn, http_error_t *errout) {
    assert(conn->response != NULL && "no response was created!");
    *errout = HTTP_ERROR_NONE;
    tcp_error_t err = TCP_ERROR_NONE;

    http_res_t *res = conn->response;
    msu_str_builder_t sb = msu_str_builder_new();

    msu_str_builder_printf(sb, "HTTP/1.1 %d %s\r\n", res->status_code, status_code_name(res->status_code));

    tcpstream_write(conn->stream, (const uint8_t *) sb->src, sb->len, &err);
    msu_str_builder_reset(sb);
    if (err) {
        *errout = HTTP_ERROR_WRITE;
        goto defer;
    }

    size_t body_len = msu_str_len(res->body);
    const msu_str_t *cl = msu_str_printf("%zu", body_len);
    httpheaders_set(res->headers, HH_CONTENT_LENGTH, cl);
    msu_str_free(cl);

    httpheaders_set(res->headers, HH_SERVER, STRLIT("LMSM/0.1.0 C11"));

    httpheaders_set(res->headers, HH_CONNECTION, HV_CONNECTION_CLOSE);

    http_headers_tree_iter_t iter = http_headers_tree_iter(res->headers->table, BT_TRAVERSE_INSERTION);
    while (http_headers_tree_iter_has_next(&iter)) {
        http_headers_tree_entry_t *entry = http_headers_tree_iter_next(&iter);

        const msu_str_t *name = http_headers_tree_entry_key(entry);
        const list_of_msu_strs_t *values = http_headers_tree_entry_value(entry);

        for (size_t i = 0; i < values->len; i++) {
            const msu_str_t *value = list_of_msu_strs_get_const(values, i);

            msu_str_builder_pushstr(sb, name);
            msu_str_builder_pushs(sb, ": ");
            msu_str_builder_pushstr(sb, value);
            msu_str_builder_pushs(sb, "\r\n");
        }

        tcpstream_write(conn->stream, (const uint8_t *) sb->src, sb->len, &err);
        msu_str_builder_reset(sb);
        if (err) {
            *errout = HTTP_ERROR_WRITE;
            goto defer;
        }
    }

    msu_str_builder_pushs(sb, "\r\n");
    tcpstream_write(conn->stream, (const uint8_t *) sb->src, sb->len, &err);
    msu_str_builder_reset(sb);
    if (err) {
        *errout = HTTP_ERROR_WRITE;
        goto defer;
    }

    if (res->body) {
        tcpstream_write(conn->stream, (const uint8_t *) msu_str_data(res->body), msu_str_len(res->body), &err);
        if (err) {
            *errout = HTTP_ERROR_WRITE;
            goto defer;
        }
    }

defer:
    httpres_free(conn->response);
    msu_str_builder_free(sb);
    conn->response = NULL;
}

void httpconn_close(http_conn_t *conn) {
    assert(conn->stream && "socket is already closed");
    tcpstream_free(conn->stream);
    conn->stream = NULL;
}

void http_serve(tcp_listener_t *server, http_conn_handler_fn connhandler) {
    tcp_error_t err = TCP_ERROR_NONE;
    while (1) {
        tcp_conn_t tcpconn = tcp_accept(server, &err);
        if (err) {
            tcpstream_free(tcpconn.stream);
            socketaddr_free(tcpconn.peer_address);
            continue;
        }

        http_conn_t *conn = malloc(sizeof(http_conn_t));
        assert(conn && "out of memory!\n");
        conn->done_reading = false;
        conn->stream = tcpconn.stream;
        conn->peer_addr = tcpconn.peer_address;
        conn->recv_state = CONN_STATE_INIT;
        conn->response = NULL;
        conn->buffer = msu_str_builder_new();
        conn->buffer_offset = 0;

        connhandler(conn);

        msu_str_builder_free(conn->buffer);
        httpres_free(conn->response);
        tcpstream_free(conn->stream);
        socketaddr_free(conn->peer_addr);
        free(conn);
    }
}

http_headers_t *httpheaders_new() {
    http_headers_t *out = malloc(sizeof(http_headers_t));
    assert(out && "out of memory!\n");
    out->table = http_headers_tree_new();
    return out;
}

void httpheaders_set(http_headers_t *headers, const msu_str_t *name, const msu_str_t *value) {
    name = msu_str_lower(name);
    http_headers_tree_entry_t *entry = http_headers_tree_get(headers->table, name);
    if (!entry) {
        list_of_msu_strs_t *header_values = list_of_msu_strs_new();
        list_of_msu_strs_append(header_values, msu_str_clone(value));
        http_headers_tree_insert(headers->table, msu_str_clone(name), header_values);
    } else {
        list_of_msu_strs_clear(entry->value);
        list_of_msu_strs_append(entry->value, msu_str_clone(value));
    }
    msu_str_free(name);
}

void httpheaders_add(http_headers_t *headers, const msu_str_t *name, const msu_str_t *value) {
    name = msu_str_lower(name);
    http_headers_tree_entry_t *entry = http_headers_tree_get(headers->table, name);
    if (!entry) {
        list_of_msu_strs_t *header_values = list_of_msu_strs_new();
        list_of_msu_strs_append(header_values, msu_str_clone(value));
        http_headers_tree_insert(headers->table, msu_str_lower(name), header_values);
    } else {
        list_of_msu_strs_append(entry->value, msu_str_clone(value));
    }
    msu_str_free(name);
}

list_of_msu_strs_t *httpheaders_get_all(http_headers_t *headers, const msu_str_t *name) {
    name = msu_str_lower(name);
    http_headers_tree_entry_t *entry = http_headers_tree_get(headers->table, name);
    msu_str_free(name);
    if (!entry) return NULL;
    return entry->value;
}

const msu_str_t **httpheaders_get_one(http_headers_t *headers, const msu_str_t *name) {
    name = msu_str_lower(name);
    http_headers_tree_entry_t *entry = http_headers_tree_get(headers->table, name);
    msu_str_free(name);
    if (!entry || entry->value->len != 1) return NULL;
    return list_of_msu_strs_get_ref(entry->value, 0);
}

void httpheaders_free(http_headers_t *headers) {
    if (headers) {
        http_headers_tree_free(headers->table);
        free(headers);
    }
}

void httpres_free(http_res_t *res) {
    if (res) {
        httpheaders_free(res->headers);
        msu_str_free(res->body);
        free(res);
    }
}

STRLIT_DEF(HH_CONTENT_TYPE, "Content-Type");
STRLIT_DEF(HH_CONTENT_LENGTH, "Content-Length");
STRLIT_DEF(HH_SERVER, "Server");
STRLIT_DEF(HH_CONNECTION, "Connection");

STRLIT_DEF(CT_PLAINTEXT, "text/plain");
STRLIT_DEF(CT_HTML, "text/html");
STRLIT_DEF(CT_ICON, "image/x-icon");

STRLIT_DEF(HV_CONNECTION_CLOSE, "close");