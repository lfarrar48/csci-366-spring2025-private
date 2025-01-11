#include "msulib/http.h"

void ep_assets_get(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_index_get(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_emulator_memory_value_post(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_favicon_get(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_emulator_register_value_post(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_emulator_input_post(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_emulator_code_post(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_emulator_step_post(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_emulator_restart_post(http_conn_t *conn, http_req_t req, http_error_t *errout);
void ep_emulator_reset_post(http_conn_t *conn, http_req_t req, http_error_t *errout);

// response utilities

void bad_request(http_conn_t *conn, http_error_t *errout, const char *msg);
void internal_server_error(http_conn_t *conn, http_error_t *errout);
void ok(http_conn_t *conn, http_error_t *errout);
void reply_html(http_conn_t *conn, http_error_t *errout, http_status_t status, const msu_str_t *body);
