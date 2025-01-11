#ifndef LIST_OF_PARSENODES_H
#define LIST_OF_PARSENODES_H

#include <stdint.h>
#include <stdbool.h>


typedef struct parsenode parsenode_t;

typedef struct list_of_parsenodes {
    parsenode_t * *values;
    size_t len, cap;
} list_of_parsenodes_t;

list_of_parsenodes_t *list_of_parsenodes_new();
list_of_parsenodes_t *list_of_parsenodes_clone(const list_of_parsenodes_t *list);
void list_of_parsenodes_clear(list_of_parsenodes_t *list);
parsenode_t * list_of_parsenodes_get(list_of_parsenodes_t *list, size_t index);
parsenode_t * *list_of_parsenodes_get_ref(list_of_parsenodes_t *list, size_t index);
const parsenode_t * list_of_parsenodes_get_const(const list_of_parsenodes_t *list, size_t index);
void list_of_parsenodes_set(list_of_parsenodes_t *list, size_t index, parsenode_t * value);
void list_of_parsenodes_append(list_of_parsenodes_t *list, parsenode_t * value);
#define list_of_parsenodes_push(list, value) list_of_parsenodes_append(list, value)
parsenode_t * list_of_parsenodes_pop(list_of_parsenodes_t *list);
void list_of_parsenodes_ensure_capacity(list_of_parsenodes_t *list, size_t capacity);
void list_of_parsenodes_free(list_of_parsenodes_t *list, bool free_elements_too);




#endif // LIST_OF_PARSENODES_H