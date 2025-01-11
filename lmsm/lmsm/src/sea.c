#include "lmsm/sea.h"

token *sea_tokenize(const msu_str_t *src) {
    return tokenize(src, STRLIT("//"));
}

parser sea_parser(token *tokens) {
    static strlit KW_DO = STRLIT_VAL("do");
    static strlit KW_WHILE = STRLIT_VAL("while");
    static strlit KW_FOR = STRLIT_VAL("for");
    static strlit KW_IF = STRLIT_VAL("if");
    static strlit KW_ELSE = STRLIT_VAL("else");
    static strlit KW_RETURN = STRLIT_VAL("return");
    static strlit KW_INT = STRLIT_VAL("int");
    static strlit KW_VOID = STRLIT_VAL("void");
    static const strlit *KEYWORDS[] = {&KW_DO, &KW_WHILE, &KW_FOR, &KW_IF, &KW_ELSE, &KW_RETURN,
                                       &KW_INT, &KW_VOID};
    static const size_t N_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);
    static list_of_msu_strs_t LIST_OF_KWS = {(const msu_str_t **) KEYWORDS, N_KEYWORDS, 0};

    return (parser) {
            .root = tokens,
            .current = tokens,
            .keywords = &LIST_OF_KWS,
    };
}

parsenode_t *sea_parse_decl(parser *pk);

parsenode_t *sea_parse_stmt_impl(parser *pk);

parsenode_t *sea_parse_block(parser *pk);

parsenode_t *sea_parse_for(parser *pk);

parsenode_t *sea_parse_while(parser *pk);

parsenode_t *sea_parse_do_while(parser *pk);

parsenode_t *sea_parse_if(parser *pk);

parsenode_t *sea_parse_return(parser *pk);

parsenode_t *sea_parse_var(parser *pk);

parsenode_t *sea_parse_expr_impl(parser *pk);

parsenode_t *sea_parse(const msu_str_t *src) {
    token *tokens = sea_tokenize(src);
    parser pk = sea_parser(tokens);

    parsenode_t *pgm = parsenode_new(SEA_PROGRAM, NULL);
    while (parser_has_next(&pk)) {
        parsenode_t *child = sea_parse_decl(&pk);
        if (!child) {
            child = parsenode_new(SEA_ERROR, parser_take(&pk));
            parsenode_set_error(child, msu_str_new("expected function or global variable"));
        }
        parsenode_add_child(pgm, child);
    }

    return pgm;
}

parsenode_t *sea_parse_stmt(const msu_str_t *src) {
    token *tokens = sea_tokenize(src);
    parser pk = sea_parser(tokens);
    parsenode_t *out = sea_parse_stmt_impl(&pk);
    tokens_free(tokens);
    return out;
}

parsenode_t *sea_parse_expr(const msu_str_t *src) {
    token *tokens = sea_tokenize(src);
    parser pk = sea_parser(tokens);
    parsenode_t *out = sea_parse_expr_impl(&pk);
    tokens_free(tokens);
    return out;
}

parsenode_t *sea_parse_decl(parser *pk) {
    if (!parser_peek_kw(pk, "int") && !parser_peek_kw(pk, "void")) {
        return NULL;
    }

    parsenode_t *type = parsenode_new(SEA_TYPE, parser_take(pk));

    token *name;
    if (!parser_peek_word(pk)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected name after type"));
        return err;
    } else {
        name = parser_take(pk);
    }

    parsenode_t *decl;
    if (parser_take_kind(pk, TT_LPAREN)) {
        decl = parsenode_new(SEA_FUNCDEF, name);
        parsenode_add_child(decl, type);

        parsenode_t *params = parsenode_new(SEA_PARAMS, NULL);
        parsenode_add_child(decl, params);
        while (parser_has_next(pk) && !parser_peek_kind(pk, TT_RPAREN)) {
            parsenode_t *param = parsenode_new(SEA_PARAM, NULL);
            parsenode_add_child(params, param);

            if (parser_peek_kw(pk, "int")) {
                parsenode_t *type = parsenode_new(SEA_TYPE, parser_take(pk));
                parsenode_add_child(param, type);
            } else {
                parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
                parsenode_set_error(err, msu_str_new("expected parameter type"));
                parsenode_add_child(param, err);
            }

            if (parser_peek_word(pk) != NULL) {
                parsenode_t *argname = parsenode_new(SEA_IDENT, parser_take(pk));
                parsenode_add_child(param, argname);
            } else {
                parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
                parsenode_set_error(err, msu_str_new("expected variable name"));
                parsenode_add_child(param, err);
            }

            if (!parser_take_kind(pk, TT_COMMA)) {
                break;
            }
        }

        if (!parser_take_kind(pk, TT_RPAREN)) {
            parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
            parsenode_set_error(err, msu_str_new("expected ')' after function params"));
            parsenode_add_child(decl, err);
            return decl;
        }

        parsenode_t *block = sea_parse_block(pk);
        if (!block) {
            block = parsenode_new(SEA_ERROR, parser_take(pk));
            parsenode_set_error(block, msu_str_new("expected function body"));
        }
        parsenode_add_child(decl, block);
    } else if (parser_peek_kind(pk, TT_SEMICOLON) || parser_peek_kind(pk, TT_EQ)) {
        parsenode_t *value;
        if (parser_take_kind(pk, TT_EQ)) {
            value = sea_parse_expr_impl(pk);
            if (!value) {
                value = parsenode_new(SEA_ERROR, parser_take(pk));
                parsenode_set_error(value, msu_str_new("expected variable value"));
            }
        } else {
            value = NULL;
        }

        decl = parsenode_new(SEA_VAR, name);

        if (!parser_take_kind(pk, TT_SEMICOLON)) {
            parsenode_set_error(decl, msu_str_new("expected ';' after variable"));
        }

        parsenode_add_child(decl, type);
        parsenode_add_child(decl, value);
    } else {
        decl = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(decl, msu_str_new("expected variable of function declaration"));
    }

    return decl;
}

