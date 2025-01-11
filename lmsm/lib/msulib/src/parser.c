#include "msulib/parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

token *tokenize_impl(const msu_str_t *src, const msu_str_t *commentPat, bool extended_identifiers);

token *tokenize(const msu_str_t *src, const msu_str_t *commentPat) {
    return tokenize_impl(src, commentPat, false);
}

token *tokenize_ext(const msu_str_t *src, const msu_str_t *commentPat) {
    return tokenize_impl(src, commentPat, true);
}

token *tokenize_impl(const msu_str_t *src, const msu_str_t *commentPat, bool extended_identifiers) {
    size_t index = 0;

    token *out = NULL;
    token *cur = NULL;

    while (1) {
        while (index < msu_str_len(src) && isspace(msu_str_at(src, index))) {
            index += 1;
        }

        if (index >= msu_str_len(src)) {
            break;
        }

        if (msu_str_len(commentPat)
            && index + msu_str_len(commentPat) < msu_str_len(src)
            && msu_str_eqn(commentPat, msu_str_data(src) + index, msu_str_len(commentPat)))
        {
            index += msu_str_len(commentPat);
            while (index < msu_str_len(src) && msu_str_at(src, index) != '\n') {
                index += 1;
            }
            continue; // redo whitespace
        }

        const msu_str_t *content = NULL;
        tokenkind kind = TT_ERROR;
        size_t start = index;

        char c = msu_str_at(src, index);
        index += 1;
        if (isalpha(c)) {
            kind = TT_IDENT;
            if (extended_identifiers) {
                while (index < msu_str_len(src) && !isspace(msu_str_at(src, index))) {
                    index += 1;
                }
            } else {
                while (index < msu_str_len(src) && (isalnum(msu_str_at(src, index)) || msu_str_at(src, index) == '_')) {
                    index += 1;
                }
            }
        } else if (isdigit(c)) {
            kind = TT_INT;
            while (index < msu_str_len(src) && isdigit(msu_str_at(src, index))) {
                index += 1;
            }
        } else if (c == '(') {
            kind = TT_LPAREN;
        } else if (c == ')') {
            kind = TT_RPAREN;
        } else if (c == '{') {
            kind = TT_LBRACE;
        } else if (c == '}') {
            kind = TT_RBRACE;
        } else if (c == ':') {
            kind = TT_COLON;
        } else if (c == ',') {
            kind = TT_COMMA;
        } else if (c == ';') {
            kind = TT_SEMICOLON;
        } else if (c == '!') {
            if (index < msu_str_len(src) && msu_str_at(src, index) == '=') {
                index += 1;
                kind = TT_BANG_EQ;
            } else {
                kind = TT_BANG;
            }
        } else if (c == '=') {
            if (index < msu_str_len(src) && msu_str_at(src, index) == '=') {
                index += 1;
                kind = TT_EQ2;
            } else {
                kind = TT_EQ;
            }
        } else if (c == '+') {
            kind = TT_PLUS;
        } else if (c == '-') {
            kind = TT_DASH;
        } else if (c == '*') {
            kind = TT_STAR;
        } else if (c == '/') {
            kind = TT_SLASH;
        } else if (c == '.') {
            kind = TT_DOT;
        } else if (c == '<') {
            if (index < msu_str_len(src) && msu_str_at(src, index) == '=') {
                index += 1;
                kind = TT_LARROW_EQ;
            } else {
                kind = TT_LARROW;
            }
        } else if (c == '>') {
            if (index < msu_str_len(src) && msu_str_at(src, index) == '=') {
                index += 1;
                kind = TT_RARROW_EQ;
            } else {
                kind = TT_RARROW;
            }
        } else {
            kind = TT_ERROR;
            content = msu_str_printf("unexpected character c=%d c=%c", (int) c,
                                    (c >= ' ' && c <= 127) ? c : ' ');
        }

        if (kind != TT_ERROR) {
            content = msu_str_substring(src, start, index);
        }

        token *next = malloc(sizeof(token));
        next->content = content;
        next->kind = kind;
        next->index = start;
        next->next = NULL;

        if (cur) {
            cur->next = next;
        } else {
            out = next;
        }
        cur = next;
    }

    return out;
}

void tokens_free(token *tok) {
    while (tok) {
        token *next = tok->next;
        token_free(tok);
        tok = next;
    }
}

void token_free(token *tok) {
    if (tok) {
        msu_str_free(tok->content);
        free(tok);
    }
}

token *token_clone(token *tok) {
    token *out = malloc(sizeof(token));
    out->kind = tok->kind;
    out->index = tok->index;
    out->content = msu_str_clone(tok->content);
    out->next = NULL;
    return out;
}

void parsenode_add_child(parsenode_t *node, parsenode_t *child) {
    list_of_parsenodes_append(node->children, child);
}

void parsenode_set_error(parsenode_t *node, const msu_str_t *error) {
    node->error = error;
}

