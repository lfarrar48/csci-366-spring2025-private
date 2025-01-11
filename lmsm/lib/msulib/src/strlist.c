#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include "msulib/str.h"


#include "msulib/hash.h"

list_of_msu_strs_t *list_of_msu_strs_new() {
    list_of_msu_strs_t *out = malloc(sizeof(list_of_msu_strs_t));
    assert(out && "out of memory\n");
    memset(out, 0, sizeof(list_of_msu_strs_t));
    return out;
}

list_of_msu_strs_t *list_of_msu_strs_clone(const list_of_msu_strs_t *list) {
    list_of_msu_strs_t *out = malloc(sizeof(list_of_msu_strs_t));
    assert(out && "out of memory\n");
    out->len = list->len;
    out->cap = list->cap;
    void *data = malloc(sizeof(const msu_str_t *) * list->cap);
    assert(data && "out of memory!\n");
    out->values = data;
    memcpy(out->values, list->values, sizeof(const msu_str_t *) * list->len);
    return out;
}

void list_of_msu_strs_clear(list_of_msu_strs_t *list) {
    for (size_t i = 0; i < list->len; i++) {
        msu_str_free(list_of_msu_strs_get(list, i));
    }
    list->len = 0;
}

const msu_str_t * list_of_msu_strs_get(list_of_msu_strs_t *list, size_t index) {
    assert(index < list->len && "index out of bounds read");
    return list->values[index];
}

const const msu_str_t * list_of_msu_strs_get_const(const list_of_msu_strs_t *list, size_t index) {
    assert(index < list->len && "out of bounds read");
    return list->values[index];
}


const msu_str_t * *list_of_msu_strs_get_ref(list_of_msu_strs_t *list, size_t index) {
    assert(index < list->len && "index out of bounds read");
    return &list->values[index];
}

void list_of_msu_strs_set(list_of_msu_strs_t *list, size_t index, const msu_str_t * value) {
    assert(index < list->len && "out of bounds write");
    list->values[index] = value;
}

void list_of_msu_strs_append(list_of_msu_strs_t *list, const msu_str_t * value) {
    list_of_msu_strs_ensure_capacity(list, list->len + 1);
    list->values[list->len] = value;
    list->len += 1;
}

const msu_str_t * list_of_msu_strs_pop(list_of_msu_strs_t *list) {
    assert(list->len > 0 && "pop empty list");
    list->len -= 1;
    return list->values[list->len];
}

void list_of_msu_strs_ensure_capacity(list_of_msu_strs_t *list, size_t capacity) {
    size_t newcap = list->cap;
    if (!newcap) newcap = 16;
    while (newcap < capacity) {
        newcap *= 2;
    }
    if (newcap != list->cap) {
        void *data = realloc(list->values, sizeof(const msu_str_t *) * newcap);
        assert(data && "out of memory\n");
        list->values = data;
        list->cap = newcap;
    }
}

void list_of_msu_strs_free(list_of_msu_strs_t *list, bool free_children_too) {
    if (list) {
        if (free_children_too) {
            for (size_t i = 0; i < list->len; i++) {
                msu_str_free(list_of_msu_strs_get(list, i));
            }
    }
        free(list->values);
        free(list);
    }
}

bool list_of_msu_strs_contains(const list_of_msu_strs_t *list, const msu_str_t * value) {
    for (size_t i = 0; i < list->len; i++) {
        if (msu_str_eq(list_of_msu_strs_get_const(list, i), value)) {
            return true;
        }
    }
    return false;
}

hash_t list_of_msu_strs_hash(const list_of_msu_strs_t *list, hash_t seed) {
    hash_t hash[2] = {0};
    hash[0] = murmurhash(hash, sizeof(hash), seed);
    for (size_t i = 0; i < list->len; i++) {
        hash[1] = msu_str_hash(list_of_msu_strs_get_const(list, i), seed);
        hash[0] = murmurhash(hash, sizeof(hash), seed);
    }
    return hash[0];
}
