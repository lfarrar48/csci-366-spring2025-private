#include <gtest/gtest.h>

extern "C" {
#include "msulib/parser.h"
}

void test_token(const char *input, tokenkind expected) {
   const msu_str_t *s = msu_str_new(input);
    token *tk = tokenize(s, NULL);
    ASSERT_NE(nullptr, tk) << "tokenize() failed";
    ASSERT_EQ(tk->next, nullptr) << "this should only parse 1 token";
    ASSERT_EQ(tk->kind, expected) << "invalid token kind for " << input;

    msu_str_free(s);
    tokens_free(tk);
}

TEST(msu_parser_tests, test_tokens) {
    test_token("1", TT_INT);
    test_token("123", TT_INT);
    test_token("hi", TT_IDENT);
    test_token("hi_there", TT_IDENT);
    test_token("(", TT_LPAREN);
    test_token(")", TT_RPAREN);
    test_token("!", TT_BANG);
    test_token("!=", TT_BANG_EQ);
    test_token("=", TT_EQ);
    test_token("==", TT_EQ2);
    test_token("+", TT_PLUS);
    test_token("-", TT_DASH);
    test_token("*", TT_STAR);
    test_token("/", TT_SLASH);
    test_token("<", TT_LARROW);
    test_token(">", TT_RARROW);
    test_token("<=", TT_LARROW_EQ);
    test_token(">=", TT_RARROW_EQ);
}

TEST(msu_parser_tests, test_comments) {
   const msu_str_t *s = msu_str_new("Hello # i am a comment");
   const msu_str_t *comment_pat = msu_str_new("#");
    token *tk = tokenize(s, comment_pat);
    ASSERT_NE(nullptr, tk) << "tokenize() failed";
    ASSERT_EQ(tk->next, nullptr) << "this should only parse 1 token";
    ASSERT_EQ(tk->kind, TT_IDENT) << "expected identifier";

    msu_str_free(s);
    msu_str_free(comment_pat);
    tokens_free(tk);
}