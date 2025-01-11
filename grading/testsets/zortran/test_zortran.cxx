
#include "gtest/gtest.h"
#include "testbase.hxx"

extern "C" {
    #include "lmsm/asm.h"
    #include "lmsm/emulator.h"
    #include "lmsm/zortran.h"
}


TEST(test_zortran, test_write) {
   const msu_str_t *src = msu_str_new("WRITE Y");
    parsenode_t *stmt = zt_parse_stmt(src);
    ASSERT_NE(stmt, nullptr) << "parsed statement should not be null";
    ASSERT_EQ(stmt->kind, ZT_WRITE);
    ASSERT_EQ(stmt->token, nullptr) << "'WRITE Y' should not have a token";
    ASSERT_EQ(stmt->children->len, 1) << "'WRITE Y' should have 1 child: a value";

    parsenode_t *val = list_of_parsenodes_get(stmt->children, 0);
    ASSERT_NE(val, nullptr) << "value should not be null";
    ASSERT_EQ(val->kind, ZT_VAR);
    ASSERT_NE(val->token, nullptr) << "var should have a token for the name of it";
    ASSERT_MSU_STREQ(val->token->content, "Y") << "'WRITE Y' should have value 'Y'";

    parsenode_free(stmt);
}

TEST(test_zortran, test_read) {
   const msu_str_t *src = msu_str_new("X = READ");
    parsenode_t *stmt = zt_parse_stmt(src);
    ASSERT_NE(stmt, nullptr) << "parsed statement was null!";
    ASSERT_EQ(stmt->kind, ZT_ASSIGN) << "'X = READ' should parse as ZT_ASSIGN";
    ASSERT_MSU_STREQ(stmt->token->content, "X") << "hi";
    ASSERT_EQ(stmt->children->len, 1) << "assignment should have 1 child: a value";

    parsenode_t *val = list_of_parsenodes_get(stmt->children, 0);
    ASSERT_NE(val, nullptr) << "value must be READ";
    ASSERT_EQ(val->kind, ZT_READ) << "value must be READ";

    msu_str_free(src);

}

