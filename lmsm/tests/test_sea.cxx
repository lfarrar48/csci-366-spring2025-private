#include <gtest/gtest.h>
#include <string>
#include <memory>
#include "testbase.hxx"

extern "C" {
#include "lmsm/sea.h"
#include "lmsm/emulator.h"
#include "lmsm/asm.h"
}

TEST(sea_tests, test_simple) {
    const msu_str_t *src = msu_str_new("1");
    parsenode_t *expr = sea_parse_expr(src);

    ASSERT_EQ(expr->kind, SEA_INT);
    ASSERT_NE(expr->token, nullptr);
    ASSERT_MSU_STREQ(expr->token->content, "1");
    ASSERT_EQ(expr->children->len, 0);

    msu_str_free(src);
    parsenode_free(expr);
}

class check_node {
private:
    parsenode_t *node;

    std::unique_ptr<int> kind;
    std::unique_ptr<size_t> n_childern;
    std::unique_ptr<std::string> token_text;

public:
    check_node(parsenode_t *node) : node(node) {}

    check_node &is(int kind) {
        this->kind = std::move(std::make_unique<int>(kind));
        return *this;
    }

    check_node &has_n_children(size_t n) {
        this->n_childern = std::move(std::make_unique<size_t>(n));
        return *this;
    }

    check_node &has_text(std::string s) {
        this->token_text = std::move(std::make_unique<std::string>(std::move(s)));
        return *this;
    }

    void drop() const {
        if (kind) {
            ASSERT_EQ(node->kind, *kind);
        }
        if (token_text) {
            ASSERT_MSU_STREQ(node->token->content, token_text->c_str());
        }
        if (n_childern) {
            ASSERT_EQ(node->children->len, *n_childern);
        }
    }

    ~check_node() {
        drop();
    }
};

TEST(sea_tests_e2e, simple) {
    const msu_str_t *src = msu_str_new(R"(
int main() {
    return 0;
}
)");

    parsenode_t *program = sea_parse(src);
    report_errors(src, program);
    msu_str_free(src);

    sea_error_t *sea_err = nullptr;
    const msu_str_t *bytecode = sea_compile(program, &sea_err);
    ASSERT_EQ(sea_err, nullptr) << msu_str_data(sea_err->message);
    std::cout << msu_str_to_cpp(bytecode) << std::endl;

    asm_error_t *asm_err = nullptr;
    int *code = asm_assemble(bytecode, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);
    msu_str_free(bytecode);

    emulator_t *em = emulator_exec(code);
    emulator_free(em);
    free(code);

    parsenode_free(program);
}

TEST(sea_tests_e2e, prints) {
    const msu_str_t *src = msu_str_new(R"(
int main() {
    putn(13);
    return 0;
}
)");

    parsenode_t *program = sea_parse(src);
    report_errors(src, program);
    msu_str_free(src);

    sea_error_t *sea_error = nullptr;
    const msu_str_t *bytecode = sea_compile(program, &sea_error);
    ASSERT_EQ(sea_error, nullptr);
    std::cout << msu_str_to_cpp(bytecode) << std::endl;

    asm_error_t *asm_err = nullptr;
    int *code = asm_assemble(bytecode, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);
    msu_str_free(bytecode);

    emulator_t *em = emulator_exec(code);
    ASSERT_STREQ(em->output_buffer, "13 ");

    emulator_free(em);
    free(code);

    parsenode_free(program);
}

TEST(sea_tests_e2e, conditionals) {
    const msu_str_t *src = msu_str_new(R"(
int main() {
    if (4 < 5) {
        putn(9);
    }
    return 0;
}
)");

    parsenode_t *program = sea_parse(src);
    report_errors(src, program);
    msu_str_free(src);

    sea_error_t *sea_error = NULL;
    const msu_str_t *bytecode = sea_compile(program, &sea_error);
    ASSERT_EQ(sea_error, nullptr) << msu_str_data(sea_error->message);
    std::cout << msu_str_to_cpp(bytecode) << std::endl;

    asm_error_t *asm_err = nullptr;
    int *code = asm_assemble(bytecode, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);
    msu_str_free(bytecode);

    emulator_t *em = emulator_exec(code);
    ASSERT_STREQ(em->output_buffer, "9 ");

    emulator_free(em);
    free(code);

    parsenode_free(program);
}

TEST(sea_tests_e2e, vars) {
    const msu_str_t *src = msu_str_new(R"(
int main() {
    int x = 3;
    putn(x);
    return 0;
}
)");

    parsenode_t *program = sea_parse(src);
    report_errors(src, program);
    msu_str_free(src);

    sea_error_t *sea_error = NULL;
    const msu_str_t *bytecode = sea_compile(program, &sea_error);
    ASSERT_EQ(sea_error, nullptr) << msu_str_data(sea_error->message);

    asm_error_t *asm_err = nullptr;
    int *code = asm_assemble(bytecode, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);
    msu_str_free(bytecode);

    emulator_t *em = emulator_exec(code);
    ASSERT_STREQ(em->output_buffer, "3 ");
    emulator_free(em);
    free(code);

    parsenode_free(program);
}

TEST(sea_tests_e2e, for_loop) {
    const msu_str_t *src = msu_str_new(R"(
int main() {
    for (int x = 0; x < 3; x = x + 1) {
        putn(x);
    }
    return 0;
}
)");

    parsenode_t *program = sea_parse(src);
    report_errors(src, program);
    msu_str_free(src);

    sea_error_t *sea_error = NULL;
    const msu_str_t *bytecode = sea_compile(program, &sea_error);
    ASSERT_EQ(sea_error, nullptr) << msu_str_data(sea_error->message);
    std::cout << msu_str_to_cpp(bytecode) << std::endl;

    asm_error_t *asm_err = nullptr;
    int *code = asm_assemble(bytecode, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);
    msu_str_free(bytecode);

    emulator_t *em = emulator_exec(code);
    ASSERT_STREQ(em->output_buffer, "0 1 2 ");

    emulator_free(em);
    free(code);

    parsenode_free(program);
}