parsenode_t *sea_parse_stmt_impl(parser *pk) {
    parsenode_t *out;

    if ((out = sea_parse_block(pk))) {
        return out;
    }

    if ((out = sea_parse_for(pk))) {
        return out;
    }

    if ((out = sea_parse_while(pk))) {
        return out;
    }

    if ((out = sea_parse_do_while(pk))) {
        if (!parser_take_kind(pk, TT_SEMICOLON)) {
            parsenode_set_error(out, msu_str_new("expected ';' after variable"));
        }
        return out;
    }

    if ((out = sea_parse_if(pk))) {
        return out;
    }

    if ((out = sea_parse_return(pk))) {
        return out;
    }

    if ((out = sea_parse_var(pk))) {
        if (!parser_take_kind(pk, TT_SEMICOLON)) {
            parsenode_set_error(out, msu_str_new("expected ';' after variable"));
        }
        return out;
    }

    if ((out = sea_parse_expr_impl(pk))) {
        if (!parser_take_kind(pk, TT_SEMICOLON)) {
            parsenode_set_error(out, msu_str_new("expected ';' after expression"));
        }
        return out;
    }

    return NULL;
}

parsenode_t *sea_parse_block(parser *pk) {
    if (!parser_take_kind(pk, TT_LBRACE)) {
        return NULL;
    }

    parsenode_t *out = parsenode_new(SEA_BLOCK, NULL);
    while (parser_has_next(pk) && !parser_peek_kind(pk, TT_RBRACE)) {
        parsenode_t *stmt = sea_parse_stmt_impl(pk);
        if (!stmt) {
            stmt = parsenode_new(SEA_ERROR, parser_take(pk));
            parsenode_set_error(stmt, msu_str_new("expected statement"));
        }
        parsenode_add_child(out, stmt);
    }

    if (!parser_take_kind(pk, TT_RBRACE)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected '}' at end of block"));
        parsenode_add_child(out, err);
        return out;
    }

    return out;
}

parsenode_t *sea_parse_expr_impl_or_var(parser *pk) {
    parsenode_t *out;

    if ((out = sea_parse_var(pk))) {
        return out;
    }

    return sea_parse_expr_impl(pk);
}

parsenode_t *sea_parse_for(parser *pk) {
    if (!parser_take_kw(pk, "for")) {
        return NULL;
    }
    parsenode_t *out = parsenode_new(SEA_FOR, NULL);

    if (!parser_take_kind(pk, TT_LPAREN)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected '(' after 'for'"));
        parsenode_add_child(out, err);
        return out;
    }

    parsenode_t *init, *cond, *incr;

    init = sea_parse_expr_impl_or_var(pk);
    parsenode_add_child(out, init);

    if (!parser_take_kind(pk, TT_SEMICOLON)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected ';' after for-loop initializer"));
        parsenode_add_child(out, err);
        return out;
    }

    cond = sea_parse_expr_impl(pk);
    parsenode_add_child(out, cond);

    if (!parser_take_kind(pk, TT_SEMICOLON)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected ';' after for-loop condition"));
        parsenode_add_child(out, err);
        return out;
    }

    incr = sea_parse_expr_impl(pk);
    parsenode_add_child(out, incr);

    if (!parser_take_kind(pk, TT_RPAREN)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected ')' for 'for' loop"));
        parsenode_add_child(out, err);
        return out;
    }

    parsenode_t *body = sea_parse_stmt_impl(pk);
    if (!body) {
        body = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(body, msu_str_new("expected for-loop body"));
    }
    parsenode_add_child(out, body);

    return out;
}

parsenode_t *sea_parse_while(parser *pk) {
    if (!parser_take_kw(pk, "while")) {
        return NULL;
    }

    parsenode_t *out = parsenode_new(SEA_WHILE, NULL);

    if (!parser_take_kind(pk, TT_LPAREN)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected '(' after 'while'"));
        parsenode_add_child(out, err);
        return out;
    }

    parsenode_t *cond = sea_parse_expr_impl(pk);
    if (!cond) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected 'while' condition"));
        parsenode_add_child(out, err);
        return out;
    }
    parsenode_add_child(out, cond);

    if (!parser_take_kind(pk, TT_RPAREN)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected ')' after 'while' condition"));
        parsenode_add_child(out, err);
        return out;
    }

    parsenode_t *body = sea_parse_stmt_impl(pk);
    if (!body) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected 'while' body"));
        parsenode_add_child(out, err);
        return out;
    }
    parsenode_add_child(out, body);

    return out;
}


parsenode_t *sea_parse_do_while(parser *pk) {
    if (!parser_take_kw(pk, "do")) {
        return NULL;
    }

    parsenode_t *out = parsenode_new(SEA_DO_WHILE, NULL);

    parsenode_t *body = sea_parse_stmt_impl(pk);
    if (!body) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected 'do-while' body"));
        parsenode_add_child(out, err);
        return out;
    }
    parsenode_add_child(out, body);

    if (!parser_take_kw(pk, "while")) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected 'while' after 'do' body"));
        parsenode_add_child(out, err);
        return out;
    }

    if (!parser_take_kind(pk, TT_LPAREN)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected '(' for do-while condition"));
        parsenode_add_child(out, err);
        return out;
    }

    parsenode_t *cond = sea_parse_expr_impl(pk);
    if (!cond) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected do-while condition"));
        parsenode_add_child(out, err);
        return out;
    }
    parsenode_add_child(out, cond);

    if (!parser_take_kind(pk, TT_RPAREN)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected ')' after do-while condition"));
        parsenode_add_child(out, err);
        return out;
    }

    return out;
}

