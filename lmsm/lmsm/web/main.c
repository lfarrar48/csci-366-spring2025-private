#include "msulib/http.h"
#include "msulib/fs.h"

#include "src/lang.h"
#include "src/endpoints.h"
#include "main.h"


void setupCwd();
void httpHandler(http_conn_t *conn);

bool wants_input = false;
char input_buffer[INPUT_BUFFER_SIZE];
emulator_t *the_one_emulator;

#define DEFER_FAIL(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    exit_code = EXIT_FAILURE;     \
    goto defer;                   \
} while (0)

int main() {
    setupCwd();

    the_one_emulator = emulator_new();
    the_one_emulator->input_buffer = input_buffer;

    int exit_code = EXIT_SUCCESS;
    tcp_error_t err = TCP_ERROR_NONE;
    socket_addr_t *addr = NULL;
    tcp_listener_t *server = NULL;

    // initialize networking apis
    sockets_init(&err);
    if (err) DEFER_FAIL("unable to initialize sockets: %s\n", tcp_error_message(err));

    // resole our server's host address
    addr = socket_resolve_addr("0.0.0.0", 8191, &err);
    if (err) DEFER_FAIL("unable to resolve address: %s\n", tcp_error_message(err));

    // bind our server to that address
    server = tcp_bind(addr, &err);
    if (err) DEFER_FAIL("unable to bind server: %s\n", tcp_error_message(err));

    // print out the bound address
    const msu_str_t *host = tcplistener_get_host(server, &err);
    if (err) DEFER_FAIL("unable to get server host: %s\n", tcp_error_message(err));
    const uint16_t port = tcplistener_get_port(server, &err);
    if (err) DEFER_FAIL("unable to get server port: %s\n", tcp_error_message(err));
    printf("serving on http://%s:%d\n", msu_str_eqs(host, "0.0.0.0") ? "localhost" : msu_str_data(host), port);
    fflush(stdout);
    msu_str_free(host);

    // run the server
    http_serve(server, httpHandler);

    // cleanup the networking apis
    sockets_cleanup();

defer:
    // free used memory
    socketaddr_free(addr);
    tcplistener_free(server);
    emulator_free(the_one_emulator);
    return exit_code;
}

void setupCwd() {
    fs_error_t err;

    err = set_working_directory(STRLIT("../lmsm/lmsm/web/"));
    assert(!err && "unable to set cwd");

    const msu_str_t *cwd = NULL;
    err = get_working_directory(&cwd);
    assert(!err && "unable to get cwd");

    printf("cwd() = %s\n", msu_str_data(cwd));
    msu_str_free(cwd);
}

// ENDPOINTS

void httpHandler(http_conn_t *conn) {
    http_error_t err = HTTP_ERROR_NONE;
    http_req_t req;

    httpcon_recv_request(conn, &req, &err);
    if (err) return;

    printf("received request '%s' '%s'\n", msu_str_data(req.info.method), msu_str_data(req.info.url));

    const msu_str_t *url = req.info.url;

    void (*handler)(http_conn_t *, http_req_t, http_error_t *) = NULL;
    if (msu_str_eqs(req.info.method, "GET")) {
        if (msu_str_sws(url, "/assets/")) {
            handler = ep_assets_get;
        } else if (msu_str_eqs(url, "/")) {
            handler = ep_index_get;
        } else if (msu_str_eqs(url, "/favicon.ico")) {
            handler = ep_favicon_get;
        }
    } else if (msu_str_eqs(req.info.method, "POST")) {
        if (msu_str_eqs(url, "/emulator/memory/cell")) {
            handler = ep_emulator_memory_value_post;
        } else if (msu_str_eqs(url, "/emulator/register/value")) {
            handler = ep_emulator_register_value_post;
        } else if (msu_str_eqs(url, "/emulator/stdin")) {
            handler = ep_emulator_input_post;
        } else if (msu_str_eqs(url, "/emulator/code")) {
            handler = ep_emulator_code_post;
        } else if (msu_str_eqs(url, "/emulator/step")) {
            handler = ep_emulator_step_post;
        } else if (msu_str_eqs(url, "/emulator/restart")) {
            handler = ep_emulator_restart_post;
        } else if (msu_str_eqs(url, "/emulator/reset")) {
            handler = ep_emulator_reset_post;
        }
    }

    if (handler) {
        handler(conn, req, &err);
    } else {
        http_res_t *res = httpcon_make_response(conn);
        res->status_code = HTTP_STATUS_NOT_FOUND;
        httpheaders_set(res->headers, HH_CONTENT_TYPE, CT_HTML);
        res->body = msu_str_new(
                "<html><head><title>Not Found</title></head><body><h1>Uh Oh! Dead End!</h1>there's nothing here!</body></html>");
        httpconn_send_response(conn, &err);
    }

    httpconn_close(conn);
}


