#include "../inc/lmsm/emulator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>


//======================================================
//  Utilities
//======================================================

void emulator_cap_value(int * val){
    // TODO - implement capping this value in place
    if (*val > 999) *val = 999;
    if (*val < -999) *val = -999;
}

int emulator_has_two_values_on_stack(emulator_t *emulator) {
    // TODO - implement capping this value in place
    return emulator->stack_pointer <= TOP_OF_MEMORY - 1;
}

//======================================================
//  Instruction Implementation
//======================================================

void emulator_i_jal(emulator_t *emulator) {
    // TODO implement
    emulator->return_address = emulator->program_counter;
    emulator->program_counter = emulator->accumulator;
}

void emulator_i_ret(emulator_t *emulator) {
    // TODO implement
    emulator->program_counter = emulator->return_address;
}

void emulator_i_rpush(emulator_t *emulator) {
    // TODO implement & check stack
    if (emulator->return_stack_pointer >= TOP_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    emulator->return_stack_pointer++;
    emulator->memory[emulator->return_stack_pointer] = emulator->return_address;
}

void emulator_i_rpop(emulator_t *emulator) {
    // TODO implement & check stack
    if (emulator->return_stack_pointer < MIDDLE_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    emulator->return_address = emulator->memory[emulator->return_stack_pointer];
    emulator->return_stack_pointer--;
}

void emulator_i_spadd(emulator_t *emulator, int value) {
    // TODO implement & check stack
    emulator->stack_pointer += (1 + value);
    if (emulator->stack_pointer > TOP_OF_MEMORY + 1 || emulator->stack_pointer < MIDDLE_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
    }
}

void emulator_i_spsub(emulator_t *emulator, int value) {
    // TODO implement & check stack
    emulator->stack_pointer -= (1 + value);
    if (emulator->stack_pointer > TOP_OF_MEMORY + 1 || emulator->stack_pointer < MIDDLE_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
    }
}

void emulator_i_slda(emulator_t *emulator, int offset) {
    // TODO implement & check offset
    int index = emulator->stack_pointer + offset;
    if (index < MIDDLE_OF_MEMORY || index > TOP_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    emulator->stack_pointer--;
    emulator->memory[emulator->stack_pointer] = emulator->memory[index];
}

void emulator_i_ssta(emulator_t *emulator, int offset) {
    // TODO implement & check offset
    if (emulator->stack_pointer >= TOP_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }

    int value = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++; // pop

    int index = emulator->stack_pointer + offset;

    if (index < MIDDLE_OF_MEMORY || index > TOP_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }

    emulator->memory[index] = value;
}

void emulator_i_push(emulator_t *emulator) {
    // TODO implement & check stack
    if (emulator->stack_pointer <= MIDDLE_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    emulator->stack_pointer--;
    emulator->memory[emulator->stack_pointer] = emulator->accumulator;
}

void emulator_i_pop(emulator_t *emulator) {
    // TODO implement & check stack
    if (emulator->stack_pointer >= TOP_OF_MEMORY + 1) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    emulator->accumulator = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
}

void emulator_i_dup(emulator_t *emulator) {
    // TODO implement & check stack
    if (emulator->stack_pointer <= MIDDLE_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    int value = emulator->memory[emulator->stack_pointer];
    emulator_i_push(emulator);
    emulator->memory[emulator->stack_pointer] = value;
}

void emulator_i_drop(emulator_t *emulator) {
    // TODO implement & check stack
    if (emulator->stack_pointer >= TOP_OF_MEMORY + 1) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    emulator->stack_pointer++;
}

void emulator_i_swap(emulator_t *emulator) {
    // TODO implement & check stack has two values
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    int a = emulator->memory[emulator->stack_pointer];
    int b = emulator->memory[emulator->stack_pointer + 1];
    emulator->memory[emulator->stack_pointer] = b;
    emulator->memory[emulator->stack_pointer + 1] = a;
}

void emulator_i_sadd(emulator_t *emulator) {
    // TODO implement, check stack has two values, cap result
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    int a = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
    int b = emulator->memory[emulator->stack_pointer];
    int result = a + b;
    emulator_cap_value(&result);
    emulator->memory[emulator->stack_pointer] = result;
}

void emulator_i_ssub(emulator_t *emulator) {
    // TODO implement, check stack has two values, cap result
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    int a = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
    int b = emulator->memory[emulator->stack_pointer];
    int result = b - a;
    emulator_cap_value(&result);
    emulator->memory[emulator->stack_pointer] = result;
}

void emulator_i_smul(emulator_t *emulator) {
    // TODO implement, check stack has two values, cap result
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    int a = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
    int b = emulator->memory[emulator->stack_pointer];
    int result = a * b;
    emulator_cap_value(&result);
    emulator->memory[emulator->stack_pointer] = result;
}

void emulator_i_sdiv(emulator_t *emulator) {
    // TODO implement, check stack has two values, cap result
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    int a = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
    int b = emulator->memory[emulator->stack_pointer];
    if (a == 0) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }
    int result = b / a;
    emulator_cap_value(&result);
    emulator->memory[emulator->stack_pointer] = result;
}

void emulator_i_smax(emulator_t *emulator) {
    // TODO implement, check stack has two values
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }

    int a = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
    int b = emulator->memory[emulator->stack_pointer];

    int result = (a > b) ? a : b;
    emulator->memory[emulator->stack_pointer] = result;
}

void emulator_i_smin(emulator_t *emulator) {
    // TODO implement, check stack has two values
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }

    int a = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
    int b = emulator->memory[emulator->stack_pointer];

    int result = (a < b) ? a : b;
    emulator->memory[emulator->stack_pointer] = result;
}

void emulator_i_scmplt(emulator_t *emulator) {
    // TODO implement, check stack has two values
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }

    int a = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
    int b = emulator->memory[emulator->stack_pointer];

    int result = (b < a) ? 1 : 0;
    emulator->memory[emulator->stack_pointer] = result;
}

void emulator_i_scmpgt(emulator_t *emulator) {
    // TODO implement, check stack has two values
    if (!emulator_has_two_values_on_stack(emulator)) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }

    int a = emulator->memory[emulator->stack_pointer];
    emulator->stack_pointer++;
    int b = emulator->memory[emulator->stack_pointer];

    int result = (b > a) ? 1 : 0;
    emulator->memory[emulator->stack_pointer] = result;
}

