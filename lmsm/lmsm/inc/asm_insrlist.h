#ifndef LIST_OF_ASM_INSRS_H
#define LIST_OF_ASM_INSRS_H

#include <stdint.h>
#include <stdbool.h>


typedef struct asm_insr asm_insr_t;
size_t asm_insr_hash(const asm_insr_t *insr, size_t seed);

typedef struct list_of_asm_insrs {
    asm_insr_t * *values;
    size_t len, cap;
} list_of_asm_insrs_t;

list_of_asm_insrs_t *list_of_asm_insrs_new();
list_of_asm_insrs_t *list_of_asm_insrs_clone(const list_of_asm_insrs_t *list);
void list_of_asm_insrs_clear(list_of_asm_insrs_t *list);
asm_insr_t * list_of_asm_insrs_get(list_of_asm_insrs_t *list, size_t index);
asm_insr_t * *list_of_asm_insrs_get_ref(list_of_asm_insrs_t *list, size_t index);
const asm_insr_t * list_of_asm_insrs_get_const(const list_of_asm_insrs_t *list, size_t index);
void list_of_asm_insrs_set(list_of_asm_insrs_t *list, size_t index, asm_insr_t * value);
void list_of_asm_insrs_append(list_of_asm_insrs_t *list, asm_insr_t * value);
#define list_of_asm_insrs_push(list, value) list_of_asm_insrs_append(list, value)
asm_insr_t * list_of_asm_insrs_pop(list_of_asm_insrs_t *list);
void list_of_asm_insrs_ensure_capacity(list_of_asm_insrs_t *list, size_t capacity);
void list_of_asm_insrs_free(list_of_asm_insrs_t *list, bool free_elements_too);


hash_t list_of_asm_insrs_hash(const list_of_asm_insrs_t *list, hash_t seed);

#endif // LIST_OF_ASM_INSRS_H