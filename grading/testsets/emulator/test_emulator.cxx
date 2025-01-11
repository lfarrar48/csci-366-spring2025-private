#include "gtest/gtest.h"
extern "C" {
#include "lmsm/emulator.h"
}

TEST(emulator_machine_suite,test_add_instruction_works){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 0;
    emulator->memory[0] = 10;
    emulator_exec_instruction(emulator, 100); // ADD 00
    ASSERT_EQ(emulator->accumulator, 10);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_add_instruction_properly_caps_values){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 995;
    emulator->memory[0] = 10;
    emulator_exec_instruction(emulator, 100); // ADD 00
    ASSERT_EQ(emulator->accumulator, 999);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_sub_instruction_works){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 20;
    emulator->memory[0] = 10;
    emulator_exec_instruction(emulator, 200); // SUB 00
    ASSERT_EQ(emulator->accumulator, 10);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_sub_instruction_properly_caps_values){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = -995;
    emulator->memory[0] = 10;
    emulator_exec_instruction(emulator, 200); // SUB 00
    ASSERT_EQ(emulator->accumulator, -999);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_store_instruction_works){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 300); // STA 00
    ASSERT_EQ(emulator->memory[0], 10);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_load_instruction_works){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 0;
    emulator->memory[20] = 10;
    emulator_exec_instruction(emulator, 520); // LDA 20
    ASSERT_EQ(emulator->accumulator, 10);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_branch_instruction_works){
    emulator_t *emulator = emulator_new();
    ASSERT_EQ(emulator->program_counter, 0);
    emulator_exec_instruction(emulator, 620); // BRA 20
    ASSERT_EQ(emulator->program_counter, 20);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_branch_if_zero_instruction_works_when_val_zero){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 0;
    ASSERT_EQ(emulator->program_counter, 0);
    emulator_exec_instruction(emulator, 720); // BRZ 20
    ASSERT_EQ(emulator->program_counter, 20);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_branch_if_zero_instruction_works_when_val_not_zero){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    ASSERT_EQ(emulator->program_counter, 0);
    emulator_exec_instruction(emulator, 720); // BRZ 20
    ASSERT_EQ(emulator->program_counter, 0);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_branch_if_positive_instruction_works_when_val_zero){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 0;
    ASSERT_EQ(emulator->program_counter, 0);
    emulator_exec_instruction(emulator, 820); // BRZ 20
    ASSERT_EQ(emulator->program_counter, 20);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_branch_if_positive_instruction_works_when_val_positive){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    ASSERT_EQ(emulator->program_counter, 0);
    emulator_exec_instruction(emulator, 820); // BRZ 20
    ASSERT_EQ(emulator->program_counter, 20);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_branch_if_positive_instruction_works_when_val_not_positive){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = -10;
    ASSERT_EQ(emulator->program_counter, 0);
    emulator_exec_instruction(emulator, 820); // BRZ 20
    ASSERT_EQ(emulator->program_counter, 0);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_output_instruction){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 902); // OUT 20
    emulator_exec_instruction(emulator, 902); // OUT 20
    ASSERT_STREQ(emulator->output_buffer, "10 10 ");
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_halt_instruction){
    emulator_t *emulator = emulator_new();
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_READY);
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 000); // HLT
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_load_immediate_instruction){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 0;
    emulator_exec_instruction(emulator, 499); // LDI 99
    ASSERT_EQ(emulator->accumulator, 99);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_push_instruction){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->memory[199], 10);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_pop_instruction_removes_top_of_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->memory[199], 10);
    ASSERT_EQ(emulator->stack_pointer, 199);

    emulator->accumulator = 0;

    emulator_exec_instruction(emulator, 921); // SPOP
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->stack_pointer, 200);

    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_pop_instruction_enters_error_state_if_nothing_to_pop){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 921); // SPOP
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_dup_instruction_duplicates_top_of_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 922); // SDUP
    ASSERT_EQ(emulator->memory[199], 10);
    ASSERT_EQ(emulator->memory[198], 10);
    ASSERT_EQ(emulator->stack_pointer, 198);
    ASSERT_EQ(emulator->accumulator, 10);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_drop_instruction_removes_top_of_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->memory[199], 10);
    ASSERT_EQ(emulator->stack_pointer, 199);

    emulator->accumulator = 0;

    emulator_exec_instruction(emulator, 923); // SDROP
    ASSERT_EQ(emulator->accumulator, 0);
    ASSERT_EQ(emulator->stack_pointer, 200);

    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_drop_instruction_enters_error_state_if_nothing_to_pop){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 923); // SDROP
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}


