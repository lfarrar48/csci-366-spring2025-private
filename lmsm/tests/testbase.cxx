#include "testbase.hxx"

testing::AssertionResult msu_str_eq_cmp_helper(
    const char *s1_expr, const char *s2_expr,
    const msu_str_t *s1, const char *s2
) {
    if (msu_str_eqs(s1, s2)) {
        return testing::AssertionSuccess();
    }

    return testing::internal::EqFailure(s1_expr, s2_expr,
        testing::PrintToString(msu_str_data(s1)), testing::PrintToString(s2),
        false);
}

bool report_errors_inner(const msu_str_t *src, const parsenode_t *node) {
    if (!node) return true;

    bool ok = true;
    if (!msu_str_is_empty(node->error)) {
        ok = false;
        fflush(stdout);
        fflush(stderr);
        fprintf(stderr, "error: %s\n", msu_str_data(node->error));
        const msu_str_t *line;
        size_t lineno, lineoff;
        if (node->token) {
            line = msu_str_get_line_for_index(src, node->token->index);
            lineno = msu_str_get_lineno_for_index(src, node->token->index);
            lineoff = msu_str_get_lineoff_for_index(src, node->token->index);
        } else {
            line = EMPTY_STRING;
            lineno = msu_str_get_lineno_for_index(src, msu_str_len(src));
            lineoff = msu_str_get_lineoff_for_index(src, msu_str_len(src));
        }
        fprintf(stderr, "%3zu | %s\n", lineno, node->token ? msu_str_data(line) : "EOF");
        fprintf(stderr, "    | ");
        for (int i = 0; i < lineoff; ++i) {
            fputc(' ', stderr);
        }
        fputs("^^^", stderr);
        fputc('\n', stderr);
    }

    for (int i = 0; i < node->children->len; ++i) {
        ok &= report_errors_inner(src, list_of_parsenodes_get_const(node->children, i));
    }
    return ok;
}

void report_errors(const msu_str_t *src, const parsenode_t *node) {
    bool okay = report_errors_inner(src, node);
    ASSERT_TRUE(okay) << "parse errors found";
}

std::string msu_str_to_cpp(const msu_str_t *me) {
    return std::move(std::string{msu_str_data(me), msu_str_len(me)});
}
