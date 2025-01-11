#include <gtest/gtest.h>
#include "testbase.hxx"

extern "C" {
#include "lmsm/asm.h"
}

list_of_asm_insrs_t *AsmParse(std::string s) {
    const msu_str_t *s2 = msu_str_new_substring(s.data(), s.length());
    printf("string is %s\n", msu_str_data(s2));
    list_of_asm_insrs_t *insrs = asm_parse(s2);
    msu_str_free(s2);
    return insrs;
}

#define ASSERT_NULL(v) ASSERT_EQ(v, nullptr)
#define ASSERT_NOTNULL(v) ASSERT_NE(v, nullptr)

TEST(asm_tests, test_hlt) {
    auto insrs = AsmParse("HLT");
    ASSERT_NOTNULL(insrs);
    ASSERT_EQ(insrs->len, 1) << "expected only 1 instruction";

    auto insr = list_of_asm_insrs_get(insrs, 0);
    ASSERT_EQ(insr->error, nullptr) << "expected no error";
    ASSERT_MSU_STREQ(insr->label, "") << "'HLT' doesn't set a label";
    ASSERT_MSU_STREQ(insr->label_reference, "") << "HLT doesn't reference labels";
    ASSERT_MSU_STREQ(insr->instruction, "HLT");
    ASSERT_EQ(insr->value, 0) << "HLT doesn't have a value";

    list_of_asm_insrs_free(insrs, true);
}

TEST(asm_tests, test_label) {
    auto insrs = AsmParse("FOO INP");
    ASSERT_NOTNULL(insrs);
    ASSERT_EQ(insrs->len, 1);

    auto insr = list_of_asm_insrs_get(insrs, 0);
    ASSERT_EQ(insr->error, nullptr) << "expected no error";
    ASSERT_MSU_STREQ(insr->label, "FOO") << "expected 'FOO' label";
    ASSERT_MSU_STREQ(insr->label_reference, "") << "'FOO COB' shouldn't reference any label";
    ASSERT_MSU_STREQ(insr->instruction, "INP") << "expected 'INP' instruction";
    ASSERT_EQ(insr->value, 0) << "shouldn't have a value";
}

TEST(asm_tests, test_value) {
    auto insrs = AsmParse("ADD 42");
    ASSERT_NOTNULL(insrs);
    ASSERT_EQ(insrs->len, 1);

    auto insr = list_of_asm_insrs_get(insrs, 0);
    ASSERT_EQ(insr->error, nullptr) << "expected no error";
    ASSERT_MSU_STREQ(insr->label, "") << "'ADD 42' doesn't have a label";
    ASSERT_MSU_STREQ(insr->label_reference, "") << "'ADD 42' doesn't reference any label";
    ASSERT_MSU_STREQ(insr->instruction, "ADD") << "expected 'ADD' instruction";
    ASSERT_EQ(insr->value, 42) << "expected value 42";
}

TEST(asm_tests, test_label_ref) {
    auto insrs = AsmParse("SUB $42");
    ASSERT_NOTNULL(insrs);
    ASSERT_EQ(insrs->len, 1);

    auto insr = list_of_asm_insrs_get(insrs, 0);
    ASSERT_EQ(insr->error, nullptr) << "expected no error";
    ASSERT_MSU_STREQ(insr->label, "") << "'SUB $42' doesn't have a label";
    ASSERT_MSU_STREQ(insr->label_reference, "$42") << "'SUB $42' references the label '$42'";
    ASSERT_MSU_STREQ(insr->instruction, "SUB") << "expected 'SUB' instruction";
    ASSERT_EQ(insr->value, 0) << "shouldn't have a value";
}

