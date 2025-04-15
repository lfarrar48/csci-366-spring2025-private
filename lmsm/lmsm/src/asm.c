#include "lmsm/asm.h"
#include "msulib/hash.h"
#include "msulib/str.h"

#include <stdlib.h>
#include "lmsm/emulator.h"

const char *ASM_INSTRUCTIONS[] = {
    "SSTA", "SLDA",
    "SPADD", "SPSUB",
    "HLT", "COB",
    "ADD", "SUB",
    "STA", "LDI", "LDA",
    "BRA", "BRZ", "BRP",
    "INP", "OUT",
    "JAL", "RET",
    "SPUSH", "SPOP", "SDUP", "SDROP", "SSWAP",
    "SADD", "SSUB", "SMUL", "SDIV", "SMAX", "SMIN", "SCMPGT", "SCMPLT", "SNOT",
    "RPUSH", "RPOP",
    "DAT",
    "CALL", "SPUSHI"
};
const size_t ASM_INSTRUCTION_COUNT = sizeof(ASM_INSTRUCTIONS) / sizeof(ASM_INSTRUCTIONS[0]);

const char *ASM_ARG_INSTRUCTIONS[] = {
    "SSTA", "SLDA",
    "SPADD", "SPSUB",
    "ADD", "SUB",
    "STA", "LDI", "LDA",
    "BRA", "BRZ", "BRP", "CALL", "SPUSHI", "DAT"
};

const size_t ASM_ARG_INSTRUCTION_COUNT = sizeof(ASM_ARG_INSTRUCTIONS) / sizeof(ASM_ARG_INSTRUCTIONS[0]);

asm_error_t *asm_error_new(asm_error_kind_t kind, const msu_str_t *context) {
    asm_error_t *out = malloc(sizeof(asm_error_t));
    assert(out && "out of memory!\n");
    out->kind = kind;
    out->message = context;
    return out;
}

