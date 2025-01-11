#ifndef LIST_OF_MSU_STRS_H
#define LIST_OF_MSU_STRS_H

#include <stdint.h>
#include <stdbool.h>


typedef struct msu_str msu_str_t;

typedef struct list_of_msu_strs {
    const msu_str_t * *values;
    size_t len, cap;
} list_of_msu_strs_t;

list_of_msu_strs_t *list_of_msu_strs_new();
list_of_msu_strs_t *list_of_msu_strs_clone(const list_of_msu_strs_t *list);
void list_of_msu_strs_clear(list_of_msu_strs_t *list);
const msu_str_t * list_of_msu_strs_get(list_of_msu_strs_t *list, size_t index);
const msu_str_t * *list_of_msu_strs_get_ref(list_of_msu_strs_t *list, size_t index);
const msu_str_t * list_of_msu_strs_get_const(const list_of_msu_strs_t *list, size_t index);
void list_of_msu_strs_set(list_of_msu_strs_t *list, size_t index, const msu_str_t * value);
void list_of_msu_strs_append(list_of_msu_strs_t *list, const msu_str_t * value);
#define list_of_msu_strs_push(list, value) list_of_msu_strs_append(list, value)
const msu_str_t * list_of_msu_strs_pop(list_of_msu_strs_t *list);
void list_of_msu_strs_ensure_capacity(list_of_msu_strs_t *list, size_t capacity);
void list_of_msu_strs_free(list_of_msu_strs_t *list, bool free_elements_too);

bool list_of_msu_strs_contains(const list_of_msu_strs_t *list, const msu_str_t * value);
hash_t list_of_msu_strs_hash(const list_of_msu_strs_t *list, hash_t seed);

#endif // LIST_OF_MSU_STRS_H