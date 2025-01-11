#ifndef MSU_HTTP_H
#define MSU_HTTP_H

#include "msulib/socket.h"
#include "msulib/str.h"

#define BT_NAME kv_store
#define BT_KEY const msu_str_t *
#define BT_VALUE list_of_msu_strs_t *
#define BT_HASHFUNC(x) msu_str_hash(x, 42)
#define BT_EQFUNC msu_str_eq
#define BT_FREE_KEY msu_str_free
#define BT_FREE_VALUE(x) list_of_msu_strs_free(x, true)
#include "templates/btree.h"

const msu_str_t **kv_store_get_one(kv_store_t *me, const msu_str_t *str);

typedef struct httpinfo {
    const msu_str_t *method;
    const msu_str_t *url;
    const msu_str_t *version;
} http_info_t;

typedef struct httpheaders http_headers_t;
typedef struct httpheader {
    const msu_str_t *name;
    const msu_str_t *value;
} http_header_t;

typedef enum httperror {
    HTTP_ERROR_NONE = 0,
    HTTP_ERROR_BAD_REQUEST,
    HTTP_ERROR_BAD_VERSION,
    HTTP_ERROR_REQ_TOO_BIG,
    HTTP_ERROR_READ,
    HTTP_ERROR_WRITE,
    HTTP_ERROR_URL_DECODE,
    HTTP_ERROR_FORM_DATA_DECODE,
} http_error_t;

typedef struct httpconn http_conn_t;
typedef struct httpres {
    uint16_t status_code;
    http_headers_t *headers;
    const msu_str_t *body;
} http_res_t;

typedef struct httpreq {
    const http_info_t info;
    const http_headers_t *headers;
    const msu_str_t *body;
} http_req_t;


// IMPORTANT: always check ur errors
void httpcon_recv_request(http_conn_t *conn, http_req_t *out, http_error_t *errout);
http_res_t *httpcon_make_response(http_conn_t *conn);
void httpconn_send_response(http_conn_t *conn, http_error_t *errout);
void httpconn_close(http_conn_t *conn);

bool http_parse_header(const msu_str_t *header, http_header_t *out);

const msu_str_t *http_url_decode(const msu_str_t *str, http_error_t *err);
kv_store_t *http_form_data_decode(const msu_str_t *str, http_error_t *err);

typedef enum httpstatus {
    HTTP_STATUS_OK = 200,
    HTTP_STATUS_CREATED = 201,
    HTTP_STATUS_BAD_REQUEST = 400,
    HTTP_STATUS_NOT_FOUND = 404,
    HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
} http_status_t;


http_headers_t *httpheaders_new();
void httpheaders_set(http_headers_t *headers, const msu_str_t *name, const msu_str_t *value);
void httpheaders_add(http_headers_t *headers, const msu_str_t *name, const msu_str_t *value);
list_of_msu_strs_t *httpheaders_get_all(http_headers_t *headers, const msu_str_t *name);
const msu_str_t **httpheaders_get_one(http_headers_t *headers, const msu_str_t *name);
void httpheaders_free(http_headers_t *headers);

typedef void (*http_conn_handler_fn)(http_conn_t *conn);
void http_serve(tcp_listener_t *server, http_conn_handler_fn connhandler);

extern const msu_str_t *HH_CONTENT_TYPE;
extern const msu_str_t *HH_CONTENT_LENGTH;
extern const msu_str_t *HH_SERVER;
extern const msu_str_t *HH_CONNECTION;

extern const msu_str_t *CT_PLAINTEXT;
extern const msu_str_t *CT_HTML;
extern const msu_str_t *CT_ICON;
extern const msu_str_t *HV_CONNECTION_CLOSE;

#endif // MSU_HTTP_H