#include "lmsm/zortran.h"

#include "msulib/str.h"

parser zt_parser(token *tokens) {
    static strlit KW_DO = STRLIT_VAL("DO");
    static strlit KW_WHILE = STRLIT_VAL("WHILE");
    static strlit KW_END = STRLIT_VAL("END");
    static strlit KW_READ = STRLIT_VAL("READ");
    static strlit KW_WRITE = STRLIT_VAL("WRITE");
    static const strlit *KEYWORDS[] = { &KW_DO, &KW_WHILE, &KW_END, &KW_READ, &KW_WRITE };
    static const size_t N_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);
    static list_of_msu_strs_t LIST_OF_KWS = { (const msu_str_t **)KEYWORDS, N_KEYWORDS, 0 };

    return (parser){
        .root = tokens,
        .current = tokens,
        .keywords = &LIST_OF_KWS,
    };
}

token *zt_tokenize(const msu_str_t *src) {
//    static strlit ZT_COMMENT_START = STRLIT("!");
    return tokenize(src, STRLIT("!"));
}

parsenode_t *zt_parse_stmt_impl(parser *parser);
parsenode_t *zt_parse_condition(parser *parser);
parsenode_t *zt_parse_expression(parser *parser);
parsenode_t *zt_parse_primary(parser *parser);

parsenode_t *zt_parse(const msu_str_t *src) {
    token *tokens = zt_tokenize(src);
    parser p = zt_parser(tokens);

    parsenode_t *out = parsenode_new(ZT_PROGRAM, NULL);
    while (parser_has_next(&p)) {
        parsenode_t *child = zt_parse_stmt_impl(&p);
        parsenode_add_child(out, child);
    }

    return out;
}

parsenode_t *zt_parse_stmt(const msu_str_t *src) {
    token *tokens = zt_tokenize(src);
    parser p = zt_parser(tokens);

    parsenode_t *child = zt_parse_stmt_impl(&p);

    // this checks that the child is null or the parser's current token is null
    if (child && msu_str_is_empty(child->error) && parser_has_next(&p)) {
        parsenode_set_error(child, msu_str_new("expected end of input"));
    }

    tokens_free(tokens);
    return child;
}

parsenode_t *zt_parse_stmt_impl(parser *parser) {
    parsenode_t *stmt;

    if (parser_take_kw(parser, "DO")) {
        stmt = parsenode_new(ZT_WHILE, NULL);
        if (!parser_take_kw(parser, "WHILE")) {
            parsenode_set_error(stmt, msu_str_new("expected 'WHILE' after 'DO'"));
            return stmt;
        }

        parsenode_t *expr = zt_parse_condition(parser);
        parsenode_add_child(stmt, expr);

        parsenode_t *block = parsenode_new(ZT_BLOCK, NULL);
        parsenode_add_child(stmt, block);
        while (parser_has_next(parser) && !parser_peek_kw(parser, "END")) {
            parsenode_t *item = zt_parse_stmt_impl(parser);
            parsenode_add_child(block, item);
        }
        if (!parser_take_kw(parser, "END")) {
            parsenode_set_error(block, msu_str_new("expected 'end' after block"));
        }
    } else if (parser_take_kw(parser, "WRITE")) {
        stmt = parsenode_new(ZT_WRITE, NULL);

        parsenode_t *value = zt_parse_expression(parser);
        if (!value) {
            value = parsenode_new(ZT_ERROR, NULL);
            parsenode_set_error(value, msu_str_new("expected value after 'WRITE'"));
        }
        parsenode_add_child(stmt, value);
    } else if (parser_peek_word(parser)) {
        token *word = parser_take(parser);
        stmt = parsenode_new(ZT_ASSIGN, word);

        if (!parser_take_punct(parser, "=")) {
            parsenode_set_error(stmt, msu_str_new("expected '=' after variable"));
        }

        parsenode_t *value;
        if (parser_take_kw(parser, "READ")) {
            value = parsenode_new(ZT_READ, NULL);
        } else {
            value = zt_parse_expression(parser);
            if (!value) {
                value = parsenode_new(ZT_ERROR, NULL);
                parsenode_set_error(value, msu_str_new("expected assigned value"));
            }
        }
        parsenode_add_child(stmt, value);
    } else {
        stmt = parsenode_new(ZT_ERROR, parser_take(parser));
        parsenode_set_error(stmt, msu_str_new("expected statement"));
    }

    return stmt;
}