TEST(emulator_machine_suite,test_swap_instruction_swaps_top_of_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator->accumulator = 20;
    emulator_exec_instruction(emulator, 920); // SPUSH

    ASSERT_EQ(emulator->accumulator, 20);
    ASSERT_EQ(emulator->memory[199], 10);
    ASSERT_EQ(emulator->memory[198], 20);
    ASSERT_EQ(emulator->stack_pointer, 198);

    emulator_exec_instruction(emulator, 924); // SSWAP

    ASSERT_EQ(emulator->accumulator, 20);
    ASSERT_EQ(emulator->memory[199], 20);
    ASSERT_EQ(emulator->memory[198], 10);
    ASSERT_EQ(emulator->stack_pointer, 198);

    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_swap_instruction_enters_error_state_if_nothing_to_swap){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 924); // SSWAP
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}


TEST(emulator_machine_suite,test_sadd_instruction_adds_the_values_on_the_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 930); // SADD
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->memory[199], 20);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_sadd_instruction_properly_caps_values){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 500;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 922); // SDUP
    emulator_exec_instruction(emulator, 930); // SADD
    ASSERT_EQ(emulator->accumulator, 500);
    ASSERT_EQ(emulator->memory[199], 999);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_sadd_instruction_enters_error_state_if_not_enough_elts){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 923); // SADD
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_ssub_instruction_subtracts_the_values_on_the_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 922); // SDUP
    emulator_exec_instruction(emulator, 931); // SSUB
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->memory[199], 0);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_ssub_instruction_properly_caps_values){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = -500;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator->accumulator = 500;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 931); // SSUB
    ASSERT_EQ(emulator->accumulator, 500);
    ASSERT_EQ(emulator->memory[199], -999);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_ssub_instruction_subtracts_the_values_on_the_stack_in_the_right_order){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator->accumulator = 20;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 931); // SSUB
    ASSERT_EQ(emulator->accumulator, 20);
    ASSERT_EQ(emulator->memory[199], -10);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_ssub_instruction_enters_error_state_if_not_enough_elts){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 924); // SSUB
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_smax_instruction_sets_max_val_on_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    ASSERT_EQ(emulator->memory[199], 10);

    emulator->accumulator = 20;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 934); // SMAX
    ASSERT_EQ(emulator->accumulator, 20);
    ASSERT_EQ(emulator->memory[199], 20); // top stack value should now be 20 rather than 10
    ASSERT_EQ(emulator->stack_pointer, 199);

    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_smax_instruction_enters_error_state_if_not_enough_elts){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 934); // SMAX
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_smin_instruction_sets_min_val_on_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 20;
    emulator_exec_instruction(emulator, 920); // SPUSH
    ASSERT_EQ(emulator->memory[199], 20);

    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 935); // SMIN
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->memory[199], 10); // top stack value should now be 10 rather than 20
    ASSERT_EQ(emulator->stack_pointer, 199);

    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_smin_instruction_enters_error_state_if_not_enough_elts){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 935); // SMIN
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_smul_instruction_multiplies_the_values_on_the_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 922); // SDUP
    emulator_exec_instruction(emulator, 932); // SMUL
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->memory[199], 100);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_smul_instruction_properly_caps_values){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 50;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 922); // SDUP
    emulator_exec_instruction(emulator, 932); // SMUL
    ASSERT_EQ(emulator->accumulator, 50);
    ASSERT_EQ(emulator->memory[199], 999);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_smul_instruction_enters_error_state_if_not_enough_elts){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 932); // SMUL
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_sdiv_instruction_divides_the_values_on_the_stack){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 20;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 920); // SPUSH
    emulator_exec_instruction(emulator, 933); // SDIV
    ASSERT_EQ(emulator->accumulator, 10);
    ASSERT_EQ(emulator->memory[199], 2);
    ASSERT_EQ(emulator->stack_pointer, 199);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_sdiv_instruction_enters_error_state_if_not_enough_elts){
    emulator_t *emulator = emulator_new();
    emulator->accumulator = 10;
    emulator_exec_instruction(emulator, 933); // SDIV
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}

TEST(emulator_machine_suite, test_call_instruction_jumps_to_correct_location_and_pushes_next_val_into_ra){
    emulator_t *emulator = emulator_new();

    emulator->program_counter = 5;               // the CALL is in position 4, so 5 (entry instruction) is in the pc
    emulator->accumulator = 10;                  // accumulator value set to 10
    emulator_exec_instruction(emulator, 910); // JAL
    ASSERT_EQ(emulator->program_counter, 10);    // program counter should be updated to the accumulator
    ASSERT_EQ(emulator->return_address, 5);      // return_address should be 5
    emulator_free(emulator);
}