TEST(asm_tests, test_label_with_ref) {
    auto insrs = AsmParse("loop_start LDA _$myVar");
    ASSERT_NOTNULL(insrs);
    ASSERT_EQ(insrs->len, 1);

    auto insr = list_of_asm_insrs_get(insrs, 0);
    ASSERT_EQ(insr->error, nullptr) << "expected no error";
    ASSERT_MSU_STREQ(insr->label, "loop_start") << "'loop_start LDA _$myVar' sets the 'loop_start' label";
    ASSERT_MSU_STREQ(insr->label_reference, "_$myVar") << "'loop_start LDA _$myVar' references the label '_$myVar'";
    ASSERT_MSU_STREQ(insr->instruction, "LDA") << "expected 'LDA' instruction";
    ASSERT_EQ(insr->value, 0) << "shouldn't have a value";
}

TEST(instruction_construction, make_instruction_spushi_takes_two_slots) {
    const msu_str_t *src = msu_str_new("SPUSHI");
    asm_insr_t *instruction = asm_parse_insr(src);
    ASSERT_MSU_STREQ(instruction->instruction, "SPUSHI");
    ASSERT_EQ(asm_insr_size(instruction), 2);

    asm_insr_free(instruction);
    msu_str_free(src);
}

TEST(instruction_construction, make_instruction_call_takes_three_slots) {
    const msu_str_t *src = msu_str_new("CALL");
    asm_insr_t *instruction = asm_parse_insr(src);
    ASSERT_MSU_STREQ(instruction->instruction, "CALL");
    ASSERT_EQ(asm_insr_size(instruction), 2);
    asm_insr_free(instruction);
    msu_str_free(src);
}

//==========================================================================
// Parsing Tests
//==========================================================================

TEST(parsing_tests, simple_instruction_parsing_works) {
    const msu_str_t *src = msu_str_new("OUT");
    list_of_asm_insrs_t *insrs = asm_parse(src);

    ASSERT_EQ(insrs->len, 1);

    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_MSU_STREQ(insr->instruction, "OUT");

    msu_str_free(src);
    list_of_asm_insrs_free(insrs, true);
}

TEST(parsing_tests, two_instruction_parsing_works_w_newline) {
    const msu_str_t *src = msu_str_new("INP\nOUT\n");
    list_of_asm_insrs_t *insrs = asm_parse(src);

    ASSERT_EQ(insrs->len, 2);

    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_MSU_STREQ(insr->instruction, "INP");

    insr = list_of_asm_insrs_get_const(insrs, 1);
    ASSERT_MSU_STREQ(insr->instruction, "OUT");

    msu_str_free(src);
    list_of_asm_insrs_free(insrs, true);
}

TEST(parsing_tests, label_is_parsed_correctly) {
    const msu_str_t *src = msu_str_new("FOO OUT\n");
    list_of_asm_insrs_t *insrs = asm_parse(src);
    
    ASSERT_EQ(insrs->len, 1);
    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_MSU_STREQ(insr->instruction, "OUT");
    ASSERT_MSU_STREQ(insr->label, "FOO");

    list_of_asm_insrs_free(insrs, true);
    msu_str_free(src);
}

TEST(parsing_tests, numeric_reference_works_for_branch) {
    const msu_str_t *src = msu_str_new("BRA 22\n");
    list_of_asm_insrs_t *insrs = asm_parse(src);

    ASSERT_EQ(insrs->len, 1);
    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_MSU_STREQ(insr->instruction, "BRA");
    ASSERT_EQ(insr->value, 22);

    list_of_asm_insrs_free(insrs, true);
    msu_str_free(src);
}

TEST(parsing_tests, label_reference_works_for_branch) {
    const msu_str_t *src = msu_str_new("BRA FOO\n");
    list_of_asm_insrs_t *insrs = asm_parse(src);

    ASSERT_EQ(insrs->len, 1);
    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_MSU_STREQ(insr->instruction, "BRA");
    ASSERT_MSU_STREQ(insr->label_reference, "FOO");

    list_of_asm_insrs_free(insrs, true);
    msu_str_free(src);
}