TEST(test_zortran, test_multiply) {
   const msu_str_t *src = msu_str_new(R"(
A = 0
X = READ
N = READ
DO WHILE N >= 0
    A = A + X
END
WRITE X
)");
    parsenode_t *program = zt_parse(src);

    ASSERT_NE(program, nullptr) << "program should be null";
    ASSERT_EQ(program->kind, ZT_PROGRAM) << "program should have a ZT_PROGRAM type";
    ASSERT_EQ(program->children->len, 5) << "program should have 5 children";

    parsenode_t *var_a = list_of_parsenodes_get(program->children, 0);
    ASSERT_NE(var_a, nullptr) << "'A = 0' parsed to null";
    ASSERT_EQ(var_a->kind, ZT_ASSIGN) << "'A = 0' should be an assign statement";
    ASSERT_NE(var_a->token, nullptr) << "should have a token for the variable name 'A'";
    ASSERT_MSU_STREQ(var_a->token->content, "A") << "should have a variable name 'A'";
    ASSERT_EQ(var_a->children->len, 1) << "'A' should have 1 child: a value";
    parsenode_t *child = list_of_parsenodes_get(var_a->children, 0);
    ASSERT_NE(child, nullptr) << "'A = 0' did not parse a child";
    ASSERT_EQ(child->kind, ZT_INT) << "the value token for '0' should have type ZT_INT";
    ASSERT_NE(child->token, nullptr) << "the value token should be '0', not null";
    ASSERT_MSU_STREQ(child->token->content, "0") << "the value token should be '0'";

    parsenode_t *var_x = list_of_parsenodes_get(program->children, 1);
    ASSERT_NE(var_x, nullptr) << "'X = READ' parsed to null";
    ASSERT_EQ(var_x->kind, ZT_ASSIGN) << "'X = READ' should be an assign statement";
    ASSERT_NE(var_x->token, nullptr) << "should have a token for the variable name 'X'";
    ASSERT_MSU_STREQ(var_x->token->content, "X") << "should have a variable name 'X'";
    ASSERT_EQ(var_x->children->len, 1) << "'X' should have 1 child: a value";
    child = list_of_parsenodes_get(var_x->children, 0);
    ASSERT_NE(child, nullptr) << "'X = READ' did not parse a child";
    ASSERT_EQ(child->kind, ZT_READ) << "the value token for 'READ' should have type ZT_READ";

    parsenode_t *var_n = list_of_parsenodes_get(program->children, 2);
    ASSERT_NE(var_n, nullptr) << "'N = READ' parsed to null";
    ASSERT_EQ(var_n->kind, ZT_ASSIGN) << "'X = READ' should be an assign statement";
    ASSERT_NE(var_n->token, nullptr) << "should have a token for the variable name 'N'";
    ASSERT_MSU_STREQ(var_n->token->content, "N") << "should have a variable name 'N'";
    ASSERT_EQ(var_n->children->len, 1) << "'N' should have 1 child: a value";
    child = list_of_parsenodes_get(var_n->children, 0);
    ASSERT_NE(child, nullptr) << "'N = READ' did not parse a child";
    ASSERT_EQ(child->kind, ZT_READ) << "the value token for 'READ' should have type ZT_READ";

    parsenode_t *loop = list_of_parsenodes_get(program->children, 3);
    ASSERT_NE(loop, nullptr) << "'DO WHILE ...' parsed to null";
    ASSERT_EQ(loop->kind, ZT_WHILE) << "'DO WHILE ...' should be an ZT_WHILE statement";
    ASSERT_EQ(loop->token, nullptr) << "loop should not have a token";
    ASSERT_EQ(loop->children->len, 2) << "'DO WHILE ...' should have 2 children, a condition and a body block";

    parsenode_t *cond = list_of_parsenodes_get(loop->children, 0);
    ASSERT_NE(cond, nullptr) << "condition is null";
    ASSERT_EQ(cond->kind, ZT_OP);
    ASSERT_NE(cond->token, nullptr) << "cond's token should be '>'";
    ASSERT_MSU_STREQ(cond->token->content, ">=") << "cond's token should be '>='";
    ASSERT_EQ(cond->children->len, 2) << "condition should have 2 children, a left and a right side";
    parsenode_t *lhs = list_of_parsenodes_get(cond->children, 0);
    ASSERT_NE(lhs, nullptr) << "left side of condition should be 'N'";
    ASSERT_EQ(lhs->kind, ZT_VAR) << "left side of condition should be a variable 'N'";
    ASSERT_NE(lhs->token, nullptr) << "ZT_VAR should have a token with its variable name: 'N'";
    ASSERT_MSU_STREQ(lhs->token->content, "N");
    parsenode_t *rhs = list_of_parsenodes_get(cond->children, 1);
    ASSERT_NE(rhs, nullptr) << "right side of condition should be '0'";
    ASSERT_EQ(rhs->kind, ZT_INT) << "right side of condition should be the ZT_INT '0'";
    ASSERT_NE(rhs->token, nullptr) << "right side of condition should be '0'";
    ASSERT_MSU_STREQ(rhs->token->content, "0") << "right side of condition should be '0'";

    parsenode_t *body = list_of_parsenodes_get(loop->children, 1);
    ASSERT_NE(body, nullptr) << "body was null";
    ASSERT_EQ(body->kind, ZT_BLOCK);
    ASSERT_EQ(body->children->len, 1) << "body should have one child: 'A = A + Z'";

    child = list_of_parsenodes_get(body->children, 0);
    ASSERT_NE(child, nullptr) << "'A = A + X' cannot be null";
    ASSERT_EQ(child->kind, ZT_ASSIGN) << "'A = A + X' should be an assign statement";
    ASSERT_EQ(child->children->len, 1) << "'X' should have 1 child: A value";
    parsenode_t *val = list_of_parsenodes_get(child->children, 0);
    ASSERT_NE(val, nullptr) << "'X = READ' parsed to null";
    ASSERT_EQ(val->kind, ZT_OP) << "'A + X' should be an operation";
    ASSERT_NE(val->token, nullptr) << "operator should be '+'";
    ASSERT_MSU_STREQ(val->token->content, "+") << "operator should be '+'";
    ASSERT_EQ(val->children->len, 2) << "operation should have 2 children, a left and a right side";
    lhs = list_of_parsenodes_get(val->children, 0);
    ASSERT_NE(lhs, nullptr) << "lhs in 'A + Z' parsed to null";
    ASSERT_EQ(lhs->kind, ZT_VAR) << "'A' should have type ZT_VAR";
    ASSERT_NE(lhs->token, nullptr) << "var should have a name";
    ASSERT_MSU_STREQ(lhs->token->content, "A") << "variable should have name 'A'";
    rhs = list_of_parsenodes_get(val->children, 1);
    ASSERT_NE(rhs, nullptr) << "lhs in 'A + Z' parsed to null";
    ASSERT_EQ(rhs->kind, ZT_VAR) << "'Z' should have type ZT_VAR";
    ASSERT_NE(rhs->token, nullptr) << "var should have a name";
    ASSERT_MSU_STREQ(rhs->token->content, "X") << "variable should have name 'X'";

    parsenode_free(program);
    msu_str_free(src);
}

