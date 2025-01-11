#include <msulib/parser.h>

typedef enum SEA_kind {
    SEA_PROGRAM,
    SEA_TYPE,
    SEA_IDENT,

    SEA_FUNCDEF,
    SEA_PARAMS,
    SEA_PARAM,

    SEA_VAR,
    SEA_BLOCK,
    SEA_IF,
    SEA_WHILE,
    SEA_DO_WHILE,
    SEA_FOR,
    SEA_RETURN,

    SEA_BINARY,
    SEA_UNARY,
    SEA_CALL,
    SEA_ARGS,
    SEA_INT,
    SEA_GROUP,

    SEA_ERROR,
} sea_kind_t;

parsenode_t *sea_parse(const msu_str_t *src);
parsenode_t *sea_parse_stmt(const msu_str_t *src);
parsenode_t *sea_parse_expr(const msu_str_t *src);

typedef struct sea_error {
    const parsenode_t *node;
    const msu_str_t *message;
} sea_error_t;

sea_error_t *sea_error_new(const parsenode_t *node, const msu_str_t *msg);

const msu_str_t *sea_compile(const parsenode_t *program, sea_error_t **errout);
const msu_str_t *sea_compile_debug(const parsenode_t *program);

void sea_error_free(sea_error_t *error);