parsenode_t *sea_parse_if(parser *pk) {
    if (!parser_take_kw(pk, "if")) {
        return NULL;
    }

    parsenode_t *out = parsenode_new(SEA_IF, NULL);

    if (!parser_take_kind(pk, TT_LPAREN)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected '(' after 'if'"));
        parsenode_add_child(out, err);
        return out;
    }

    parsenode_t *cond = sea_parse_expr_impl(pk);
    if (!cond) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected 'if' condition"));
        parsenode_add_child(out, err);
        return out;
    }
    parsenode_add_child(out, cond);

    if (!parser_take_kind(pk, TT_RPAREN)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected ')' after 'if' condition"));
        parsenode_add_child(out, err);
        return out;
    }

    parsenode_t *body = sea_parse_stmt_impl(pk);
    if (!body) {
        body = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(body, msu_str_new("expected 'if' body"));
        parsenode_add_child(out, body);
        return out;
    }
    parsenode_add_child(out, body);

    parsenode_t *else_block;
    if (!parser_take_kw(pk, "else")) {
        else_block = NULL;
    } else {
        else_block = sea_parse_stmt_impl(pk);
        if (!else_block) {
            else_block = parsenode_new(SEA_ERROR, parser_take(pk));
            parsenode_set_error(else_block, msu_str_new("expected 'else' body"));
            parsenode_add_child(out, else_block);
            return out;
        }
    }
    parsenode_add_child(out, else_block);

    return out;
}

parsenode_t *sea_parse_return(parser *pk) {
    if (!parser_take_kw(pk, "return")) return NULL;

    parsenode_t *out = parsenode_new(SEA_RETURN, NULL);

    parsenode_t *value = NULL;
    if (!parser_peek_kind(pk, TT_SEMICOLON)) {
        value = sea_parse_expr_impl(pk);
        if (!value) {
            value = parsenode_new(SEA_ERROR, parser_take(pk));
            parsenode_set_error(value, msu_str_new("expected return value"));
        }
    }
    parsenode_add_child(out, value);

    if (!parser_take_kind(pk, TT_SEMICOLON)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected ';' after return"));
        parsenode_add_child(out, err);
        return out;
    }

    return out;
}

parsenode_t *sea_parse_var(parser *pk) {
    if (!parser_peek_kw(pk, "int") && !parser_peek_kw(pk, "void")) {
        return NULL;
    }
    parsenode_t *type = parsenode_new(SEA_TYPE, parser_take(pk));

    token *name;
    if (!parser_peek_word(pk)) {
        parsenode_t *err = parsenode_new(SEA_ERROR, parser_take(pk));
        parsenode_set_error(err, msu_str_new("expected variable name"));
        return err;
    } else {
        name = parser_take(pk);
    }

    parsenode_t *value;
    if (!parser_take_kind(pk, TT_EQ)) {
        value = NULL;
    } else {
        value = sea_parse_expr_impl(pk);
        if (!value) {
            value = parsenode_new(SEA_ERROR, parser_take(pk));
            parsenode_set_error(value, msu_str_new("expected variable value"));
        }
    }

    parsenode_t *var = parsenode_new(SEA_VAR, name);
    parsenode_add_child(var, type);
    parsenode_add_child(var, value);

    return var;
}

parsenode_t *sea_parse_assign(parser *pk);

parsenode_t *sea_parse_cmp(parser *pk);

parsenode_t *sea_parse_equality(parser *pk);

parsenode_t *sea_parse_term(parser *pk);

parsenode_t *sea_parse_factor(parser *pk);

parsenode_t *sea_parse_call(parser *pk);

parsenode_t *sea_parse_unary(parser *pk);

parsenode_t *sea_parse_primary(parser *pk);

parsenode_t *sea_parse_expr_impl(parser *pk) {
    return sea_parse_assign(pk);
}

parsenode_t *sea_parse_assign(parser *pk) {
    parsenode_t *out = sea_parse_cmp(pk);
    if (!out) return NULL;

    while (parser_peek_kind(pk, TT_EQ)) {
        parsenode_t *expr = parsenode_new(SEA_BINARY, parser_take(pk));
        parsenode_add_child(expr, out);

        parsenode_t *rhs = sea_parse_cmp(pk);
        if (!rhs) {
            rhs = parsenode_new(TT_ERROR, parser_take(pk));
            parsenode_set_error(rhs, msu_str_new("expected expression"));
        }
        parsenode_add_child(expr, rhs);

        out = expr;
    }

    return out;
}

parsenode_t *sea_parse_cmp(parser *pk) {
    parsenode_t *out = sea_parse_equality(pk);
    if (!out) return NULL;

    while (parser_peek_kind(pk, TT_LARROW) || parser_peek_kind(pk, TT_LARROW_EQ)
           || parser_peek_kind(pk, TT_RARROW) || parser_peek_kind(pk, TT_RARROW_EQ)) {
        parsenode_t *expr = parsenode_new(SEA_BINARY, parser_take(pk));
        parsenode_add_child(expr, out);

        parsenode_t *rhs = sea_parse_equality(pk);
        if (!rhs) {
            rhs = parsenode_new(TT_ERROR, parser_take(pk));
            parsenode_set_error(rhs, msu_str_new("expected expression"));
        }
        parsenode_add_child(expr, rhs);

        out = expr;
    }

    return out;
}

