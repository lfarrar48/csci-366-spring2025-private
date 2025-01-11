#include "lmsm/firth.h"

parsenode_t *fr_parse_elt_impl(parser *firth_parser);

token *fr_tokenize(const msu_str_t *src);

parser fr_parser(token *pToken);

parsenode_t *fr_maybe_consume_comment(parser *pParser);

bool fr_peek_soft_kw(parser *firth_parser, const char *kw);

parsenode_t *fr_parse(const msu_str_t *src) {
    token *tokens = fr_tokenize(src);
    parser p = fr_parser(tokens);

    parsenode_t *program = parsenode_new(FR_PROGRAM, NULL);

    fr_maybe_consume_comment(&p);
    while (parser_has_next(&p)) {
        parsenode_t *child = fr_parse_elt_impl(&p);
        parsenode_add_child(program, child);
    }
    parsenode_t *bad_comment_node = fr_maybe_consume_comment(&p);
    if (bad_comment_node) {
        parsenode_add_child(program, bad_comment_node);
    }

    return program;
}

parsenode_t *fr_parse_elt(const msu_str_t *src) {
    token *tokens = fr_tokenize(src);
    parser p = fr_parser(tokens);
    return fr_parse_elt_impl(&p);
}

parsenode_t *fr_parse_elt_impl(parser *firth_parser) {
    parsenode_t *elt;

    parsenode_t *bad_comment_node = fr_maybe_consume_comment(firth_parser);
    if (bad_comment_node) {
        return bad_comment_node;
    }

    if (parser_peek_kind(firth_parser, TT_INT)) {
        elt = parsenode_new(FR_INT, parser_take(firth_parser));
    } else if (parser_peek_kind(firth_parser, TT_PLUS)) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (parser_peek_kind(firth_parser, TT_DASH)) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (parser_peek_kind(firth_parser, TT_STAR)) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (parser_peek_kind(firth_parser, TT_SLASH)) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (parser_peek_kind(firth_parser, TT_DOT)) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "get")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "min")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "max")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "push")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "pop")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "dup")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "drop")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "exit")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "swap")) {
        elt = parsenode_new(FR_OP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "do")) {
        elt = parsenode_new(FR_DO_LOOP, parser_take(firth_parser));
        while (parser_has_next(firth_parser) &&
               !fr_peek_soft_kw(firth_parser, "loop")) {
            parsenode_t *child = fr_parse_elt_impl(firth_parser);
            parsenode_add_child(elt, child);
        }
        if (parser_has_next(firth_parser) &&
            fr_peek_soft_kw(firth_parser, "loop")) {
            parser_take_word(firth_parser);
        } else {
            parsenode_t *error_node = parsenode_new(FR_ERROR, NULL);
            parsenode_set_error(error_node, msu_str_new("Unterminated Do Loop"));
            parsenode_add_child(elt, error_node);
        }
    } else if (fr_peek_soft_kw(firth_parser, "stop")) {
        elt = parsenode_new(FR_STOP, parser_take(firth_parser));
    } else if (fr_peek_soft_kw(firth_parser, "zero?") ||
               fr_peek_soft_kw(firth_parser, "positive?")) {

        token *token = parser_take(firth_parser);

        if (msu_str_eqs(token->content, "zero?")) {
            elt = parsenode_new(FR_ZERO_TEST, token);
        } else {
            elt = parsenode_new(FR_POSITIVE_TEST, token);
        }

        parsenode_t *true_block = parsenode_new(FR_TRUE_BLOCK, NULL);
        parsenode_add_child(elt, true_block);

        parsenode_t *false_block = parsenode_new(FR_FALSE_BLOCK, NULL);
        parsenode_add_child(elt, false_block);

        while (parser_has_next(firth_parser) &&
               !fr_peek_soft_kw(firth_parser, "else") &&
               !fr_peek_soft_kw(firth_parser, "end")) {
            parsenode_t *child = fr_parse_elt_impl(firth_parser);
            parsenode_add_child(true_block, child);
        }
        if (parser_has_next(firth_parser) &&
            fr_peek_soft_kw(firth_parser, "else")) {
            parser_take(firth_parser); // consume the else
            while (parser_has_next(firth_parser) &&
                   !fr_peek_soft_kw(firth_parser, "end")) {
                parsenode_t *child = fr_parse_elt_impl(firth_parser);
                parsenode_add_child(false_block, child);
            }
        }
        if (parser_has_next(firth_parser) &&
            fr_peek_soft_kw(firth_parser, "end")) {
            parser_take_word(firth_parser);
        } else {
            parsenode_t *error_node = parsenode_new(FR_ERROR, NULL);
            parsenode_set_error(error_node, msu_str_new("Unterminated Zero Test"));
            parsenode_add_child(elt, error_node);
        }
    } else if (fr_peek_soft_kw(firth_parser, "var")) {
        parser_take(firth_parser);
        if (parser_has_next(firth_parser) && parser_peek_word(firth_parser)) {
            elt = parsenode_new(FR_VAR, parser_take(firth_parser));
        } else {
            elt = parsenode_new(FR_ERROR, NULL);
            parsenode_set_error(elt, msu_str_new("Variable name required"));
        }
    } else if (parser_take_kind(firth_parser, TT_COLON)) {
        if (parser_has_next(firth_parser) && parser_peek_word(firth_parser)) {
            elt = parsenode_new(FR_FUNCTION_DEF, parser_take(firth_parser));
            while (parser_has_next(firth_parser) && !parser_peek_kind(firth_parser, TT_SEMICOLON)) {
                parsenode_t *child = fr_parse_elt_impl(firth_parser);
                parsenode_add_child(elt, child);
            }
        } else {
            elt = parsenode_new(FR_ERROR, NULL);
            parsenode_set_error(elt, msu_str_new("Function name required"));
        }
        if (parser_has_next(firth_parser) && parser_peek_kind(firth_parser, TT_SEMICOLON)) {
            parser_take(firth_parser);
        } else {
            parsenode_t *error_node = parsenode_new(FR_ERROR, NULL);
            parsenode_set_error(error_node, msu_str_new("Unterminated Function Definition"));
            parsenode_add_child(elt, error_node);
        }
    } else if (fr_peek_soft_kw(firth_parser, "asm")) {
        elt = parsenode_new(FR_ASM, parser_take(firth_parser));
        while (parser_has_next(firth_parser) &&
               !fr_peek_soft_kw(firth_parser, "end")) {
            parsenode_t *child = parsenode_new(FR_ASM_WORD, parser_take(firth_parser));
            parsenode_add_child(elt, child);
        }
        if (parser_has_next(firth_parser) &&
            fr_peek_soft_kw(firth_parser, "end")) {
            parser_take_word(firth_parser);
        } else {
            parsenode_t *error_node = parsenode_new(FR_ERROR, NULL);
            parsenode_set_error(error_node, msu_str_new("Unterminated ASM block"));
            parsenode_add_child(elt, error_node);
        }
    } else if (parser_peek_word(firth_parser)) {
        elt = parsenode_new(FR_WORD, parser_take(firth_parser));
    } else {
        elt = parsenode_new(FR_ERROR, parser_take(firth_parser));
        parsenode_set_error(elt, msu_str_new("unexpected token!"));
    }

    return elt;
}