TEST(emulator_machine_suite, return_set_program_counter_to_return_address){
    emulator_t *emulator = emulator_new();
    emulator->program_counter = 50; // current program counter
    emulator->return_address = 2;  // address to return to
    emulator_exec_instruction(emulator, 911); // RET
    ASSERT_EQ(emulator->program_counter, 2); // program counter should be updated to the return_address value on the stack
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_rpush_instruction){
    emulator_t *emulator = emulator_new();
    emulator->return_address = 10;
    emulator_exec_instruction(emulator, 925); // RPUSH
    ASSERT_EQ(emulator->return_address, 10);
    ASSERT_EQ(emulator->memory[100], 10);
    ASSERT_EQ(emulator->return_address_pointer, 100);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_rpop_instruction_removes_top_of_stack){
    emulator_t *emulator = emulator_new();
    emulator->return_address = 10;
    emulator_exec_instruction(emulator, 925); // RPUSH
    ASSERT_EQ(emulator->return_address, 10);
    ASSERT_EQ(emulator->memory[100], 10);
    ASSERT_EQ(emulator->return_address_pointer, 100);

    emulator->return_address = 20;

    emulator_exec_instruction(emulator, 926); // RPOP
    ASSERT_EQ(emulator->return_address, 10);
    ASSERT_EQ(emulator->return_address_pointer, 99);

    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_rpop_instruction_enters_error_state_if_nothing_to_pop){
    emulator_t *emulator = emulator_new();
    emulator->return_address = 10;
    emulator_exec_instruction(emulator, 926); // RPOP
    ASSERT_EQ(emulator->status, emulator_machine_status::STATUS_HALTED);
    ASSERT_EQ(emulator->error_code, emulator_error_code::ERROR_BAD_STACK);
    emulator_free(emulator);
}


TEST(emulator_machine_suite,step_steps_to_the_next_instruction_and_executes_current){

    emulator_t *emulator = emulator_new();

    emulator->accumulator = 10;
    emulator->program_counter = 0;
    emulator->memory[0] = 902; // OUT
    emulator->memory[1] = 000; // HLT

    emulator_step(emulator); // should execute the 902 instruction and move to the entry position

    ASSERT_EQ(emulator->program_counter, 1); // should have bumped the pc
    ASSERT_STREQ(emulator->output_buffer, "10 "); // should have executed the instruction (OUT)

    emulator_free(emulator);
}

TEST(emulator_machine_suite,step_does_not_step_to_the_next_instruction_and_execute_if_machine_is_halted){

    emulator_t *emulator = emulator_new();

    emulator->accumulator = 10;
    emulator->program_counter = 0;
    emulator->memory[0] = 902; // OUT
    emulator->memory[1] = 000; // HLT

    emulator->status = STATUS_HALTED; // halt the machine
    emulator_step(emulator); // should execute the 902 asm_insr_t and move to the entry position

    ASSERT_EQ(emulator->program_counter, 0); // should not have bumped the pc since halted
    ASSERT_STREQ(emulator->output_buffer, ""); // should not have executed the asm_insr_t (OUT)

    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_stadd_instruction_works){
    emulator_t *emulator = emulator_new();
    emulator->stack_pointer = 150;
    emulator_exec_instruction(emulator, -001); // ADD 01
    ASSERT_EQ(emulator->stack_pointer, 151);
    emulator_exec_instruction(emulator, -002); // ADD 02
    ASSERT_EQ(emulator->stack_pointer, 153);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_stsub_instruction_works){
    emulator_t *emulator = emulator_new();
    emulator->stack_pointer = 150;
    emulator_exec_instruction(emulator, -101); // SUB 01
    ASSERT_EQ(emulator->stack_pointer, 149);
    emulator_exec_instruction(emulator, -102); // SUB 02
    ASSERT_EQ(emulator->stack_pointer, 147);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_stack_load_instruction_works){
    emulator_t *emulator = emulator_new();
    emulator->stack_pointer = 150;
    emulator->memory[150] = 50;
    emulator->memory[153] = 40;
    emulator_exec_instruction(emulator, -201); // SLDA 00
    ASSERT_EQ(emulator->stack_pointer, 149);
    ASSERT_EQ(emulator->memory[149], 50);
    emulator_exec_instruction(emulator, -205); // SLDA 04
    ASSERT_EQ(emulator->stack_pointer, 148);
    ASSERT_EQ(emulator->memory[148], 40);
    emulator_free(emulator);
}

TEST(emulator_machine_suite,test_stack_store_instruction_works){
    emulator_t *emulator = emulator_new();
    emulator->stack_pointer = 149;
    emulator->memory[149] = 10;
    emulator->memory[153] = 40;
    emulator_exec_instruction(emulator, -401); // SSTA 00
    ASSERT_EQ(emulator->stack_pointer, 150);
    ASSERT_EQ(emulator->memory[150], 10);

    emulator_exec_instruction(emulator, -403); // SSTA 02
    ASSERT_EQ(emulator->stack_pointer, 151);
    ASSERT_EQ(emulator->memory[153], 10);
    emulator_free(emulator);
}

