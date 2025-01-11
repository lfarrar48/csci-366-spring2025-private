#ifndef zortran_h
#define zortran_h

#include <msulib/str.h>
#include <msulib/parser.h>

typedef enum zortran_node_kind {
    ZT_PROGRAM,
    ZT_BLOCK,
    ZT_ASSIGN,
    ZT_READ,
    ZT_INT,
    ZT_VAR,
    ZT_OP,
    ZT_WHILE,
    ZT_WRITE,
    ZT_ERROR,
} zortran_node_kind;

parsenode_t *zt_parse(const msu_str_t *src);
parsenode_t *zt_parse_stmt(const msu_str_t *src);
const msu_str_t *zt_compile(parsenode_t *node);

#endif // zortran_h