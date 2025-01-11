#include "gtest/gtest.h"

#include "testbase.hxx"

extern "C" {
    #include "lmsm/firth.h"
    #include "lmsm/asm.h"
    #include "lmsm/emulator.h"
}

TEST(test_firth, test_number) {
    const msu_str_t *src = msu_str_new("1");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_INT) << "1 should parse as FR_INT";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, test_operators) {
    const char *operators[10] = { "max", "min", "push", "pop", "dup", "swap", "+", "-", "*", "/"};
    for (const char* op : operators) {
        const msu_str_t *src = msu_str_new(op);
        parsenode_t *elt = fr_parse_elt(src);
        ASSERT_NE(elt, nullptr) << "parsed statement was null!";
        ASSERT_EQ(elt->kind, FR_OP) << op << " should parse as FR_OP";
        parsenode_free(elt);
        msu_str_free(src);
    }
}

TEST(test_firth, do_loop) {
    const msu_str_t *src = msu_str_new("do 1 loop");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_DO_LOOP) << "do 1 loop should parse as FR_LOOP";

    ASSERT_NE(list_of_parsenodes_get(elt->children, 0), nullptr) << "first child was null!";
    ASSERT_EQ(list_of_parsenodes_get(elt->children, 0)->kind, FR_INT) << "should be an int";

    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, do_empty_loop) {
    const msu_str_t *src = msu_str_new("do loop");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_DO_LOOP) << "do 1 loop should parse as FR_LOOP";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, do_unterminated_loop) {
    const msu_str_t *src = msu_str_new("do 1");
    parsenode_t *elt = fr_parse_elt(src);

    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_DO_LOOP) << "do should parse as FR_LOOP";

    ASSERT_NE(list_of_parsenodes_get(elt->children, 1), nullptr) << "first child was null!";
    ASSERT_EQ(list_of_parsenodes_get(elt->children, 1)->kind, FR_ERROR) << "should be an unterminated loop error";

    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, test_stop) {
    const msu_str_t *src = msu_str_new("stop");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_STOP) << "should parse as FR_STOP";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, zero_test_basic) {
    const msu_str_t *src = msu_str_new("zero? . end");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_ZERO_TEST) << "should parse as FR_ZERO_TEST";

    ASSERT_EQ(elt->children->len, 2) << "Should have two children";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->kind, FR_TRUE_BLOCK) << "First child should be true case";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->children->len, 1) << "Should have one child";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,1)->kind, FR_FALSE_BLOCK) << "Second child should be false case";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,1)->children->len, 0) << "Should have zero children";

    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, zero_test_with_else) {
    const msu_str_t *src = msu_str_new("zero? else . end");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_ZERO_TEST) << "should parse as FR_ZERO_TEST";

    ASSERT_EQ(elt->children->len, 2) << "Should have two children";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->kind, FR_TRUE_BLOCK) << "First child should be true case";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->children->len, 0) << "Should have one child";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,1)->kind, FR_FALSE_BLOCK) << "Second child should be false case";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,1)->children->len, 1) << "Should have zero children";

    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, zero_test_true_and_with_else) {
    const msu_str_t *src = msu_str_new("zero? , else . end");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_ZERO_TEST) << "should parse as FR_ZERO_TEST";

    ASSERT_EQ(elt->children->len, 2) << "Should have two children";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->kind, FR_TRUE_BLOCK) << "First child should be true case";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->children->len, 1) << "Should have one child";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,1)->kind, FR_FALSE_BLOCK) << "Second child should be false case";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,1)->children->len, 1) << "Should have zero children";

    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, zero_test_unterminated) {
    const msu_str_t *src = msu_str_new("zero? .");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_ZERO_TEST) << "should parse as FR_ZERO_TEST";

    ASSERT_EQ(elt->children->len, 3) << "Should have two children";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->kind, FR_TRUE_BLOCK) << "First child should be true case";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->children->len, 1) << "Should have one child";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,1)->kind, FR_FALSE_BLOCK) << "Second child should be false case";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,1)->children->len, 0) << "Should have zero children";

    ASSERT_EQ(list_of_parsenodes_get(elt->children,2)->kind, FR_ERROR) << "Should be unterminated zero error";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, var_basic) {
    const msu_str_t *src = msu_str_new("var x");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_VAR) << "should parse as FR_VAR";
    ASSERT_TRUE(msu_str_eqs(elt->token->content,"x")) << "Var should have name 'x'";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, var_needs_name) {
    const msu_str_t *src = msu_str_new("var 1");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_ERROR) << "should parse as FR_ERROR";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, function_definition) {
    const msu_str_t *src = msu_str_new(": foo + ;");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_FUNCTION_DEF) << "should parse as FR_FUNCTION_DEF";
    ASSERT_TRUE(msu_str_eqs(elt->token->content,"foo")) << "Function should have name 'foo'";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, function_definition_unterminated) {
    const msu_str_t *src = msu_str_new(": foo");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_FUNCTION_DEF) << "should parse as FR_FUNCTION_DEF";
    ASSERT_TRUE(msu_str_eqs(elt->token->content,"foo")) << "Function should have name 'foo'";

    ASSERT_EQ(elt->children->len,1) << "should be one error child";
    ASSERT_EQ(list_of_parsenodes_get(elt->children,0)->kind,FR_ERROR) << "should be one error child";

    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, can_have_forth_style_comments_in_function) {
    const msu_str_t *src = msu_str_new(": foo ( a b -- c ) + ;");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_FUNCTION_DEF) << "should parse as FR_FUNCTION_DEF";
    ASSERT_EQ(elt->children->len, 1) << "should have one child";
    ASSERT_TRUE(msu_str_eqs(elt->token->content,"foo")) << "Function should have name 'foo'";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, asm_example) {
    const msu_str_t *src = msu_str_new("asm lda 22 end");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_ASM) << "should parse as FR_ASM";
    ASSERT_EQ(elt->children->len, 2) << "should have two children";

    ASSERT_NE(list_of_parsenodes_get(elt->children, 0), nullptr) << "first child was null!";
    ASSERT_EQ(list_of_parsenodes_get(elt->children, 0)->kind, FR_ASM_WORD) << "should be an asm word";

    ASSERT_NE(list_of_parsenodes_get(elt->children, 1), nullptr) << "second child was null!";
    ASSERT_EQ(list_of_parsenodes_get(elt->children, 1)->kind, FR_ASM_WORD) << "should be an an asm word";

    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, empty_asm_example) {
    const msu_str_t *src = msu_str_new("asm end");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_ASM) << "should parse as FR_ASM";
    ASSERT_EQ(elt->children->len, 0) << "should have two children";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, unterminated_asm_example) {
    const msu_str_t *src = msu_str_new("asm out");
    parsenode_t *elt = fr_parse_elt(src);

    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_ASM) << "do should parse as FR_ASM";
    ASSERT_EQ(elt->children->len, 2) << "should have 2 children";

    ASSERT_NE(list_of_parsenodes_get(elt->children, 0), nullptr) << "first child was null!";
    ASSERT_EQ(list_of_parsenodes_get(elt->children, 0)->kind, FR_ASM_WORD) << "should be an asm word";

    ASSERT_NE(list_of_parsenodes_get(elt->children, 1), nullptr) << "first child was null!";
    ASSERT_EQ(list_of_parsenodes_get(elt->children, 1)->kind, FR_ERROR) << "should be an unterminated asm error";

    parsenode_free(elt);
    msu_str_free(src);
}

