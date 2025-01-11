#include "lmsm/emulator.h"

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
}

int emulator_has_two_values_on_stack(emulator_t *emulator) {
    // TODO - implement capping this value in place
}

//======================================================
//  Instruction Implementation
//======================================================

void emulator_i_jal(emulator_t *emulator) {
    // TODO implement
}

void emulator_i_ret(emulator_t *emulator) {
    // TODO implement
}

void emulator_i_rpush(emulator_t *emulator) {
    // TODO implement & check stack
}

void emulator_i_rpop(emulator_t *emulator) {
    // TODO implement & check stack
}

void emulator_i_spadd(emulator_t *emulator, int value) {
    // TODO implement & check stack
}

void emulator_i_spsub(emulator_t *emulator, int value) {
    // TODO implement & check stack
}

void emulator_i_slda(emulator_t *emulator, int offset) {
    // TODO implement & check offset
}

void emulator_i_ssta(emulator_t *emulator, int offset) {
    // TODO implement & check offset
}

void emulator_i_push(emulator_t *emulator) {
    // TODO implement & check stack
}

void emulator_i_pop(emulator_t *emulator) {
    // TODO implement & check stack
}

void emulator_i_dup(emulator_t *emulator) {
    // TODO implement & check stack
}

void emulator_i_drop(emulator_t *emulator) {
    // TODO implement & check stack
}

void emulator_i_swap(emulator_t *emulator) {
    // TODO implement & check stack has two values
}

void emulator_i_sadd(emulator_t *emulator) {
    // TODO implement, check stack has two values, cap result
}

void emulator_i_ssub(emulator_t *emulator) {
    // TODO implement, check stack has two values, cap result
}

void emulator_i_smul(emulator_t *emulator) {
    // TODO implement, check stack has two values, cap result
}

void emulator_i_sdiv(emulator_t *emulator) {
    // TODO implement, check stack has two values, cap result
}

void emulator_i_smax(emulator_t *emulator) {
    // TODO implement, check stack has two values
}

void emulator_i_smin(emulator_t *emulator) {
    // TODO implement, check stack has two values
}

void emulator_i_scmplt(emulator_t *emulator) {
    // TODO implement, check stack has two values
}

void emulator_i_scmpgt(emulator_t *emulator) {
    // TODO implement, check stack has two values
}

void emulator_i_snot(emulator_t *emulator) {
    // TODO implement, check stack has two values
}

void emulator_i_out(emulator_t *emulator) {
    // TODO implement, append to output_buffer
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
}

void emulator_i_add(emulator_t *emulator, int location) {
    // TODO implement
}

void emulator_i_sub(emulator_t *emulator, int location) {
    // TODO implement
}

void emulator_i_load_immediate(emulator_t *emulator, int value) {
    // TODO implement
}

void emulator_i_store(emulator_t *emulator, int location) {
    // TODO implement
}

void emulator_i_halt(emulator_t *emulator) {
    // TODO implement
}

void emulator_i_branch_unconditional(emulator_t *emulator, int location) {
    // TODO implement
}

void emulator_i_branch_if_zero(emulator_t *emulator, int location) {
    // TODO implement
}

void emulator_i_branch_if_positive(emulator_t *emulator, int location) {
    // TODO implement
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
    } else if (100 <= instruction && instruction <= 199) {
        emulator_i_add(emulator, instruction - 100);
    } else {
        emulator->error_code = ERROR_UNKNOWN_INSTRUCTION;
        emulator->status = STATUS_HALTED;
    }
    emulator_cap_value(&emulator->accumulator); // always cap the accumulator after every step
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
