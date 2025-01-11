#ifndef emulator_H
#define emulator_H

//===================================================================
//  ENUMS for the virtual machine
//===================================================================

typedef enum emulator_machine_status {
    STATUS_RUNNING,
    STATUS_HALTED,
    STATUS_READY,
} emulator_machine_status;

typedef enum emulator_error_code {
    ERROR_NONE,
    ERROR_BAD_STACK,
    ERROR_OUTPUT_EXHAUSTED,
    ERROR_UNKNOWN_INSTRUCTION,
} emulator_error_code;

#define TOP_OF_MEMORY 199
#define MIDDLE_OF_MEMORY 100
#define OUTPUT_BUFFER_SIZE 4000
#define INPUT_BUFFER_SIZE 400

//===================================================================
//  Represents the core computational infrastructure of the
//  LMSM architecture
//===================================================================

typedef struct emulator_t {
    int program_counter;
    emulator_machine_status status;
    emulator_error_code error_code;
    int accumulator;
    int stack_pointer;
    int return_address;
    int return_stack_pointer;
    int memory[TOP_OF_MEMORY + 1];
    char output_buffer[OUTPUT_BUFFER_SIZE];
    char *input_buffer;
} emulator_t;

//=====================================================
// API
//=====================================================

// create a new little man stack machine
emulator_t * emulator_new();

// deletes the machine
void emulator_free(emulator_t *emulator);

// loads a program into a little man stack machine
void emulator_load(emulator_t *emulator, int program[], int length);

// run the little man machine
void emulator_run(emulator_t *emulator);

// step on asm_insr_t on the little man machine
void emulator_step(emulator_t *emulator);

// step on asm_insr_t on the little man machine
void emulator_exec_instruction(emulator_t *emulator, int instruction);

void emulator_reset(emulator_t *emulator);

// machine code must be a pointer to an array of machine
// instructions of size MIDDLE_OF_MEMORY
emulator_t *emulator_exec(int *machine_code);

#endif // emulator_H