TEST(test_firth, basic_word) {
    const msu_str_t *src = msu_str_new("foo");
    parsenode_t *elt = fr_parse_elt(src);
    ASSERT_NE(elt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(elt->kind, FR_WORD) << "should parse as FW_WORD";
    ASSERT_TRUE(msu_str_eqs(elt->token->content,"foo")) << "Word should have value 'foo";
    parsenode_free(elt);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_program) {
    const msu_str_t *src = msu_str_new("1 1 + .");
    const msu_str_t *asm_src = fr_compile_debug(src);

    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << asm_err->message;

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 2);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_multiplication) {
    const msu_str_t *src = msu_str_new("2 3 * .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 6);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_subtraction) {
    const msu_str_t *src = msu_str_new("3 1 - .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 2);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_division) {
    const msu_str_t *src = msu_str_new("6 2 / .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 3);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_max) {
    const msu_str_t *src = msu_str_new("6 2 max .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 6);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_min) {
    const msu_str_t *src = msu_str_new("6 2 min .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 2);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_pop) {
    const msu_str_t *src = msu_str_new("1 2 3 pop + .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 3);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_dup) {
    const msu_str_t *src = msu_str_new("1 2 3 dup + .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 6);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_swap) {
    const msu_str_t *src = msu_str_new("2 3 swap - .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 1);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_drop) {
    const msu_str_t *src = msu_str_new("2 3 drop");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 3);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_print) {
    const msu_str_t *src = msu_str_new("1 .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_STREQ(emulator->output_buffer, "1 ");

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_variable) {
    const msu_str_t *src = msu_str_new("var x 2 x! x .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_STREQ(emulator->output_buffer, "2 ");

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_zero_test_positive) {
    const msu_str_t *src = msu_str_new("0 zero? 1 . end");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_STREQ(emulator->output_buffer, "1 ");

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_zero_test_negative) {
    const msu_str_t *src = msu_str_new("1 zero? 1 else 2 end .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_STREQ(emulator->output_buffer, "2 ");

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_positive_test_positive) {
    const msu_str_t *src = msu_str_new("0 positive? 1 . end");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_STREQ(emulator->output_buffer, "1 ");

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_positive_test_negative) {
    const msu_str_t *src = msu_str_new("0 1 - positive? 1 else 2 end .");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_STREQ(emulator->output_buffer, "2 ");

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_loop) {
    const msu_str_t *src = msu_str_new(" var x "
                              " 3 x! "
                              " do "
                              "  x . "
                              "  x 1 - x! "
                              "  x zero?"
                              "    stop "
                              "  end "
                              " loop ");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_STREQ(emulator->output_buffer, "3 2 1 ");

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, basic_function_works) {
    const msu_str_t *src = msu_str_new(": foo 2 . ; foo");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 2);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, unterminated_function) {
    const msu_str_t *src = msu_str_new(": foo 2 . foo");
    const msu_str_t *asm_src = fr_compile_debug(src);
    ASSERT_EQ(0, msu_str_len(asm_src)); // Error, should produce empty compilation
    msu_str_free(asm_src);
    msu_str_free(src);
}

TEST(end_to_end_firth, recursive_fib) {
    const msu_str_t *src = msu_str_new(
            "12 fib . \n"
            ": fib dup zero? exit end dup 1 - zero? exit end dup 2 - fib swap 1 - fib + ;");
    const msu_str_t *asm_src = fr_compile_debug(src);
    asm_error_t *asm_err = nullptr;
    int *machine_code = asm_assemble(asm_src, &asm_err);
    ASSERT_EQ(asm_err, nullptr) << msu_str_to_cpp(asm_err->message);

    emulator_t *emulator = emulator_exec(machine_code);
    ASSERT_EQ(emulator->accumulator, 144);

    emulator_free(emulator);
    free(machine_code);
    msu_str_free(asm_src);
    msu_str_free(src);
}


