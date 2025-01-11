#ifndef NEO366_PARSER_H
#define NEO366_PARSER_H

#include <stdlib.h>
#include "msulib/parsenodelist.h"
#include "msulib/str.h"

typedef enum tokenkind {
    TT_INT,
    TT_IDENT,
    TT_ERROR,

    TT_LPAREN,
    TT_RPAREN,
    TT_LBRACE,
    TT_RBRACE,

    TT_COLON,
    TT_SEMICOLON,
    TT_COMMA,

    TT_BANG,
    TT_BANG_EQ,
    TT_EQ,
    TT_EQ2,
    TT_PLUS,
    TT_DASH,
    TT_STAR,
    TT_SLASH,
    TT_DOT,
    TT_LARROW,
    TT_RARROW,
    TT_LARROW_EQ,
    TT_RARROW_EQ,
} tokenkind;

typedef struct token {
    const msu_str_t *content;
    tokenkind kind;
    size_t index;
    struct token *next;
} token;

token *tokenize(const msu_str_t *src, const msu_str_t *commentPat);
token *tokenize_ext(const msu_str_t *src, const msu_str_t *commentPat);

void tokens_free(token *tok);
void token_free(token *tok);
token *token_clone(token *tok);

typedef struct parser {
    token *root;
    token *current;
    list_of_msu_strs_t *keywords;
} parser;

typedef struct parsenode {
    int kind;
    const msu_str_t *error;
    token *token;
    list_of_parsenodes_t *children;
} parsenode_t;

void parsenode_add_child(parsenode_t *node, parsenode_t *child);
void parsenode_set_error(parsenode_t *node, const msu_str_t *child);

bool parser_has_next(parser *parser); // if there are more tokens in the source

token *parser_peek_kw(parser *pw, const char *lex); // if a keyword with `lex` can be matched
token *parser_take_kw(parser *pw, const char *lex); // if a keyword with 'lex' can be consumed
token *parser_peek_word(parser *pw); // if a keyword with `lex` can be matched
token *parser_take_word(parser *pw); // if a keyword with 'lex' can be consumed

token *parser_peek_punct(parser *pw, const char *lex); // if a keyword with `lex` can be matched
token *parser_take_punct(parser *pw, const char *lex); // if a keyword with 'lex' can be consumed

token *parser_peek(parser *pw); // null if at eoi
token *parser_take(parser *pw);  // null if at eoi, returns current token and then advances

token *parser_peek_kind(parser *pw, tokenkind kind); // null if not matched
token *parser_take_kind(parser *pw, tokenkind kind); // null if not matched, returns token then advances

parsenode_t *parsenode_new(int kind, token *token);
void parsenode_free(parsenode_t *node);

void parsenode_print(parsenode_t *node);
bool parsenode_has_errors(parsenode_t *elt);
void parsenode_print_errors(parsenode_t *elt);

void parsenode_collect_errors(const parsenode_t *node, list_of_parsenodes_t *out);

#endif //NEO366_PARSER_H
