#include "gtest/gtest.h"

extern "C" {
#include "msulib/str.h"
}

TEST(test_msu_string, test_simple) {
   const msu_str_t *s = msu_str_new("Hello Friend");
    ASSERT_EQ(msu_str_len(s), 12);
    ASSERT_STREQ(msu_str_data(s), "Hello Friend");

   const msu_str_t *s2 = msu_str_new("Sniff Sniff");
    ASSERT_EQ(msu_str_len(s2), 11);
    ASSERT_STREQ(msu_str_data(s2), "Sniff Sniff");

   const msu_str_t *s3 = msu_str_new("my name is pathfinder");
    ASSERT_EQ(msu_str_len(s3), 21);
    ASSERT_STREQ(msu_str_data(s3), "my name is pathfinder");

    msu_str_free(s);
    msu_str_free(s2);
}

TEST(test_msu_string, test_list_of_msu_strs) {
   const msu_str_t *s1 = msu_str_new("hello");
   const msu_str_t *s2 = msu_str_new("world");
   const msu_str_t *s3 = msu_str_new("my name is pathfinder");

    list_of_msu_strs_t *ls = list_of_msu_strs_new();
    list_of_msu_strs_append(ls, s1);
    list_of_msu_strs_append(ls, s2);
    list_of_msu_strs_append(ls, s3);

    ASSERT_EQ(ls->len, 3);
    ASSERT_TRUE(msu_str_eq(list_of_msu_strs_get(ls, 0), s1));
    ASSERT_TRUE(msu_str_eq(list_of_msu_strs_get(ls, 1), s2));
    ASSERT_TRUE(msu_str_eq(list_of_msu_strs_get(ls, 2), s3));
}

TEST(test_msu_string, test_splitlines) {
   const msu_str_t *s = msu_str_new(R"(
HELLO THERE,
HI Friend,
My Name is Pathfinder!
Bye!
)");

    list_of_msu_strs_t *lines = msu_str_splitlines(s);
    for (int i = 0; i < lines->len; ++i) {
       const msu_str_t *s = list_of_msu_strs_get_const(lines, i);
        std::string s2{msu_str_data(s), msu_str_len(s)};
        std::cout << "'" << s2 << "'" << std::endl;
    }
    ASSERT_EQ(lines->len, 6);
}

TEST(test_msu_string, test_splitlines_empty) {
   const msu_str_t *s = msu_str_new("");
    list_of_msu_strs_t *lines = msu_str_splitlines(s);
    ASSERT_EQ(lines->len, 1);

   const msu_str_t *line1 = list_of_msu_strs_get_const(lines, 0);
    ASSERT_STREQ(msu_str_data(line1), "");
}

TEST(test_msu_string, test_replacement) {
    const msu_str_t *src = msu_str_new("hi {{name}}{{name}}, my name is {{name}}!");
    const msu_str_t *pat = msu_str_new("{{name}}");
    const msu_str_t *pathy = msu_str_new("pathfinder");

    const msu_str_t *replaced = msu_str_replace_all(src, pat, pathy);
    ASSERT_STREQ(msu_str_data(replaced), "hi pathfinderpathfinder, my name is pathfinder!");
    msu_str_free(src);
    msu_str_free(pat);
    msu_str_free(pathy);
}

TEST(test_msu_string, test_split) {
    const msu_str_t *src = msu_str_new("a::b:");
    const msu_str_t *delim = msu_str_new(":");
    const list_of_msu_strs_t *parts = msu_str_split(src, delim);
    ASSERT_EQ(parts->len, 4);
    for (int i = 0; i < parts->len; ++i) {
        std::printf("'%s'\n", msu_str_data(list_of_msu_strs_get_const(parts, i)));
    }
}