bool fr_peek_soft_kw(parser *firth_parser, const char *kw) {
    token *token = parser_peek_word(firth_parser);
    if (token) {
        return msu_str_eqs(token->content, kw);
    }
    return false;
}

parsenode_t *fr_maybe_consume_comment(parser *pParser) {
    if (parser_peek_kind(pParser, TT_LPAREN)) {
        while (parser_has_next(pParser) && !parser_peek_kind(pParser, TT_RPAREN)) {
            parser_take(pParser);
        }
        if (parser_take_kind(pParser, TT_RPAREN)) {
            return NULL;
        } else {
            parsenode_t *error_node = parsenode_new(FR_ERROR, NULL);
            parsenode_set_error(error_node, msu_str_new("Unterminated Comment"));
            return error_node;
        }
    }
    return NULL;
}

token *fr_tokenize(const msu_str_t *src) {
    return tokenize_ext(src, NULL);
}

parser fr_parser(token *tokens) {
    static const msu_str_t *KEYWORDS[] = {};
    static list_of_msu_strs_t KEYWORD_LIST = {
            .values = KEYWORDS,
            .len = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]),
            .cap = 0,
    };

    return (parser) {
            .root = tokens,
            .current = tokens,
            .keywords = &KEYWORD_LIST,
    };
}

fr_context_t *fr_context_new() {
    fr_context_t *ctx = calloc(1, sizeof(fr_context_t));
    ctx->label_num = 0;
    ctx->variables = list_of_msu_strs_new();
    ctx->loop_label_stack = list_of_msu_strs_new();
    return ctx;
}

void fr_context_free(fr_context_t *ctx) {
    list_of_msu_strs_free(ctx->variables, true);
    list_of_msu_strs_free(ctx->loop_label_stack, true);
    free(ctx);
}