TEST(parsing_tests, unknown_instruction_is_handled) {
    const msu_str_t *src = msu_str_new("FOO FOO");
    list_of_asm_insrs_t *insrs = asm_parse(src);

    ASSERT_EQ(insrs->len, 1);
    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_NE(insr->error, nullptr) << "expected error";
    ASSERT_EQ(insr->error->kind, ASM_ERROR_BAD_INSR);

    list_of_asm_insrs_free(insrs, true);
    msu_str_free(src);
}

TEST(parsing_tests, no_argument_for_instruction_causes_error) {
    const msu_str_t *src = msu_str_new("BRZ");
    list_of_asm_insrs_t *insrs = asm_parse(src);

    ASSERT_EQ(insrs->len, 1);
    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_NE(insr->error, nullptr) << "expected error";
    ASSERT_EQ(insr->error->kind, ASM_ERROR_BAD_ARG);

    list_of_asm_insrs_free(insrs, true);
    msu_str_free(src);
}

TEST(parsing_tests, numbers_out_of_machine_range_cause_error) {
    const msu_str_t *src = msu_str_new("DAT 1000");
    list_of_asm_insrs_t *insrs = asm_parse(src);

    ASSERT_EQ(insrs->len, 1);
    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_NE(insr->error, nullptr) << "expected error";
    ASSERT_EQ(insr->error->kind, ASM_ERROR_BAD_ARG);

    list_of_asm_insrs_free(insrs, true);
    msu_str_free(src);
}

TEST(parsing_tests, negative_numbers_out_of_machine_range_are_capped_properly) {
    const msu_str_t *src = msu_str_new("DAT -1000");
    list_of_asm_insrs_t *insrs = asm_parse(src);

    ASSERT_EQ(insrs->len, 1);
    const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, 0);
    ASSERT_NE(insr->error, nullptr) << "expected error";
    ASSERT_EQ(insr->error->kind, ASM_ERROR_BAD_ARG);

    list_of_asm_insrs_free(insrs, true);
    msu_str_free(src);
}

//==========================================================================
// Code generation tests
//==========================================================================

TEST(code_generation, add_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("ADD 1");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 101) << "expected '101' (ADD:100 + value:1)";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, sub_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SUB 1");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 201) << "expected '201' (SUB:200 + value:1)";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, store_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("STA 1");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 301) << "expected '301' (STA:300 + value:1)";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, load_immediate_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("LDI 20");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 420) << "expected '420' (LDI:400 + value:20)";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, load_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("LDA 12");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 512) << "expected '512' (LDA:500 + value:12)";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, branch_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("BRA 42");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 642) << "expected '642' (BRA:600 + value:42)";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, branch_if_zero_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("BRZ 89");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 789) << "expected '789' (BRA:700 + value:89)";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, branch_if_positive_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("BRP 12");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 812) << "expected '812' (BRA:800 + value:12)";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, input_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("INP");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 901) << "expected '901')";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, out_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("OUT");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 902) << "expected '902')";
    ASSERT_EQ(code[1], 0);
    ASSERT_EQ(err, nullptr);

    msu_str_free(src);
    free(code);
}

TEST(code_generation, halt_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("HLT");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 0) << "expected '0'";
    ASSERT_EQ(err, nullptr);

    free(code);
    msu_str_free(src);
}

TEST(code_generation, cob_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("COB");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 0) << "expected '0'";
    ASSERT_EQ(err, nullptr);

    free(code);
    msu_str_free(src);
}

TEST(code_generation, dat_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("DAT 701");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 0) << "expected '701'";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";
    ASSERT_EQ(err, nullptr);

    free(code);
    msu_str_free(src);
}

TEST(code_generation, spush_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SPUSH");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 920) << "expected ''";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";
    ASSERT_EQ(err, nullptr);

    free(code);
    msu_str_free(src);
}

TEST(code_generation, spop_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SPOP");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 921) << "expected 921";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";
    ASSERT_EQ(err, nullptr);

    free(code);
    msu_str_free(src);
}

TEST(code_generation, sdup_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SDUP");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 922) << "expected 922";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";
    ASSERT_EQ(err, nullptr);

    free(code);
    msu_str_free(src);
}

