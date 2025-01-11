#include "msulib/str.h"
#include "msulib/http.h"

typedef bool (*lang_compile_fn)(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load);

bool compile_sea(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load); // true if succeeded
bool compile_firth(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load); // true if succeeded
bool compile_zortran(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load); // true if succeeded
bool compile_asm(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load); // true if succeeded