parsenode_t *zt_parse_condition(parser *parser) {
    parsenode_t *lhs = zt_parse_expression(parser);
    if (!lhs) return NULL;

    parsenode_t *expr;
    if (parser_peek_punct(parser, ">=") || parser_peek_punct(parser, "=")) {
        expr = parsenode_new(ZT_OP, parser_take(parser));
        parsenode_add_child(expr, lhs);

        parsenode_t *rhs = zt_parse_expression(parser);  // 👈 ALLOW ANY EXPRESSION
        if (!rhs) {
            rhs = parsenode_new(ZT_ERROR, NULL);
            parsenode_set_error(rhs, msu_str_new("expected right-hand expression"));
        }
        parsenode_add_child(expr, rhs);
        return expr;
    } else {
        expr = parsenode_new(ZT_ERROR, parser_take(parser));
        parsenode_set_error(expr, msu_str_new("expected '>=' or '=' for conditional expression"));
        return expr;
    }
}


parsenode_t *zt_parse_expression(parser *parser) {
    parsenode_t *lhs = zt_parse_primary(parser);
    if (!lhs) return NULL;

    if (parser_peek_punct(parser, "+") || parser_peek_punct(parser, "-")) {
        token *op_tok = parser_take(parser);
        parsenode_t *rhs = zt_parse_primary(parser);
        if (!rhs) {
            rhs = parsenode_new(ZT_ERROR, NULL);
            parsenode_set_error(rhs, msu_str_new("expected value after operator"));
        }
        parsenode_t *op = parsenode_new(ZT_OP, op_tok);
        parsenode_add_child(op, lhs);
        parsenode_add_child(op, rhs);
        return op;
    }

    return lhs;
}


parsenode_t *zt_parse_primary(parser *parser) {
    parsenode_t *out = NULL;

    if (parser_peek_kind(parser, TT_INT)) {
        out = parsenode_new(ZT_INT, parser_take(parser));
    } else if (parser_peek_word(parser)) {
        out = parsenode_new(ZT_VAR, parser_take(parser));
    }

    return out;
}

typedef struct zt_var {
    const msu_str_t *name;
    token *value;
    struct zt_var *next;
} zt_var_t;

void zt_var_free(zt_var_t *var) {
    if (var->value) {
        msu_str_free(var->name);
    }
    free(var);
}

typedef struct zt_context {
    zt_var_t *vhead;
    zt_var_t *vtail;
    int label_num;
} zt_context_t;

void zt_context_free(zt_context_t *ctx) {
    zt_var_t *node = ctx->vhead;
    while (node) {
        zt_var_t *next = node->next;
        zt_var_free(node);
        node = next;
    }
    ctx->label_num = 0;
}

void zt_context_ensure_var(zt_context_t *ctx, const msu_str_t *name) {
    zt_var_t *node = ctx->vhead;
    while (node) {
        if (msu_str_eq(name, node->name)) return;
        node = node->next;
    }

    zt_var_t *new_node = malloc(sizeof(zt_var_t));
    new_node->name = name;
    new_node->value = NULL;
    new_node->next = NULL;
    if (!ctx->vhead) {
        ctx->vhead = new_node;
    } else {
        ctx->vtail->next = new_node;
    }
    ctx->vtail = new_node;
}