bool asm_is_insr(const char *insr) {
    for (size_t i = 0; i < ASM_INSTRUCTION_COUNT; i++) {
        if (strcmp(insr, ASM_INSTRUCTIONS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool asm_is_arg_insr(const char *insr) {
    for (int i = 0; i < ASM_ARG_INSTRUCTION_COUNT; ++i) {
        if (strcmp(insr, ASM_ARG_INSTRUCTIONS[i]) == 0) {
            return true;
        }
    }
    return false;
}

asm_insr_t *asm_parse_insr(const msu_str_t *line) {
    asm_insr_t *new_insr = calloc(1, sizeof(asm_insr_t));
    new_insr->error = NULL;
    new_insr->label = msu_str_new("");
    new_insr->instruction = msu_str_new("");
    new_insr->label_reference = msu_str_new("");
    new_insr->value = 0;

    size_t len = strlen(msu_str_data(line));
    char *buffer = malloc(len + 1);
    strcpy(buffer, msu_str_data(line));

    char *label = strtok(buffer, " \t\n");
    char *insr = strtok(NULL, " \t\n");
    char *arg = strtok(NULL, " \t\n");

    if (label && !asm_is_insr(label)) {
        new_insr->label = msu_str_new(label);
        new_insr->instruction = msu_str_new(insr ? insr : "");
    } else {
        new_insr->label = msu_str_new("");
        new_insr->instruction = msu_str_new(label ? label : "");
        arg = insr;
    }

    if (!asm_is_insr(msu_str_data(new_insr->instruction))) {
        new_insr->error = asm_error_new(ASM_ERROR_BAD_INSR, msu_str_clone(new_insr->instruction));
        free(buffer);
        return new_insr;
    }

    if (asm_is_arg_insr(msu_str_data(new_insr->instruction))) {
        if (!arg) {
            new_insr->error = asm_error_new(ASM_ERROR_BAD_ARG, msu_str_new("missing argument"));
            free(buffer);
            return new_insr;
        }

        if (!isdigit((unsigned char)arg[0]) || strchr(arg, '$') != NULL) {
            new_insr->label_reference = msu_str_new(arg);
            new_insr->value = 0;
            free(buffer);
            return new_insr;
        } else {
            char *endptr;
            int val = strtol(arg, &endptr, 10);
            if (*endptr != '\0' || val > 999 || val < -999) {
                new_insr->error = asm_error_new(ASM_ERROR_BAD_ARG, msu_str_new(arg ? arg : "(null)"));
            } else {
                new_insr->value = val;
            }
        }
    }
    free(buffer);
    return new_insr;
}

list_of_asm_insrs_t *asm_parse(const msu_str_t *src) {
    list_of_asm_insrs_t *insrs = list_of_asm_insrs_new();
    list_of_msu_strs_t *lines = msu_str_splitlines(src);
    for (int lineno = 0; lineno < lines->len; ++lineno) {
        const msu_str_t *line = list_of_msu_strs_get(lines, lineno);
        if (msu_str_is_blank(line)) {
            continue;
        }

        asm_insr_t *insr = asm_parse_insr(line);
        list_of_asm_insrs_append(insrs, insr);
    }

    list_of_msu_strs_free(lines, true);
    return insrs;
}

int asm_insr_size(const asm_insr_t *insr) {
    if (msu_str_eqs(insr->instruction, "CALL")) return 2;
    if (msu_str_eqs(insr->instruction, "SPUSHI")) return 2;
    return 1;
}

int asm_find_label_offset(const list_of_asm_insrs_t *insrs, const msu_str_t *label) {
    // TODO look through the list of instructions for the one with the given label
    //  if none exists return -1
    int pc = 0;
    for (size_t i = 0; i < insrs->len; i++) {
        const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, i);
        if (!msu_str_is_empty(insr->label) && msu_str_eq(insr->label, label)) {
            return pc;
        }
        pc += asm_insr_size(insr);
    }
    return -1;
}

asm_error_t *asm_emit(list_of_asm_insrs_t *insrs, int outcode[], size_t codesize) {
    int off = 0;
    for (size_t i = 0; i < insrs->len; i++) {
        const asm_insr_t *insr = list_of_asm_insrs_get_const(insrs, i);
        int slots = asm_insr_size(insr);
        if (off + slots >= codesize) {
            return asm_error_new(ASM_ERROR_TOO_LARGE, msu_str_new("too many instructions"));
        }

        int value;
        if (!msu_str_is_empty(insr->label_reference)) {
            value = asm_find_label_offset(insrs, insr->label_reference);
            if (value == -1) {
                return asm_error_new(ASM_ERROR_BAD_LABEL, msu_str_printf("unknown label '%s'\n",
                                                                         msu_str_data(insr->label_reference)));
            }
        } else {
            value = insr->value;
        }

        const char *inst = msu_str_data(insr->instruction);

        if (strcmp(inst, "HLT") == 0 || strcmp(inst, "COB") == 0) outcode[off++] = 0;
        else if (strcmp(inst, "ADD") == 0) outcode[off++] = 100 + value;
        else if (strcmp(inst, "SUB") == 0) outcode[off++] = 200 + value;
        else if (strcmp(inst, "STA") == 0) outcode[off++] = 300 + value;
        else if (strcmp(inst, "LDI") == 0) outcode[off++] = 400 + value;
        else if (strcmp(inst, "LDA") == 0) outcode[off++] = 500 + value;
        else if (strcmp(inst, "BRA") == 0) outcode[off++] = 600 + value;
        else if (strcmp(inst, "BRZ") == 0) outcode[off++] = 700 + value;
        else if (strcmp(inst, "BRP") == 0) outcode[off++] = 800 + value;
        else if (strcmp(inst, "INP") == 0) outcode[off++] = 901;
        else if (strcmp(inst, "OUT") == 0) outcode[off++] = 902;
        else if (strcmp(inst, "JAL") == 0) outcode[off++] = 910;
        else if (strcmp(inst, "RET") == 0) outcode[off++] = 911;
        else if (strcmp(inst, "SPUSH") == 0) outcode[off++] = 920;
        else if (strcmp(inst, "SPOP") == 0) outcode[off++] = 921;
        else if (strcmp(inst, "SDUP") == 0) outcode[off++] = 922;
        else if (strcmp(inst, "SDROP") == 0) outcode[off++] = 923;
        else if (strcmp(inst, "SSWAP") == 0) outcode[off++] = 924;
        else if (strcmp(inst, "SADD") == 0) outcode[off++] = 930;
        else if (strcmp(inst, "SSUB") == 0) outcode[off++] = 931;
        else if (strcmp(inst, "SMUL") == 0) outcode[off++] = 932;
        else if (strcmp(inst, "SDIV") == 0) outcode[off++] = 933;
        else if (strcmp(inst, "SMAX") == 0) outcode[off++] = 934;
        else if (strcmp(inst, "SMIN") == 0) outcode[off++] = 935;
        else if (strcmp(inst, "SPUSHI") == 0) {
            outcode[off++] = 400 + value;
            outcode[off++] = 920;
        } else if (strcmp(inst, "CALL") == 0) {
            outcode[off++] = 400 + value;
            outcode[off++] = 910;
        } else if (strcmp(inst, "DAT") == 0) {
            outcode[off++] = value;
        } else {
            return asm_error_new(ASM_ERROR_BAD_INSR, msu_str_printf("unknown instruction '%s'\n", inst));
        }
    }

    return NULL;
}

asm_insr_t *asm_insr_clone(const asm_insr_t *src) {
    asm_insr_t *out = malloc(sizeof(asm_insr_t));
    assert(out && "out of memory!\n");
    out->label = msu_str_clone(src->label);
    out->instruction = msu_str_clone(src->instruction);
    out->value = src->value;
    out->label_reference = msu_str_clone(src->label_reference);
    out->error = src->error;
    return out;
}

asm_error_t *asm_error_clone(const asm_error_t *src) {
    asm_error_t *out = malloc(sizeof(asm_error_t));
    assert(out && "out of memory!\n");
    out->kind = src->kind;
    out->message = msu_str_clone(src->message);
    return out;
}

void asm_insr_free(asm_insr_t *current) {
    if (current) {
        msu_str_free(current->label);
        msu_str_free(current->instruction);
        msu_str_free(current->label_reference);
        asm_error_free(current->error);
        free(current);
    }
}

void asm_error_free(asm_error_t *err) {
    if (err) {
        msu_str_free(err->message);
        free(err);
    }
}

bool asm_report_errors(list_of_asm_insrs_t *insrs) {
    bool errors = false;
    for (int i = 0; i < insrs->len; ++i) {
        const asm_insr_t *insr = list_of_asm_insrs_get(insrs, i);
        if (insr->error) {
            if (!errors) {
                printf("\n\n"
                       "######################################\n"
                       "# Assembler Errors:\n"
                       "######################################\n\n");
            }
            errors = true;
            printf("  %s - %s\n", msu_str_data(insr->instruction), msu_str_data(insr->error->message));
        }
    }
    if (errors) {
        printf("\n\n");
    }
    return errors;
}

int *asm_assemble(const msu_str_t *src, asm_error_t **err) {
    list_of_asm_insrs_t *insrs = asm_parse(src);

    int *code = (int *) calloc(MIDDLE_OF_MEMORY, sizeof(int));
    assert(code && "out of memory!\n");

    if (!asm_report_errors(insrs)) {
        *err = asm_emit(insrs, code, MIDDLE_OF_MEMORY);
    }
    list_of_asm_insrs_free(insrs, true);
    return code;
}
