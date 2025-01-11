#include <msulib/str.h>
#include <msulib/parser.h>

typedef enum firth_node_kind {
    FR_PROGRAM,
    FR_INT,
    FR_OP,
    FR_ZERO_TEST,
    FR_TRUE_BLOCK,
    FR_FALSE_BLOCK,
    FR_POSITIVE_TEST,
    FR_DO_LOOP,
    FR_STOP,
    FR_VAR,
    FR_FUNCTION_DEF,
    FR_WORD,
    FR_ASM,
    FR_ASM_WORD,
    FR_ERROR,
} firth_node_kind;

typedef struct fr_context_t {
    int label_num;
    list_of_msu_strs_t *variables;
    list_of_msu_strs_t *loop_label_stack;
} fr_context_t;

parsenode_t *fr_parse(const msu_str_t *src);
parsenode_t *fr_parse_elt(const msu_str_t *src);

fr_context_t *fr_context_new();
void fr_context_free(fr_context_t *ctx);

void fr_compile_node(const parsenode_t *node, fr_context_t *ctx, msu_str_builder_t output);
void firth_code_gen(const parsenode_t *program, fr_context_t *ctx, msu_str_builder_t output);

const msu_str_t *fr_compile(const msu_str_t *src);
const msu_str_t *fr_compile_debug(const msu_str_t *src);
const msu_str_t *fr_compile_program(const parsenode_t *program);
