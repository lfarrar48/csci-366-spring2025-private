/*
 * opt.h - Optimizer
 *
 * sometimes generated assembly is noisy
 * this means overhead and extra instructions
 * for high efficiency programs optimization is needed
 * that's what this module is for
*/

#include "lmsm/asm.h"

list_of_asm_insrs_t *asm_optimize(const list_of_asm_insrs_t *insrs);