void fr_compile_node(const parsenode_t *node, fr_context_t *ctx, msu_str_builder_t output) {
    if (node->kind == FR_OP) {
        if (msu_str_eqs(node->token->content, ".")) {
            msu_str_builder_pushs(output, "SDUP\nSPOP\nOUT\n");
        } else if (msu_str_eqs(node->token->content, "+")) {
            msu_str_builder_pushs(output, "SADD\n");
        } else if (msu_str_eqs(node->token->content, "-")) {
            msu_str_builder_pushs(output, "SSUB\n");
        } else if (msu_str_eqs(node->token->content, "*")) {
            msu_str_builder_pushs(output, "SMUL\n");
        } else if (msu_str_eqs(node->token->content, "/")) {
            msu_str_builder_pushs(output, "SDIV\n");
        } else if (msu_str_eqs(node->token->content, "max")) {
            msu_str_builder_pushs(output, "SMAX\n");
        } else if (msu_str_eqs(node->token->content, "min")) {
            msu_str_builder_pushs(output, "SMIN\n");
        } else if (msu_str_eqs(node->token->content, "get")) {
            msu_str_builder_pushs(output, "INP\nSPUSH\n");
        } else if (msu_str_eqs(node->token->content, "pop")) {
            msu_str_builder_pushs(output, "SPOP\n");
        } else if (msu_str_eqs(node->token->content, "dup")) {
            msu_str_builder_pushs(output, "SDUP\n");
        } else if (msu_str_eqs(node->token->content, "swap")) {
            msu_str_builder_pushs(output, "SSWAP\n");
        } else if (msu_str_eqs(node->token->content, "drop")) {
            msu_str_builder_pushs(output, "SDROP\n");
        } else if (msu_str_eqs(node->token->content, "exit")) {
            msu_str_builder_pushs(output, "RET\n");
        }
    } else if (node->kind == FR_INT) {
        msu_str_builder_pushs(output, "LDI ");
        msu_str_builder_pushstr(output, node->token->content);
        msu_str_builder_pushs(output, "\n");
        msu_str_builder_pushs(output, "SPUSH\n");
    } else if (node->kind == FR_DO_LOOP) {
        int loop_label_num = ctx->label_num++;
        msu_str_builder_printf(output, "loop_%d_start ADD ZERO\n", loop_label_num);

        const msu_str_t *loop_label_end = msu_str_printf("loop_%d_end", loop_label_num);
        list_of_msu_strs_push(ctx->loop_label_stack, loop_label_end);
        for (size_t i = 0; i < node->children->len; i++) {
            parsenode_t *child = list_of_parsenodes_get(node->children, i);
            fr_compile_node(child, ctx, output);
        }
        list_of_msu_strs_pop(ctx->loop_label_stack);
        msu_str_free(loop_label_end);

        msu_str_builder_printf(output, "BRA loop_%d_start\n", loop_label_num);
        msu_str_builder_printf(output, "loop_%d_end ADD ZERO\n", loop_label_num);
    } else if (node->kind == FR_ZERO_TEST || node->kind == FR_POSITIVE_TEST) {
        int if_zero_label_num = ctx->label_num++;

        // branch if top of stack zero
        msu_str_builder_pushs(output, "SPOP\n");
        if (node->kind == FR_ZERO_TEST) {
            msu_str_builder_pushs(output, "BRZ ");
        } else {
            msu_str_builder_pushs(output, "BRP ");
        }
        parsenode_t *true_branch = list_of_parsenodes_get(node->children, 0);
        parsenode_t *false_branch = list_of_parsenodes_get(node->children, 1);

        if (true_branch->children->len > 0) {
            msu_str_builder_printf(output, "if_%d", if_zero_label_num);
        } else {
            msu_str_builder_printf(output, "end_%d", if_zero_label_num);
        }
        msu_str_builder_pushs(output, "\n");

        // generate else
        if (false_branch->children->len > 0) {
            for (size_t i = 0; i < false_branch->children->len; i++) {
                parsenode_t *child = list_of_parsenodes_get(false_branch->children, i);
                fr_compile_node(child, ctx, output);
            }
        }

        // jump to end of zero condition
        msu_str_builder_pushs(output, "BRA ");
        msu_str_builder_printf(output, "end_%d", if_zero_label_num);
        msu_str_builder_pushs(output, "\n");

        // generate if zero condition
        if (true_branch->children->len > 0) {
            msu_str_builder_printf(output, "if_%d", if_zero_label_num);
            msu_str_builder_pushs(output, " ");
            for (size_t i = 0; i < true_branch->children->len; i++) {
                parsenode_t *child = list_of_parsenodes_get(true_branch->children, i);
                fr_compile_node(child, ctx, output);
            }
        }

        // label end of zero conditional
        msu_str_builder_printf(output, "end_%d", if_zero_label_num);
        msu_str_builder_pushs(output, " ADD ZERO\n");
    } else if (node->kind == FR_WORD) {
        if (msu_str_ends_with(node->token->content, "!")) {
            msu_str_builder_pushs(output, "SPOP\nSTA ");
            const msu_str_t *variable_name = msu_str_slice_right(node->token->content, 1);
            msu_str_builder_pushstr(output, variable_name);
            msu_str_free(variable_name);
            msu_str_builder_pushs(output, "\n");
        } else if (list_of_msu_strs_contains(ctx->variables, node->token->content)) {
            msu_str_builder_pushs(output, "LDA ");
            msu_str_builder_pushstr(output, node->token->content);
            msu_str_builder_pushs(output, "\nSPUSH\n");
        } else {
            msu_str_builder_pushs(output, "RPUSH\n");
            msu_str_builder_pushs(output, "CALL ");
            msu_str_builder_pushstr(output, node->token->content);
            msu_str_builder_pushs(output, "\n");
            msu_str_builder_pushs(output, "RPOP\n");
        }
    } else if(node->kind == FR_VAR) {
        // ignore
    } else if(node->kind == FR_STOP) {
        if (ctx->loop_label_stack->len > 0) {
            const msu_str_t *label = list_of_msu_strs_get_const(ctx->loop_label_stack, ctx->loop_label_stack->len - 1);
            msu_str_builder_printf(output, "BRA %s\n", msu_str_data(label));
        } else {
            printf("\nNo Loop To Break: %s\n", msu_str_data(node->token->content));
        }
    } else {
        printf("\nUnknown Node Type: %i\n", node->kind);
    }
}