const msu_str_t *zt_context_ensure_val(zt_context_t *vars, token *val) {
    zt_var_t *node = vars->vhead;
    const msu_str_t *name = msu_str_printf("$%s", msu_str_data(val->content));
    while (node) {
        if (msu_str_eq(name, node->name)) return name;
        node = node->next;
    }

    zt_var_t *new_node = malloc(sizeof(zt_var_t));
    new_node->name = name;
    new_node->value = val;
    new_node->next = NULL;
    if (!vars->vhead) {
        vars->vhead = new_node;
    } else {
        vars->vtail->next = new_node;
    }
    vars->vtail = new_node;
    return name;
}

void zt_compile_impl(parsenode_t *node, zt_context_t *ctx, msu_str_builder_t sb) {
    if (node->kind == ZT_PROGRAM) {
        for (size_t i = 0; i < node->children->len; i++) {
            parsenode_t *child = list_of_parsenodes_get(node->children, i);
            zt_compile_impl(child, ctx, sb);
        }
        msu_str_builder_pushs(sb, "HLT\n");
        zt_var_t *var = ctx->vhead;
        while (var) {
            const msu_str_t *name = var->name;
            const char *value = var->value ? msu_str_data(var->value->content) : "0";
            msu_str_builder_printf(sb, "%s DAT %s\n", msu_str_data(name), value);
            var = var->next;
        }
    } else if (node->kind == ZT_BLOCK) {
        for (size_t i = 0; i < node->children->len; i++) {
            parsenode_t *child = list_of_parsenodes_get(node->children, i);
            zt_compile_impl(child, ctx, sb);
        }
    } else if (node->kind == ZT_ASSIGN) {
        zt_context_ensure_var(ctx, node->token->content);
        zt_compile_impl(list_of_parsenodes_get(node->children, 0), ctx, sb);
        msu_str_builder_printf(sb, "STA %s\n", msu_str_data(node->token->content));
    } else if (node->kind == ZT_WHILE) {
        int label_num = ctx->label_num++;
        const msu_str_t *head_label = msu_str_printf("_$head%d", label_num);
        const msu_str_t *body_label = msu_str_printf("_$body%d", label_num);
        const msu_str_t *end_label = msu_str_printf("_$end%d", label_num);

        msu_str_builder_printf(sb, "%s\n", msu_str_data(head_label));

        parsenode_t *cond = list_of_parsenodes_get(node->children, 0);
        parsenode_t *lhs = list_of_parsenodes_get(cond->children, 0);
        parsenode_t *rhs = list_of_parsenodes_get(cond->children, 1);

        bool lhs_simple = lhs->kind == ZT_VAR;
        bool rhs_simple = rhs->kind == ZT_VAR || rhs->kind == ZT_INT;

        if (lhs_simple && rhs_simple) {
            zt_context_ensure_var(ctx, lhs->token->content);
            msu_str_builder_printf(sb, "LDA %s\n", msu_str_data(lhs->token->content));

            if (rhs->kind == ZT_INT) {
                const msu_str_t *val = zt_context_ensure_val(ctx, rhs->token);
                msu_str_builder_printf(sb, "SUB %s\n", msu_str_data(val));
            } else {
                zt_context_ensure_var(ctx, rhs->token->content);
                msu_str_builder_printf(sb, "SUB %s\n", msu_str_data(rhs->token->content));
            }
        } else {
            const msu_str_t *tmpL = msu_str_printf("$tmp%d", ctx->label_num++);
            zt_compile_impl(lhs, ctx, sb);
            msu_str_builder_printf(sb, "STA %s\n", msu_str_data(tmpL));
            zt_context_ensure_var(ctx, tmpL);

            const msu_str_t *tmpR = msu_str_printf("$tmp%d", ctx->label_num++);
            zt_compile_impl(rhs, ctx, sb);
            msu_str_builder_printf(sb, "STA %s\n", msu_str_data(tmpR));
            zt_context_ensure_var(ctx, tmpR);

            msu_str_builder_printf(sb, "LDA %s\n", msu_str_data(tmpL));
            msu_str_builder_printf(sb, "SUB %s\n", msu_str_data(tmpR));
        }

        const char *insr = NULL;
        if (msu_str_eqs(cond->token->content, ">=")) {
            insr = "BRP";
        } else if (msu_str_eqs(cond->token->content, "=")) {
            insr = "BRZ";
        } else {
            msu_str_printf("error: invalid op %s", cond->token->content);
        }

        msu_str_builder_printf(sb, "%s %s\n", insr, msu_str_data(body_label));
        msu_str_builder_printf(sb, "BRA %s\n", msu_str_data(end_label));

        // --- FIXED THESE TWO LINES ---
        msu_str_builder_printf(sb, "%s\nADD 0\n", msu_str_data(body_label));
        parsenode_t *block = list_of_parsenodes_get(node->children, 1);
        zt_compile_impl(block, ctx, sb);
        msu_str_builder_printf(sb, "BRA %s\n", msu_str_data(head_label));
        msu_str_builder_printf(sb, "%s\nADD 0\n", msu_str_data(end_label));
    } else if (node->kind == ZT_INT) {
        const msu_str_t *label = zt_context_ensure_val(ctx, node->token);
        msu_str_builder_printf(sb, "LDA %s\n", msu_str_data(label));
    } else if (node->kind == ZT_WRITE) {
        parsenode_t *value = list_of_parsenodes_get(node->children, 0);
        zt_compile_impl(value, ctx, sb);
        msu_str_builder_pushs(sb, "OUT\n");
    } else if (node->kind == ZT_VAR) {
        zt_context_ensure_var(ctx, node->token->content);
        msu_str_builder_printf(sb, "LDA %s\n", msu_str_data(node->token->content));
    } else if (node->kind == ZT_OP) {
        parsenode_t *lhs = list_of_parsenodes_get(node->children, 0);
        parsenode_t *rhs = list_of_parsenodes_get(node->children, 1);

        if (lhs->kind == ZT_VAR && rhs->kind == ZT_VAR) {
            zt_context_ensure_var(ctx, lhs->token->content);
            zt_context_ensure_var(ctx, rhs->token->content);
            msu_str_builder_printf(sb, "LDA %s\n", msu_str_data(lhs->token->content));
            if (msu_str_eqs(node->token->content, "+")) {
                msu_str_builder_printf(sb, "ADD %s\n", msu_str_data(rhs->token->content));
            } else if (msu_str_eqs(node->token->content, "-")) {
                msu_str_builder_printf(sb, "SUB %s\n", msu_str_data(rhs->token->content));
            }
        } else {
            zt_compile_impl(lhs, ctx, sb);
            const msu_str_t *tmp = msu_str_printf("$tmp%d", ctx->label_num++);
            msu_str_builder_printf(sb, "STA %s\n", msu_str_data(tmp));
            zt_context_ensure_var(ctx, tmp);
            zt_compile_impl(rhs, ctx, sb);
            if (msu_str_eqs(node->token->content, "+")) {
                msu_str_builder_printf(sb, "ADD %s\n", msu_str_data(tmp));
            } else if (msu_str_eqs(node->token->content, "-")) {
                msu_str_builder_printf(sb, "SUB %s\n", msu_str_data(tmp));
            }
        }
    } else if (node->kind == ZT_READ) {
        msu_str_builder_printf(sb, "INP\n");
    } else {
        printf("Unknown parse element type: %d\n", node->kind);
    }
}
const msu_str_t *zt_compile(parsenode_t *node) {
    msu_str_builder_t sb = msu_str_builder_new();
    zt_context_t ctx = {
        .vhead = NULL,
        .vtail = NULL,
        .label_num = 0,
    };
    zt_compile_impl(node, &ctx, sb);
    zt_context_free(&ctx);

    return msu_str_builder_into_string_and_free(sb);
}