parsenode_t *sea_parse_equality(parser *pk) {
    parsenode_t *out = sea_parse_term(pk);
    if (!out) return NULL;

    while (parser_peek_kind(pk, TT_EQ2) || parser_peek_kind(pk, TT_BANG_EQ)) {
        parsenode_t *expr = parsenode_new(SEA_BINARY, parser_take(pk));
        parsenode_add_child(expr, out);

        parsenode_t *rhs = sea_parse_term(pk);
        if (!rhs) {
            rhs = parsenode_new(TT_ERROR, parser_take(pk));
            parsenode_set_error(rhs, msu_str_new("expected expression"));
        }
        parsenode_add_child(expr, rhs);

        out = expr;
    }

    return out;
}

parsenode_t *sea_parse_term(parser *pk) {
    parsenode_t *out = sea_parse_factor(pk);
    if (!out) return NULL;

    while (parser_peek_kind(pk, TT_PLUS) || parser_peek_kind(pk, TT_DASH)) {
        parsenode_t *expr = parsenode_new(SEA_BINARY, parser_take(pk));
        parsenode_add_child(expr, out);

        parsenode_t *rhs = sea_parse_factor(pk);
        if (!rhs) {
            rhs = parsenode_new(TT_ERROR, parser_take(pk));
            parsenode_set_error(rhs, msu_str_new("expected expression"));
        }
        parsenode_add_child(expr, rhs);

        out = expr;
    }

    return out;
}

parsenode_t *sea_parse_factor(parser *pk) {
    parsenode_t *out = sea_parse_unary(pk);
    if (!out) return NULL;

    while (parser_peek_kind(pk, TT_STAR) || parser_peek_kind(pk, TT_SLASH)) {
        parsenode_t *expr = parsenode_new(SEA_BINARY, parser_take(pk));
        parsenode_add_child(expr, out);

        parsenode_t *rhs = sea_parse_factor(pk);
        if (!rhs) {
            rhs = parsenode_new(TT_ERROR, parser_take(pk));
            parsenode_set_error(rhs, msu_str_new("expected expression"));
        }
        parsenode_add_child(expr, rhs);

        out = expr;
    }

    return out;
}

parsenode_t *sea_parse_unary(parser *pk) {
    if (parser_peek_kind(pk, TT_DASH) || parser_peek_kind(pk, TT_BANG)) {
        parsenode_t *expr = parsenode_new(SEA_UNARY, parser_take(pk));
        parsenode_t *child = sea_parse_call(pk);
        parsenode_add_child(expr, child);
        return expr;
    } else {
        return sea_parse_call(pk);
    }
}

parsenode_t *sea_parse_call(parser *pk) {
    parsenode_t *out = sea_parse_primary(pk);
    if (!out) return NULL;

    if (parser_take_kind(pk, TT_LPAREN)) {
        parsenode_t *args = parsenode_new(SEA_ARGS, NULL);
        while (parser_has_next(pk) && !parser_peek_kind(pk, TT_RPAREN)) {
            parsenode_t *value = sea_parse_expr_impl(pk);
            if (!value) {
                value = parsenode_new(SEA_ERROR, parser_take(pk));
                parsenode_set_error(value, msu_str_new("expected argument value"));
            }
            parsenode_add_child(args, value);

            if (!parser_take_kind(pk, TT_COMMA)) {
                break;
            }
        }

        if (!parser_take_kind(pk, TT_RPAREN)) {
            parsenode_set_error(out, msu_str_new("expected ')' after call arguments"));
        }

        parsenode_t *expr = parsenode_new(SEA_CALL, NULL);
        parsenode_add_child(expr, out);
        parsenode_add_child(expr, args);
        out = expr;
    }

    return out;
}

parsenode_t *sea_parse_primary(parser *pk) {
    if (parser_peek_kind(pk, TT_INT)) {
        parsenode_t *out = parsenode_new(SEA_INT, parser_take(pk));
        return out;
    }

    if (parser_peek_word(pk)) {
        parsenode_t *out = parsenode_new(SEA_IDENT, parser_take(pk));
        return out;
    }

    if (parser_take_kind(pk, TT_LPAREN)) {
        parsenode_t *out = sea_parse_expr_impl(pk);
        if (!out) out = parsenode_new(SEA_ERROR, parser_take(pk));
        if (!parser_take_kind(pk, TT_RPAREN)) {
            parsenode_set_error(out, msu_str_new("expected ')' for grouped expression"));
        }
        return out;
    }

    return NULL;
}

typedef struct var {
    const msu_str_t *name;
    size_t frame_offset;
    bool is_global;
    struct var *next;
} var_t;

typedef struct scope {
    var_t *vars;
    struct scope *parent;
} scope_t;

void var_free(var_t *var) {
    if (var) {
        msu_str_free(var->name);
        free(var);
    }
}

void vars_free(var_t *vars) {
    while (vars) {
        var_t *next = vars->next;
        var_free(vars);
        vars = next;
    }
}

void scope_free(scope_t *scope) {
    if (scope) {
        vars_free(scope->vars);
    }
}

var_t *scope_add_var(scope_t *scope, const msu_str_t *name, size_t offset, bool is_global) {
    var_t *var = malloc(sizeof(var_t));
    var->name = msu_str_clone(name);
    var->frame_offset = offset;
    var->next = scope->vars;
    var->is_global = is_global;
    scope->vars = var;
    return var;
}