TEST(code_generation, sdrop_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SDROP");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(code[0], 923) << "expected 923";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";
    ASSERT_EQ(err, nullptr);

    free(code);
    msu_str_free(src);
}

TEST(code_generation, sswap_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SSWAP");
    asm_error_t *error = nullptr;
    int *code = asm_assemble(src, &error);

    ASSERT_EQ(code[0], 924) << "expected 924";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";
    ASSERT_EQ(error, nullptr) << msu_str_to_cpp(error->message);

    free(code);
    msu_str_free(src);
}

TEST(code_generation, sadd_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SADD");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 930) << "expected 930";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";

    asm_error_free(err);
    free(code);
    msu_str_free(src);
}

TEST(code_generation, ssub_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SSUB");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 931) << "expected 931";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";

    asm_error_free(err);
    free(code);
    msu_str_free(src);
}

TEST(code_generation, smax_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SMAX");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 934) << "expected 934";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";

    asm_error_free(err);
    free(code);
    msu_str_free(src);
}

TEST(code_generation, smin_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SMIN");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 935) << "expected 935";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}

TEST(code_generation, smul_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SMUL");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 932) << "expected 932";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}

TEST(code_generation, sdiv_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SDIV");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 933) << "expected 933";
    ASSERT_EQ(code[1], 0) << "expected HLT:000";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}

TEST(code_generation, spushi_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("SPUSHI 33");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 433) << "expected 401 (LDI:400 + value:33)";
    ASSERT_EQ(code[1], 920) << "expected SPUSH:920";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}

TEST(code_generation, call_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("CALL 1");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);
    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);

    /// CALL is a synthetic asm_instruction that compiles an LDI of the target address to jump to
    /// and finally a CALL machine code (911)
    ASSERT_EQ(code[0], 401) << "expected 401 (LDI:400 + value:1)";
    ASSERT_EQ(code[1], 910) << "expected JAL:910";
    ASSERT_EQ(code[2], 0) << "expected HLT:000";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}

TEST(code_generation, ret_instruction_generates_properly) {
    const msu_str_t *src = msu_str_new("RET");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);
    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);

    ASSERT_EQ(code[0], 911) << "expected RET:911";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}


TEST(code_generation, instructions_next_to_one_another_generate_in_order_properly) {
    const msu_str_t *src = msu_str_new("SPUSHI 1\n"
                                       "OUT\n"
                                       "HLT\n");

    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 401) << "expected 401 (LDI:400 + value:1)";
    ASSERT_EQ(code[1], 920) << "expected SPUSH:920";
    ASSERT_EQ(code[2], 902) << "expected OUT:902";
    ASSERT_EQ(code[3], 000) << "expected HLT:000";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}

TEST(code_generation, labels_resolve_at_code_generation_time) {
    const msu_str_t *src = msu_str_new("LDA FOO\n"
                                       "FOO DAT 1\n");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    ASSERT_EQ(code[0], 501) << "expected 501 (LDA:500 + FOO:1)";
    ASSERT_EQ(code[1], 1) << "expected 1 (DAT:1)";
    ASSERT_EQ(code[2], 0) << "expected HLT:000";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}

TEST(code_generation, bad_label_causes_error) {
    const msu_str_t *src = msu_str_new("LDA BAR\n"
                                       "FOO DAT 1");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_NE(err, nullptr) << "expected 'BAD_LABEL'";
    ASSERT_EQ(err->kind, ASM_ERROR_BAD_LABEL) << "expected 'BAD_LABEL'";

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}

//==========================================================================
// Complete assembly tests
//==========================================================================

TEST(end_to_end, bootstrap){
    const msu_str_t *src = msu_str_new("INP\nOUT\n");
    asm_error_t *err = nullptr;
    int *code = asm_assemble(src, &err);

    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);

    ASSERT_EQ(code[0], 901);
    ASSERT_EQ(code[1], 902);
    ASSERT_EQ(code[2], 0);

    free(code);
    asm_error_free(err);
    msu_str_free(src);
}