TEST(test_zortran, compilation) {
   const msu_str_t *src = msu_str_new(R"(
A = 1
B = 2
C = A + B
WRITE A
WRITE B
WRITE C
)");

    parsenode_t *ast = zt_parse(src);
   const msu_str_t *bc = zt_compile(ast);

    std::string output = std::string(msu_str_data(bc), msu_str_len(bc));
    std::cout << output << std::endl;

    std::string expected{R"(LDA $1
STA A
LDA $2
STA B
LDA A
ADD B
STA C
LDA A
OUT
LDA B
OUT
LDA C
OUT
HLT
A DAT 0
$1 DAT 1
B DAT 0
$2 DAT 2
C DAT 0
)"};
    ASSERT_EQ(expected, output);
}

TEST(zortran_e2e, test_simple) {
    // parse the program
   const msu_str_t *src = msu_str_new("X = READ\nY = READ\nWRITE X + Y");
    parsenode_t *pgm = zt_parse(src);
    msu_str_free(src);

    // compile it to asm code
   const msu_str_t *ascode = zt_compile(pgm);
    parsenode_free(pgm);

    // load asm code to asm-ir
    list_of_asm_insrs_t *asm_ir = asm_parse(ascode);
    msu_str_free(ascode);

    // compile asm-ir to lmsm bytecode
    int code[100] = {0};
    asm_error_t *err = asm_emit(asm_ir, code, 100);
    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    list_of_asm_insrs_free(asm_ir, true);

    // create an emulator, load the code and setup inputs
    emulator_t *em = emulator_new();
    emulator_load(em, code, 100);
    char *input = strdup("12 13");
    em->input_buffer = input;

    emulator_run(em);
    ASSERT_STREQ(em->output_buffer, "25 ");

    free(input);
    emulator_free(em);
}

TEST(zortran_e2e, test_multiply) {
   const msu_str_t *src = msu_str_new(R"(
factor = READ
scaler = READ
result = 0
DO WHILE scaler - 1 >= 0
    result = result + factor
    scaler = scaler - 1
END
WRITE result
)");

    parsenode_t *pgm = zt_parse(src);
    msu_str_free(src);

    const msu_str_t *asm_code = zt_compile(pgm);
    parsenode_free(pgm);

    std::string asm_code_cxx = std::string{msu_str_data(asm_code), msu_str_len(asm_code)};
    std::cout << "```\n" << asm_code_cxx << "\n```" << std::endl;

    list_of_asm_insrs_t *asm_ir = asm_parse(asm_code);
    msu_str_free(asm_code);

    int code[100] = {0};
    asm_error_t *err = asm_emit(asm_ir, code, 100);
    ASSERT_EQ(err, nullptr) << msu_str_to_cpp(err->message);
    list_of_asm_insrs_free(asm_ir, true);

    emulator_t *em = emulator_new();
    emulator_load(em, code, 100);
    char *input = strdup("4 5");
    em->input_buffer = input;

    emulator_run(em);
    ASSERT_STREQ(em->output_buffer, "20 ");

    free(input);
    emulator_free(em);
}