void emulator_i_snot(emulator_t *emulator) {
    // TODO implement, check stack has two values
    if (emulator->stack_pointer >= TOP_OF_MEMORY) {
        emulator->error_code = ERROR_BAD_STACK;
        emulator->status = STATUS_HALTED;
        return;
    }

    int val = emulator->memory[emulator->stack_pointer];
    emulator->memory[emulator->stack_pointer] = (val == 0) ? 1 : 0;
}

void emulator_i_out(emulator_t *emulator) {
    // TODO implement, append to output_buffer
    char buf[12];
    snprintf(buf, sizeof(buf), "%d ", emulator->accumulator);
    strncat(emulator->output_buffer, buf, OUTPUT_BUFFER_SIZE - strlen(emulator->output_buffer) - 1);
}

void emulator_i_inp(emulator_t *emulator) {
    int n;
    if (NULL == emulator->input_buffer) {
        printf("Enter a number: ");
        scanf("%d", &n);
    } else {
        char *tok;
        // this is a mess but it indeed works
        if ((size_t) emulator->input_buffer != (size_t) -1) {
            tok = strtok(emulator->input_buffer, " \r\n\t");
            emulator->input_buffer = (char *) (size_t) -1;
        } else {
            tok = strtok(NULL, " \r\n\t");
        }

        sscanf(tok, "%d", &n);
    }
    emulator->accumulator = n;
}

void emulator_i_load(emulator_t *emulator, int location) {
    // TODO implement
    emulator->accumulator = emulator->memory[location];
}

void emulator_i_add(emulator_t *emulator, int location) {
    // TODO implement
    emulator->accumulator += emulator->memory[location];
}

void emulator_i_sub(emulator_t *emulator, int location) {
    // TODO implement
    emulator->accumulator -= emulator->memory[location];
}

void emulator_i_load_immediate(emulator_t *emulator, int value) {
    // TODO implement
    emulator->accumulator = value;
}

void emulator_i_store(emulator_t *emulator, int location) {
    // TODO implement
    emulator->memory[location] = emulator->accumulator;
}

void emulator_i_halt(emulator_t *emulator) {
    // TODO implement
    emulator->status = STATUS_HALTED;
}

void emulator_i_branch_unconditional(emulator_t *emulator, int location) {
    // TODO implement
    emulator->program_counter = location;
}

void emulator_i_branch_if_zero(emulator_t *emulator, int location) {
    // TODO implement
    if (emulator->accumulator == 0) {
        emulator->program_counter = location;
    }
}

void emulator_i_branch_if_positive(emulator_t *emulator, int location) {
    // TODO implement
    if (emulator->accumulator >= 0) {
        emulator->program_counter = location;
    }
}

void emulator_step(emulator_t *emulator) {
    if (emulator->status != STATUS_HALTED) {
        int instruction = emulator->memory[emulator->program_counter];
        emulator->program_counter++;
        emulator_exec_instruction(emulator, instruction);
    }
}

void emulator_debug(emulator_t *emulator) {
    printf("Emulator:\n\n");
    printf("  PC  : %i\n", emulator->program_counter);
    printf("  SP  : %i\n", emulator->stack_pointer);
    printf("  RA  : %i", emulator->return_address);
    printf("  RAP : %i", emulator->return_stack_pointer);
    printf("  Stack : [ ");
    int sp = emulator->stack_pointer;
    while (sp < TOP_OF_MEMORY + 1) {
        printf("%i ", emulator->memory[sp++]);
    }
    printf("]\n");
    printf("  Return Stack : [ ");
    int ra = MIDDLE_OF_MEMORY;
    while (ra <= emulator->return_stack_pointer) {
        printf("%i ", emulator->memory[ra++]);
    }
    printf("]\n");
    printf("\n");
}

//======================================================
//  emulator_t Implementation
//======================================================