var_t *scope_find_var(scope_t *scope, const msu_str_t *name) {
    while (scope) {
        var_t *var = scope->vars;
        while (var) {
            if (msu_str_eq(var->name, name)) {
                return var;
            }
            var = var->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

void scope_push(scope_t **scope) {
    scope_t *next = malloc(sizeof(scope_t));
    next->vars = NULL;
    next->parent = *scope;
    *scope = next;
}

void scope_pop(scope_t **scope) {
    if (*scope) {
        scope_t *next = (*scope)->parent;
        scope_free(*scope);
        *scope = next;
    }
}

typedef struct constval {
    const msu_str_t *label;
    int value;
    struct constval *next;
} constval_t;

typedef struct sea_compile_ctx {
    list_of_parsenodes_t *functions;
    list_of_parsenodes_t *globals;
    constval_t *constants;
    int labelno;
    scope_t *scope;
    size_t max_var_offset;
    size_t imm_offset;
    size_t var_offset;
    const msu_str_t *return_label;
} sea_compile_ctx;

sea_compile_ctx sea_compile_ctx_new() {
    return (sea_compile_ctx) {
            .functions = list_of_parsenodes_new(),
            .globals = list_of_parsenodes_new(),
            .constants = NULL,
            .labelno = 0,
            .scope = NULL,
            .max_var_offset = 0,
            .imm_offset = 0,
            .var_offset = 0,
            .return_label = NULL,
    };
}

void sea_compile_ctx_add_var(sea_compile_ctx *me, const msu_str_t *name, size_t frame_offset, bool is_global) {
    scope_add_var(me->scope, name, frame_offset, is_global);
}

size_t sea_compile_ctx_get_var_offset(sea_compile_ctx *me, const msu_str_t *name) {
    return me->imm_offset + scope_find_var(me->scope, name)->frame_offset;
}

bool sea_compile_ctx_add_global(sea_compile_ctx *me, parsenode_t *node) {
    for (int i = 0; i < me->globals->len; ++i) {
        parsenode_t *func = list_of_parsenodes_get(me->globals, i);
        if (msu_str_eq(func->token->content, node->token->content)) {
            return false;
        }
    }
    list_of_parsenodes_append(me->globals, node);
    return true;
}

bool sea_compile_ctx_add_function(sea_compile_ctx *me, parsenode_t *node) {
    for (int i = 0; i < me->functions->len; ++i) {
        parsenode_t *func = list_of_parsenodes_get(me->functions, i);
        if (msu_str_eq(func->token->content, node->token->content)) {
            return false;
        }
    }
    list_of_parsenodes_append(me->functions, node);
    return true;
}

const msu_str_t *sea_compile_ctx_ensure_constant(sea_compile_ctx *me, int value) {
    constval_t *val = me->constants;
    while (val) {
        if (val->value) return val->label;
        val = val->next;
    }

    val = malloc(sizeof(constval_t));
    val->label = msu_str_printf("$%d", value);
    val->value = value;
    val->next = me->constants;
    me->constants = val;
    return val->label;
}

const parsenode_t *sea_compile_ctx_find_function(sea_compile_ctx *ctx, const msu_str_t *name) {
    for (int i = 0; i < ctx->functions->len; ++i) {
        const parsenode_t *func = list_of_parsenodes_get(ctx->functions, i);
        if (msu_str_eq(func->token->content, name)) {
            return func;
        }
    }
    return NULL;
}

bool sea_function_always_returns(const parsenode_t *node) {
    if (node->kind == SEA_FUNCDEF) {
        parsenode_t *block = list_of_parsenodes_get(node->children, 2);
        return sea_function_always_returns(block);
    } else if (node->kind == SEA_BLOCK) {
        for (int i = 0; i < node->children->len; ++i) {
            if (sea_function_always_returns(list_of_parsenodes_get(node->children, i))) {
                return true;
            }
        }
        return false;
    } else if (node->kind == SEA_IF) {
        parsenode_t *then_body = list_of_parsenodes_get(node->children, 1);
        parsenode_t *else_body = list_of_parsenodes_get(node->children, 2);

        bool then_returns = sea_function_always_returns(then_body);
        bool else_returns = else_body == NULL ? false : sea_function_always_returns(else_body);
        return then_returns && else_returns;
    } else if (node->kind == SEA_FOR) {
        parsenode_t *body = list_of_parsenodes_get(node->children, 3);
        return sea_function_always_returns(body);
    } else if (node->kind == SEA_WHILE) {
        parsenode_t *body = list_of_parsenodes_get(node->children, 1);
        return sea_function_always_returns(body);
    } else if (node->kind == SEA_DO_WHILE) {
        parsenode_t *body = list_of_parsenodes_get(node->children, 0);
        return sea_function_always_returns(body);
    } else if (node->kind == SEA_RETURN) {
        return true;
    } else {
        return false;
    }
}

size_t sea_count_max_locals(const parsenode_t *node) {
    if (node->kind == SEA_BLOCK) {
        size_t locals = 0;
        for (int i = 0; i < node->children->len; ++i) {
            const parsenode_t *child = list_of_parsenodes_get_const(node->children, i);
            locals += sea_count_max_locals(child);
        }
        return locals;
    }

    if (node->kind == SEA_IF) {
        const parsenode_t *then_body = list_of_parsenodes_get_const(node->children, 1);
        const parsenode_t *else_body = list_of_parsenodes_get_const(node->children, 2);

        size_t then_body_size = sea_count_max_locals(then_body);
        size_t else_body_size = else_body == NULL ? 0 : sea_count_max_locals(else_body);
        return then_body_size + else_body_size;
    }

    if (node->kind == SEA_FOR) {
        size_t locals = 0;

        const parsenode_t *init = list_of_parsenodes_get(node->children, 0);
        if (init) locals += 1;

        const parsenode_t *body = list_of_parsenodes_get(node->children, 3);
        locals += sea_count_max_locals(body);

        return locals;
    }

    if (node->kind == SEA_WHILE) {
        const parsenode_t *body = list_of_parsenodes_get(node->children, 1);
        return sea_count_max_locals(body);
    }

    if (node->kind == SEA_DO_WHILE) {
        const parsenode_t *body = list_of_parsenodes_get(node->children, 0);
        return sea_count_max_locals(body);
    }

    if (node->kind == SEA_VAR) {
        return 1;
    }

    return 0;
}

const sea_error_t SEA_ERROR_NONE = {NULL, NULL};

sea_error_t *sea_error_new(const parsenode_t *node, const msu_str_t *msg) {
    sea_error_t *out = malloc(sizeof(sea_error_t));
    assert(out && "out of memory!\n");
    out->node = node;
    out->message = msg;
    return out;
}

void sea_compile_impl(const parsenode_t *node, sea_compile_ctx *ctx, msu_str_builder_t out, sea_error_t **errout) {
    if (node->kind == SEA_PROGRAM) {
        msu_str_builder_pushs(out, "CALL main\nHLT\n");

        for (int i = 0; i < ctx->globals->len; i++) {
            parsenode_t *global = list_of_parsenodes_get(ctx->globals, i);
            parsenode_print(global);
            assert(false && "unimplemented");
        }

        for (int i = 0; i < ctx->functions->len; ++i) {
            parsenode_t *func = list_of_parsenodes_get(ctx->functions, i);

            if (!sea_function_always_returns(func)) {
                *errout = sea_error_new(func, msu_str_printf("function '%s' does not always return",
                                                             msu_str_data(func->token->content)));
                return;
            }

            msu_str_builder_printf(out, "%s ", msu_str_data(func->token->content));

            const parsenode_t *ret = list_of_parsenodes_get(func->children, 0);
            const parsenode_t *params = list_of_parsenodes_get(func->children, 1);
            const parsenode_t *body = list_of_parsenodes_get(func->children, 2);

            size_t locals = sea_count_max_locals(body);
            ctx->scope = NULL;
            scope_push(&ctx->scope);
            ctx->max_var_offset = locals;
            ctx->imm_offset = 0;
            ctx->var_offset = 0;
            ctx->return_label = msu_str_printf("$ret.%s", msu_str_data(func->token->content));

            if (locals > 0) {
                msu_str_builder_printf(out, "SPSUB %zu\n", locals - 1);
            }

            for (int j = 0; j < params->children->len; ++j) {
                const parsenode_t *param = list_of_parsenodes_get(params->children, j);
                size_t offset = locals + params->children->len - j;
                scope_add_var(ctx->scope, param->token->content, offset, false);
            }

            sea_compile_impl(body, ctx, out, errout);
            if (*errout) return;

            msu_str_builder_printf(out, "%s ", msu_str_data(ctx->return_label));

            if (msu_str_eqs(ret->token->content, "int")) {
                msu_str_builder_printf(out, "SPOP\n");
                if (locals > 0) {
                    msu_str_builder_printf(out, "SPADD %zu\n", locals + params->children->len - 1);
                }
                ctx->imm_offset -= 1;
            } else {
                if (locals > 0) {
                    msu_str_builder_printf(out, "SPADD %zu\n", locals + params->children->len - 1);
                }
            }
            assert(ctx->imm_offset == 0);
            msu_str_builder_pushs(out, "RET\n");

            msu_str_free(ctx->return_label);
            scope_pop(&ctx->scope);
        }

        for (int i = 0; i < ctx->globals->len; i++) {
            parsenode_t *global = list_of_parsenodes_get(ctx->globals, i);
            const msu_str_t *name = global->token->content;
            msu_str_builder_printf(out, "$global.%s DAT 0\n", name);
        }
    } else if (node->kind == SEA_BLOCK) {
        scope_push(&ctx->scope);
        for (int i = 0; i < node->children->len; ++i) {
            const parsenode_t *child = list_of_parsenodes_get_const(node->children, i);
            sea_compile_impl(child, ctx, out, errout);
            if (*errout) return;
        }
        scope_pop(&ctx->scope);
    } else if (node->kind == SEA_FOR) {
        const parsenode_t *init = list_of_parsenodes_get(node->children, 0);
        const parsenode_t *cond = list_of_parsenodes_get(node->children, 1);
        const parsenode_t *incr = list_of_parsenodes_get(node->children, 2);
        const parsenode_t *body = list_of_parsenodes_get(node->children, 3);

        int labelno = ctx->labelno++;
        const msu_str_t *cont = msu_str_printf("$for.cond%d", labelno);
        const msu_str_t *end = msu_str_printf("$for.end%d", labelno);

        if (init) {
            sea_compile_impl(init, ctx, out, errout);
            if (*errout) return;
        }

        msu_str_builder_printf(out, "%s ", msu_str_data(cont));

        if (cond) {
            sea_compile_impl(cond, ctx, out, errout);
            if (*errout) return;
        } else {
            msu_str_builder_printf(out, "LDI 1\n");
        }

        msu_str_builder_printf(out, "SPOP\n");
        ctx->imm_offset -= 1;
        msu_str_builder_printf(out, "BRZ %s\n", msu_str_data(end));
        sea_compile_impl(body, ctx, out, errout);

        if (incr) {
            sea_compile_impl(incr, ctx, out, errout);
            if (*errout) return;
        }
        msu_str_builder_printf(out, "BRA %s\n", msu_str_data(cont));

        const msu_str_t *label0 = sea_compile_ctx_ensure_constant(ctx, 0);
        msu_str_builder_printf(out, "%s ADD %s\n", msu_str_data(end), msu_str_data(label0));

        msu_str_free(cont);
        msu_str_free(end);
    } else if (node->kind == SEA_WHILE) {
        const parsenode_t *cond = list_of_parsenodes_get(node->children, 0);
        const parsenode_t *body = list_of_parsenodes_get(node->children, 1);

        int labelno = ctx->labelno++;
        const msu_str_t *cont = msu_str_printf("$while.cond%d", labelno);
        const msu_str_t *end = msu_str_printf("$while.end%d", labelno);

        msu_str_builder_printf(out, "%s ", msu_str_data(cont));
        sea_compile_impl(cond, ctx, out, errout);
        if (*errout) return;
        msu_str_builder_printf(out, "SPOP\n");
        ctx->imm_offset -= 1;
        msu_str_builder_printf(out, "BRZ %s\n", msu_str_data(end));

        sea_compile_impl(body, ctx, out, errout);
        if (*errout) return;
        msu_str_builder_printf(out, "BRA %s\n", msu_str_data(cont));

        const msu_str_t *label0 = sea_compile_ctx_ensure_constant(ctx, 0);
        msu_str_builder_printf(out, "%s ADD %s\n", msu_str_data(end), msu_str_data(label0));

        msu_str_free(cont);
        msu_str_free(end);
    } else if (node->kind == SEA_DO_WHILE) {
        const parsenode_t *body = list_of_parsenodes_get(node->children, 0);
        const parsenode_t *cond = list_of_parsenodes_get(node->children, 1);

        int labelno = ctx->labelno++;
        const msu_str_t *start = msu_str_printf("$dowhile.start%d\n", labelno);
        const msu_str_t *end = msu_str_printf("$dowhile.end%d\n", labelno);

        const msu_str_t *label0 = sea_compile_ctx_ensure_constant(ctx, 0);
        msu_str_builder_printf(out, "%s ADD %s\n", msu_str_data(start), msu_str_data(label0));

        sea_compile_impl(body, ctx, out, errout);
        if (*errout) return;
        sea_compile_impl(cond, ctx, out, errout);
        if (*errout) return;

        msu_str_builder_printf(out, "SPOP\n");
        ctx->imm_offset -= 1;
        msu_str_builder_printf(out, "BRZ %s\n", msu_str_data(end));
        msu_str_builder_printf(out, "BRA %s\n", msu_str_data(start));

        msu_str_builder_printf(out, "%s ADD %s\n", msu_str_data(end), msu_str_data(label0));

        msu_str_free(start);
        msu_str_free(end);
    } else if (node->kind == SEA_IF) {
        const parsenode_t *cond = list_of_parsenodes_get(node->children, 0);
        const parsenode_t *then = list_of_parsenodes_get(node->children, 1);
        const parsenode_t *else_ = list_of_parsenodes_get(node->children, 2);

        int labelno = ctx->labelno++;
        const msu_str_t *end = msu_str_printf("$if.end%d", labelno);
        const msu_str_t *false_label = msu_str_printf("$if.false%d", labelno);

        sea_compile_impl(cond, ctx, out, errout);
        if (*errout) return;
        msu_str_builder_printf(out, "SPOP\n");
        ctx->imm_offset -= 1;
        msu_str_builder_printf(out, "BRZ %s\n", msu_str_data(false_label));
        sea_compile_impl(then, ctx, out, errout);
        if (*errout) return;

        const msu_str_t *label0 = sea_compile_ctx_ensure_constant(ctx, 0);
        if (else_) {
            msu_str_builder_printf(out, "BRA %s\n", end);

            msu_str_builder_printf(out, "%s ADD %s\n", msu_str_data(false_label), msu_str_data(label0));

            sea_compile_impl(else_, ctx, out, errout);
            if (*errout) return;

            msu_str_builder_printf(out, "%s ADD %s\n", msu_str_data(end), msu_str_data(label0));
        } else {
            msu_str_builder_printf(out, "%s ADD %s\n", msu_str_data(false_label), msu_str_data(label0));
        }

        msu_str_free(end);
        msu_str_free(false_label);
    } else if (node->kind == SEA_VAR) {
        size_t offset = ctx->var_offset++;
        var_t *var = scope_add_var(ctx->scope, node->token->content, offset, false);

        const parsenode_t *value = list_of_parsenodes_get(node->children, 1);
        if (value) {
            sea_compile_impl(value, ctx, out, errout);
            if (*errout) return;
//            msu_str_builder_printf(out, "SPOP\n");
            assert(ctx->imm_offset == 1);
            ctx->imm_offset -= 1;
            msu_str_builder_printf(out, "SSTA %zu\n", var->frame_offset);
        }
    } else if (node->kind == SEA_RETURN) {
        const parsenode_t *value = list_of_parsenodes_get(node->children, 0);
        if (value) {
            sea_compile_impl(value, ctx, out, errout);
            if (*errout) return;
        }

        msu_str_builder_printf(out, "BRA %s\n", msu_str_data(ctx->return_label));
    } else if (node->kind == SEA_BINARY) {
        const parsenode_t *lhs = list_of_parsenodes_get(node->children, 0);
        const parsenode_t *rhs = list_of_parsenodes_get(node->children, 1);

        if (msu_str_eqs(node->token->content, "=")) {
            if (lhs->kind != SEA_IDENT) {
                *errout = sea_error_new(lhs, msu_str_new("cannot assign to non-var"));
                return;
            }
            var_t *var = scope_find_var(ctx->scope, lhs->token->content);

            sea_compile_impl(rhs, ctx, out, errout);
            if (*errout) return;
//            msu_str_builder_printf(out, "SPOP\n");
            ctx->imm_offset -= 1;
            msu_str_builder_printf(out, "SSTA %zu\n", ctx->imm_offset + var->frame_offset);
            return;
        }

        sea_compile_impl(lhs, ctx, out, errout);
        if (*errout) return;
        sea_compile_impl(rhs, ctx, out, errout);
        if (*errout) return;
        if (msu_str_eqs(node->token->content, "+")) {
            msu_str_builder_printf(out, "SADD\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, "-")) {
            msu_str_builder_printf(out, "SSUB\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, "*")) {
            msu_str_builder_printf(out, "SMUL\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, "/")) {
            msu_str_builder_printf(out, "SDIV\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, ">")) {
            msu_str_builder_pushs(out, "SCMPGT\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, "<=")) {
            msu_str_builder_pushs(out, "SCMPGT\nSNOT\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, "<")) {
            msu_str_builder_pushs(out, "SCMPLT\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, ">=")) {
            msu_str_builder_pushs(out, "SCMPLT\nSNOT\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, "==")) {
            msu_str_builder_pushs(out, "SEQ\n");
            ctx->imm_offset -= 1;
        } else if (msu_str_eqs(node->token->content, "!=")) {
            msu_str_builder_pushs(out, "SEQ\nSNOT\n");
            ctx->imm_offset -= 1;
        } else {
            fprintf(stderr, "current op is '%s'\n", msu_str_data(node->token->content));
            assert(false && "operand unimplemented");
        }
    } else if (node->kind == SEA_UNARY) {
        const parsenode_t *operand = list_of_parsenodes_get(node->children, 0);

        if (msu_str_eqs(node->token->content, "-")) {
            msu_str_builder_printf(out, "SPUSHI 0\n");
            sea_compile_impl(operand, ctx, out, errout);
            if (*errout) return;
            msu_str_builder_printf(out, "SSUB\n");
        } else if (msu_str_eqs(node->token->content, "!")) {
            msu_str_builder_printf(out, "SNOT");
        } else {
            *errout = sea_error_new(node, msu_str_printf("invalid operand '%s'", msu_str_data(node->token->content)));
            return;
        }
    } else if (node->kind == SEA_CALL) {
        const parsenode_t *functor = list_of_parsenodes_get(node->children, 0);
        const parsenode_t *args = list_of_parsenodes_get(node->children, 1);

        if (functor->kind != SEA_IDENT) {
            *errout = sea_error_new(functor, msu_str_new("function name must be an identifier"));
            return;
        }

        for (int i = 0; i < args->children->len; ++i) {
            const parsenode_t *arg = list_of_parsenodes_get(args->children, i);
            sea_compile_impl(arg, ctx, out, errout);
            if (*errout) return;
        }

        if (msu_str_eqs(functor->token->content, "putn")) {
            if (args->children->len != 1) {
                *errout = sea_error_new(args, msu_str_new("invalid arguments for 'void putn(int)'"));
                return;
            }

            msu_str_builder_printf(out, "SPOP\nOUT\n");
            ctx->imm_offset -= 1;
            return;
        }

        if (msu_str_eqs(functor->token->content, "getn")) {
            if (args->children->len != 0) {
                *errout = sea_error_new(args, msu_str_new("invalid arguments for 'int getn(void)'"));
                return;
            }

            msu_str_builder_printf(out, "INP\nSPUSH\n");
            ctx->imm_offset += 1;
            return;
        }

        const parsenode_t *func = sea_compile_ctx_find_function(ctx, functor->token->content);
        if (!func) {
            *errout = sea_error_new(functor, msu_str_printf("undefined reference to function '%s'",
                     msu_str_data(functor->token->content)));
            return;
        }
        const parsenode_t *func_params = list_of_parsenodes_get(func->children, 1);
        const parsenode_t *func_output = list_of_parsenodes_get(func->children, 0);

        ctx->imm_offset -= func_params->children->len; // subtract arguments
        msu_str_builder_printf(out, "CALL %s\n", msu_str_data(functor->token->content));

        if (func_output) {
            ctx->imm_offset += 1;
        }
    } else if (node->kind == SEA_INT) {
        msu_str_builder_printf(out, "SPUSHI %s\n", msu_str_data(node->token->content));
        ctx->imm_offset += 1;
    } else if (node->kind == SEA_IDENT) {
        var_t *var = scope_find_var(ctx->scope, node->token->content);
        size_t offset = ctx->imm_offset + var->frame_offset;
        msu_str_builder_printf(out, "SLDA %zu\n", offset);
        ctx->imm_offset += 1;
    } else if (node->kind == SEA_GROUP) {
        const parsenode_t *child = list_of_parsenodes_get(node->children, 0);
        sea_compile_impl(child, ctx, out, errout);
        if (*errout) return;
    } else {
        fprintf(stderr, "current instruction kind is %d\n", node->kind);
        *errout = sea_error_new(node, msu_str_printf("unimplemented: node->kind = %d", node->kind));
        return;
    }
}

const msu_str_t *sea_compile(const parsenode_t *node, sea_error_t **errout) {
    msu_str_builder_t out = msu_str_builder_new();
    sea_compile_ctx ctx = sea_compile_ctx_new();

    if (node->kind == SEA_PROGRAM) {
        for (int i = 0; i < node->children->len; ++i) {
            parsenode_t *def = list_of_parsenodes_get(node->children, i);
            if (def->kind == SEA_FUNCDEF) {
                if (!sea_compile_ctx_add_function(&ctx, def)) {
                    msu_str_builder_free(out);
                    return NULL;
                }
            } else if (def->kind == SEA_VAR) {
                if (!sea_compile_ctx_add_global(&ctx, def)) {
                    msu_str_builder_free(out);
                    return NULL;
                }
            } else {
                *errout = sea_error_new(def, msu_str_printf("unimplemented: def->kind = %d\n", def->kind));
                return NULL;
            }
        }
    }

    sea_compile_impl(node, &ctx, out, errout);
    if (*errout) return NULL;

    constval_t *constant = ctx.constants;
    while (constant) {
        msu_str_builder_printf(out, "%s DAT %d\n", msu_str_data(constant->label), constant->value);
        constant = constant->next;
    }

    // TODO: free ctx
    return msu_str_builder_into_string_and_free(out);
}

void sea_error_free(sea_error_t *error) {
    if (error) {
        msu_str_free(error->message);
        free(error);
    }
}
