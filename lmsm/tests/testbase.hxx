
#include <gtest/gtest.h>

extern "C" {
#include "msulib/str.h"
#include "msulib/parser.h"
}

testing::AssertionResult msu_str_eq_cmp_helper(
    const char *s1_expr, const char *s2_expr,
    const msu_str_t *s1, const char *s2
);

#define ASSERT_MSU_STREQ(s1, s2) \
    EXPECT_PRED_FORMAT2(msu_str_eq_cmp_helper, s1, s2)

void report_errors(const msu_str_t *src, const parsenode_t *node);
std::string msu_str_to_cpp(const msu_str_t *me);