bool parser_has_next(parser *pw) {
    return NULL != pw->current;
}

token *parser_peek_kw(parser *pw, const char *lex) {
    if (!pw->current) return NULL;
    if (pw->current->kind != TT_IDENT) return NULL;
    if (!msu_str_eqs(pw->current->content, lex)) return NULL;
    return pw->current;
}

token *parser_take_kw(parser *pw, const char *lex) {
    token *tok = parser_peek_kw(pw, lex);
    if (!tok) return NULL;
    pw->current = pw->current->next;
    return tok;
}

token *parser_peek_word(parser *pw) {
    if (!pw->current) return NULL;
    if (pw->current->kind != TT_IDENT) return NULL;
    for (size_t i = 0; i < pw->keywords->len; i++) {
        const msu_str_t *kw = list_of_msu_strs_get(pw->keywords, i);
        if (msu_str_eq(pw->current->content, kw)) {
            return NULL;
        }
    }
    return pw->current;
}

token *parser_take_word(parser *pw) {
    token *tok = parser_peek_word(pw);
    if (!tok) return NULL;
    pw->current = pw->current->next;
    return tok;
}

token *parser_peek_punct(parser *pw, const char *lex) {
    if (!pw->current) return NULL;
    if (pw->current->kind == TT_ERROR ||
        pw->current->kind == TT_IDENT ||
        pw->current->kind == TT_INT) {
        return NULL;
    }
    if (!msu_str_eqs(pw->current->content, lex)) return NULL;
    return pw->current;
}

token *parser_take_punct(parser *pw, const char *lex) {
    token *tok = parser_peek_punct(pw, lex);
    if (!tok) return NULL;
    pw->current = pw->current->next;
    return tok;
}

token *parser_peek(parser *pw) {
    return pw->current;
}

token *parser_take(parser *pw) {
    token *tok = parser_peek(pw);
    if (!tok) return NULL;
    pw->current = pw->current->next;
    return tok;
}

token *parser_peek_kind(parser *pw, tokenkind kind) {
    if (!pw->current) return NULL;
    if (pw->current->kind != kind) return NULL;
    return pw->current;
}

token *parser_take_kind(parser *pw, tokenkind kind) {
    token *tok = parser_peek_kind(pw, kind);
    if (!tok) return NULL;
    pw->current = pw->current->next;
    return tok;
}

parsenode_t *parsenode_new(int kind, token *token) {
    parsenode_t *out = malloc(sizeof(parsenode_t));
    out->kind = kind;
    out->error = EMPTY_STRING;
    out->token = token ? token_clone(token) : NULL;
    out->children = list_of_parsenodes_new();
    return out;
}

void parsenode_free(parsenode_t *node) {
    if (node) {
        list_of_parsenodes_free(node->children, true);
        free(node);
    }
}

#define panic(msg, ...) do { \
    fprintf(stderr, msg, __VA_ARGS__); \
    exit(1); \
} while(0)


void parsenode_print_impl(parsenode_t *node, int ident) {
    int tmp = ident;
    while (tmp-- > 0) {
        printf("  ");
    }
    const char *content = "<unknown>";
    token *tok = node->token;
    if (tok) {
        const msu_str_t *str = tok->content;
        if (str) {
            content = msu_str_data(str);
        }
    }
    printf("Node - (Token: %s, Kind: %i)\n", content, node->kind);
    for (size_t i = 0; i < node->children->len; i++) {
        parsenode_t *child = list_of_parsenodes_get(node->children, i);
        parsenode_print_impl(child, ident + 1);
    }
}
void parsenode_print(parsenode_t *node) {
    parsenode_print_impl(node, 0);
}

bool parsenode_has_errors(parsenode_t *elt) {
    if (elt->error) {
        return true;
    }
    for (int i = 0; i < elt->children->len; ++i) {
        parsenode_t *child = list_of_parsenodes_get(elt->children, i);
        if (parsenode_has_errors(child) == true) {
            return true;
        }
    }
    return false;
}

void parsenode_print_errors_in_tree(parsenode_t *elt) {
    if (elt->error) {
        printf("  - Error %s\n", msu_str_data(elt->error));
    }
    for (int i = 0; i < elt->children->len; ++i) {
        parsenode_t *child = list_of_parsenodes_get(elt->children, i);
        parsenode_print_errors_in_tree(child);
    }
}

void parsenode_print_errors(parsenode_t *elt) {
    printf("\n\n"
           "######################################\n"
           "# Compilation Errors:\n"
           "######################################\n\n");
    parsenode_print_errors_in_tree(elt);
    printf("\n\n");
}

void parsenode_collect_errors(const parsenode_t *node, list_of_parsenodes_t *out) {
    if (!node) return;

    if (node->error) {
        list_of_parsenodes_append(out, (void *) node);
    }

    for (int i = 0; i < node->children->len; ++i) {
        parsenode_t *child = list_of_parsenodes_get(node->children, i);
        parsenode_collect_errors(child, out);
    }
}
