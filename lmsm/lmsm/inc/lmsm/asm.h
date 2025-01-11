
#ifndef asm_h
#define asm_h

#include <msulib/str.h>
#include <msulib/parser.h>

extern const char *ASM_INSTRUCTIONS[];
extern const size_t ASM_INSTRUCTION_COUNT;

extern const char *ASM_ARG_INSTRUCTIONS[];
extern const size_t ASM_ARG_INSTRUCTION_COUNT;

typedef enum asm_insr_kind {
    ASM_INSR_SSTA = -500,
    ASM_INSR_SLDA = -300,
    ASM_INSR_SDEC = -200,
    ASM_INSR_SINC = -100,
    ASM_INSR_HLT = 0,
    ASM_INSR_COB = 0,
    ASM_INSR_ADD = 100,
    ASM_INSR_SUB = 200,
    ASM_INSR_STA = 300,
    ASM_INSR_LDI = 400,
    ASM_INSR_LDA = 500,
    ASM_INSR_BRA = 600,
    ASM_INSR_BRZ = 700,
    ASM_INSR_BRP = 800,

    ASM_INSR_INP = 901,
    ASM_INSR_OUT = 902,

    ASM_INSR_JAL = 910,
    ASM_INSR_RET = 911,

    ASM_INSR_SPUSH = 920,
    ASM_INSR_SPOP = 921,
    ASM_INSR_SDUP = 922,
    ASM_INSR_SSWAP = 923,
    ASM_INSR_SADD = 930,
    ASM_INSR_SSUB = 931,
    ASM_INSR_SMUL = 932,
    ASM_INSR_SDIV = 933,
    ASM_INSR_SMAX = 934,
    ASM_INSR_SMIN = 935,

    ASM_INSR_DAT = 999,
} asm_insr_kind;

typedef enum asm_error_kind {
    ASM_ERROR_NONE = 0,
    ASM_ERROR_TOO_LARGE,
    ASM_ERROR_BAD_LABEL,
    ASM_ERROR_BAD_ARG,
    ASM_ERROR_BAD_INSR,
} asm_error_kind_t;

typedef struct asm_insr asm_insr_t;
typedef struct asm_error asm_error_t;

struct asm_error {
    asm_error_kind_t kind;
    const msu_str_t *message;
};

struct asm_insr {
    const msu_str_t *label;
    const msu_str_t *instruction;
    int value;
    const msu_str_t *label_reference;
    asm_error_t *error;
};
typedef struct list_of_asm_insrs list_of_asm_insrs_t;

asm_error_t *asm_error_new(asm_error_kind_t kind, const msu_str_t *context);
asm_insr_t *asm_insr_clone(const asm_insr_t *src);
asm_error_t *asm_error_clone(const asm_error_t *src);

int asm_insr_size(const asm_insr_t *insr);

asm_insr_t *asm_parse_insr(const msu_str_t *line);
list_of_asm_insrs_t *asm_parse(const msu_str_t *src);
int *asm_assemble(const msu_str_t *src, asm_error_t **errout);
asm_error_t *asm_emit(list_of_asm_insrs_t *insrs, int *outcode, size_t codesize);

void asm_insr_free(asm_insr_t *insr);
void asm_error_free(asm_error_t *err);

#endif // asm_h

#include "lmsm/asm_insrlist.h"