void emulator_exec_instruction(emulator_t *emulator, int instruction) {
    // emulator_debug(emulator); // uncomment to print out the emulator state on each iteration

    if (instruction == 0) {
        emulator_i_halt(emulator);
    }
    else if (100 <= instruction && instruction <= 199) {
        emulator_i_add(emulator, instruction - 100);
    }
    else if (200 <= instruction && instruction <= 299) {
        emulator_i_sub(emulator, instruction - 200);
    }
    else if (300 <= instruction && instruction <= 399) {
        emulator_i_store(emulator, instruction - 300);
    }
    else if (400 <= instruction && instruction <= 499) {
        emulator_i_load_immediate(emulator, instruction - 400);
    }
    else if (500 <= instruction && instruction <= 599) {
        emulator_i_load(emulator, instruction - 500);
    }
    else if (600 <= instruction && instruction <= 699) {
        emulator_i_branch_unconditional(emulator, instruction - 600);
    }
    else if (700 <= instruction && instruction <= 799) {
        emulator_i_branch_if_zero(emulator, instruction - 700);
    }
    else if (800 <= instruction && instruction <= 899) {
        emulator_i_branch_if_positive(emulator, instruction - 800);
    }
    else if (instruction == 901) {
        emulator_i_inp(emulator);
    }
    else if (instruction == 902) {
        emulator_i_out(emulator);
    }
    else if (instruction == 910) {
        emulator_i_jal(emulator);
    }
    else if (instruction == 911) {
        emulator_i_ret(emulator);
    }
    else if (instruction == 920) {
        emulator_i_push(emulator);
    }
    else if (instruction == 921) {
        emulator_i_pop(emulator);
    }
    else if (instruction == 922) {
        emulator_i_dup(emulator);
    }
    else if (instruction == 923) {
        emulator_i_drop(emulator);
    }
    else if (instruction == 924) {
        emulator_i_swap(emulator);
    }
    else if (instruction == 925) {
        emulator_i_rpush(emulator);
    }
    else if (instruction == 926) {
        emulator_i_rpop(emulator);
    }
    else if (instruction == 930) {
        emulator_i_sadd(emulator);
    }
    else if (instruction == 931) {
        emulator_i_ssub(emulator);
    }
    else if (instruction == 932) {
        emulator_i_smul(emulator);
    }
    else if (instruction == 933) {
        emulator_i_sdiv(emulator);
    }
    else if (instruction == 934) {
        emulator_i_smax(emulator);
    }
    else if (instruction == 935) {
        emulator_i_smin(emulator);
    }
    else if (instruction == 937) {
        emulator_i_scmpgt(emulator);
    }
    else if (instruction == 938) {
        emulator_i_scmplt(emulator);
    }
    else if (instruction == 939) {
        emulator_i_snot(emulator);
    }
    else if (-99 <= instruction && instruction <= -1) {
        emulator_i_spadd(emulator, -(instruction + 1));
    }
    else if (-199 <= instruction && instruction <= -100) {
        emulator_i_spsub(emulator, -(instruction + 101));
    }
    else if (-299 <= instruction && instruction <= -200) {
        emulator_i_slda(emulator, -(instruction + 201));
    }
    else if (-500 <= instruction && instruction <= -401) {
        emulator_i_ssta(emulator, -(instruction + 401));
    }
    else {
        emulator->error_code = ERROR_UNKNOWN_INSTRUCTION;
        emulator->status = STATUS_HALTED;
    }

    emulator_cap_value(&emulator->accumulator);
}

void emulator_load(emulator_t *emulator, int *program, int length) {
    for (int i = 0; i < length; ++i) {
        emulator->memory[i] = program[i];
    }
}

void emulator_init(emulator_t *the_machine) {
    the_machine->accumulator = 0;
    the_machine->status = STATUS_READY;
    the_machine->error_code = ERROR_NONE;
    the_machine->program_counter = 0;
    the_machine->stack_pointer = TOP_OF_MEMORY + 1;
    the_machine->return_address = 0;
    the_machine->return_stack_pointer = MIDDLE_OF_MEMORY - 1;
    the_machine->input_buffer = NULL;
    memset(the_machine->output_buffer, 0, sizeof(char) * OUTPUT_BUFFER_SIZE);
    memset(the_machine->memory, 0, sizeof(int) * (TOP_OF_MEMORY + 1));
}

void emulator_reset(emulator_t *emulator) {
    emulator_init(emulator);
}

void emulator_run(emulator_t *emulator) {
    emulator->status = STATUS_RUNNING;
    while (emulator->status != STATUS_HALTED) {
        emulator_step(emulator);
    }
}

emulator_t *emulator_new() {
    emulator_t *emulator = malloc(sizeof(emulator_t));
    assert(emulator && "out of memory\n");
    emulator_init(emulator);
    return emulator;
}

void emulator_free(emulator_t *emulator) {
    free(emulator);
}

emulator_t *emulator_exec(int *machine_code) {
    emulator_t *emulator = emulator_new();
    emulator_load(emulator, machine_code, MIDDLE_OF_MEMORY);
    emulator_run(emulator);
    return emulator;
}