void fr_code_gen_top_level(const parsenode_t *program, fr_context_t *ctx, msu_str_builder_t output) {
    for (size_t i = 0; i < program->children->len; i++) {
        parsenode_t *child = list_of_parsenodes_get(program->children, i);
        if (child->kind != FR_FUNCTION_DEF) {
            fr_compile_node(child, ctx, output);
        }
    }
    msu_str_builder_pushs(output, "ZERO HLT\n"); // label the halt so we can use it for no-ops
}

void fr_code_gen_functions(const parsenode_t *program, fr_context_t *ctx, msu_str_builder_t output) {
    for (size_t i = 0; i < program->children->len; i++) {
        parsenode_t *child = list_of_parsenodes_get(program->children, i);
        if (child->kind == FR_FUNCTION_DEF) {
            parsenode_t *function = child;
            // function label
            msu_str_builder_pushstr(output, function->token->content);
            msu_str_builder_pushs(output, " ");
            // function body
            for (size_t i = 0; i < function->children->len; i++) {
                parsenode_t *child = list_of_parsenodes_get(function->children, i);
                fr_compile_node(child, ctx, output);
            }
            // always append a RET
            msu_str_builder_pushs(output, "RET\n");
        }
    }
}

void fr_code_gen_variables(const parsenode_t *program, fr_context_t *ctx, msu_str_builder_t output) {
    (void) ctx;
    for (size_t i = 0; i < program->children->len; i++) {
        parsenode_t *child = list_of_parsenodes_get(program->children, i);
        if (child->kind == FR_VAR) {
            // variable label
            msu_str_builder_pushstr(output, child->token->content);
            msu_str_builder_pushs(output, " DAT 0\n");
        }
    }
}

void firth_code_collect_vars(const parsenode_t *program, fr_context_t *ctx) {
    for (size_t i = 0; i < program->children->len; i++) {
        parsenode_t *child = list_of_parsenodes_get(program->children, i);
        if (child->kind == FR_VAR) {
            list_of_msu_strs_append(ctx->variables, child->token->content);
        }
    }
}

void firth_code_gen(const parsenode_t *program, fr_context_t *ctx, msu_str_builder_t output) {
    firth_code_collect_vars(program, ctx);
    fr_code_gen_top_level(program, ctx, output);
    fr_code_gen_functions(program, ctx, output);
    fr_code_gen_variables(program, ctx, output);
}

const msu_str_t *fr_compile_impl(const msu_str_t *src, bool debug) {
    parsenode_t *program = fr_parse(src);
    if (parsenode_has_errors(program)) {
        parsenode_print_errors(program);
        return EMPTY_STRING;
    }
    if (debug) {
        printf("Firth Parse Tree:\n");
        parsenode_print(program);
    }
    const msu_str_t *asm_bc = fr_compile_program(program);
    if (debug) {
        printf("\nGenerated Assembly:\n\n%s", msu_str_data(asm_bc));
    }
    parsenode_free(program);
    return asm_bc;
}

const msu_str_t *fr_compile_debug(const msu_str_t *src) {
    return fr_compile_impl(src, true);
}

const msu_str_t *fr_compile(const msu_str_t *src) {
    return fr_compile_impl(src, false);
}

const msu_str_t *fr_compile_program(const parsenode_t *program) {
    if (program->kind != FR_PROGRAM) return EMPTY_STRING;
    msu_str_builder_t sb = msu_str_builder_new();
    fr_context_t *ctx = fr_context_new();
    firth_code_gen(program, ctx, sb);
    fr_context_free(ctx);
    const msu_str_t *asm_src = msu_str_builder_into_string_and_free(sb);
    return asm_src;
}
