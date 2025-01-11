#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include "msulib/parsenodelist.h"
#include "msulib/parser.h"




list_of_parsenodes_t *list_of_parsenodes_new() {
    list_of_parsenodes_t *out = malloc(sizeof(list_of_parsenodes_t));
    assert(out && "out of memory\n");
    memset(out, 0, sizeof(list_of_parsenodes_t));
    return out;
}

list_of_parsenodes_t *list_of_parsenodes_clone(const list_of_parsenodes_t *list) {
    list_of_parsenodes_t *out = malloc(sizeof(list_of_parsenodes_t));
    assert(out && "out of memory\n");
    out->len = list->len;
    out->cap = list->cap;
    void *data = malloc(sizeof(parsenode_t *) * list->cap);
    assert(data && "out of memory!\n");
    out->values = data;
    memcpy(out->values, list->values, sizeof(parsenode_t *) * list->len);
    return out;
}

void list_of_parsenodes_clear(list_of_parsenodes_t *list) {
    for (size_t i = 0; i < list->len; i++) {
        parsenode_free(list_of_parsenodes_get(list, i));
    }
    list->len = 0;
}

parsenode_t * list_of_parsenodes_get(list_of_parsenodes_t *list, size_t index) {
    assert(index < list->len && "index out of bounds read");
    return list->values[index];
}

const parsenode_t * list_of_parsenodes_get_const(const list_of_parsenodes_t *list, size_t index) {
    assert(index < list->len && "out of bounds read");
    return list->values[index];
}


parsenode_t * *list_of_parsenodes_get_ref(list_of_parsenodes_t *list, size_t index) {
    assert(index < list->len && "index out of bounds read");
    return &list->values[index];
}

void list_of_parsenodes_set(list_of_parsenodes_t *list, size_t index, parsenode_t * value) {
    assert(index < list->len && "out of bounds write");
    list->values[index] = value;
}

void list_of_parsenodes_append(list_of_parsenodes_t *list, parsenode_t * value) {
    list_of_parsenodes_ensure_capacity(list, list->len + 1);
    list->values[list->len] = value;
    list->len += 1;
}

parsenode_t * list_of_parsenodes_pop(list_of_parsenodes_t *list) {
    assert(list->len > 0 && "pop empty list");
    list->len -= 1;
    return list->values[list->len];
}

void list_of_parsenodes_ensure_capacity(list_of_parsenodes_t *list, size_t capacity) {
    size_t newcap = list->cap;
    if (!newcap) newcap = 16;
    while (newcap < capacity) {
        newcap *= 2;
    }
    if (newcap != list->cap) {
        void *data = realloc(list->values, sizeof(parsenode_t *) * newcap);
        assert(data && "out of memory\n");
        list->values = data;
        list->cap = newcap;
    }
}

void list_of_parsenodes_free(list_of_parsenodes_t *list, bool free_children_too) {
    if (list) {
        if (free_children_too) {
            for (size_t i = 0; i < list->len; i++) {
                parsenode_free(list_of_parsenodes_get(list, i));
            }
    }
        free(list->values);
        free(list);
    }
}


