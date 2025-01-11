#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include "lmsm/asm.h"



size_t asm_insr_hash(const asm_insr_t *cur, size_t seed) {
    hash_t hashbean[5];
    hashbean[0] = msu_str_hash(cur->label, seed);
    hashbean[1] = msu_str_hash(cur->instruction, seed);
    hashbean[2] = murmurhash(&cur->value, sizeof(cur->value), seed);
    hashbean[3] = msu_str_hash(cur->label_reference, seed);
    hashbean[4] = murmurhash(&cur->error, sizeof(asm_error_t *), seed);
    return (size_t) murmurhash(hashbean, sizeof(hashbean), seed);
}



#include "msulib/hash.h"

list_of_asm_insrs_t *list_of_asm_insrs_new() {
    list_of_asm_insrs_t *out = malloc(sizeof(list_of_asm_insrs_t));
    assert(out && "out of memory\n");
    memset(out, 0, sizeof(list_of_asm_insrs_t));
    return out;
}

list_of_asm_insrs_t *list_of_asm_insrs_clone(const list_of_asm_insrs_t *list) {
    list_of_asm_insrs_t *out = malloc(sizeof(list_of_asm_insrs_t));
    assert(out && "out of memory\n");
    out->len = list->len;
    out->cap = list->cap;
    void *data = malloc(sizeof(asm_insr_t *) * list->cap);
    assert(data && "out of memory!\n");
    out->values = data;
    memcpy(out->values, list->values, sizeof(asm_insr_t *) * list->len);
    return out;
}

void list_of_asm_insrs_clear(list_of_asm_insrs_t *list) {
    for (size_t i = 0; i < list->len; i++) {
        asm_insr_free(list_of_asm_insrs_get(list, i));
    }
    list->len = 0;
}

asm_insr_t * list_of_asm_insrs_get(list_of_asm_insrs_t *list, size_t index) {
    assert(index < list->len && "index out of bounds read");
    return list->values[index];
}

const asm_insr_t * list_of_asm_insrs_get_const(const list_of_asm_insrs_t *list, size_t index) {
    assert(index < list->len && "out of bounds read");
    return list->values[index];
}


asm_insr_t * *list_of_asm_insrs_get_ref(list_of_asm_insrs_t *list, size_t index) {
    assert(index < list->len && "index out of bounds read");
    return &list->values[index];
}

void list_of_asm_insrs_set(list_of_asm_insrs_t *list, size_t index, asm_insr_t * value) {
    assert(index < list->len && "out of bounds write");
    list->values[index] = value;
}

void list_of_asm_insrs_append(list_of_asm_insrs_t *list, asm_insr_t * value) {
    list_of_asm_insrs_ensure_capacity(list, list->len + 1);
    list->values[list->len] = value;
    list->len += 1;
}

asm_insr_t * list_of_asm_insrs_pop(list_of_asm_insrs_t *list) {
    assert(list->len > 0 && "pop empty list");
    list->len -= 1;
    return list->values[list->len];
}

void list_of_asm_insrs_ensure_capacity(list_of_asm_insrs_t *list, size_t capacity) {
    size_t newcap = list->cap;
    if (!newcap) newcap = 16;
    while (newcap < capacity) {
        newcap *= 2;
    }
    if (newcap != list->cap) {
        void *data = realloc(list->values, sizeof(asm_insr_t *) * newcap);
        assert(data && "out of memory\n");
        list->values = data;
        list->cap = newcap;
    }
}

void list_of_asm_insrs_free(list_of_asm_insrs_t *list, bool free_children_too) {
    if (list) {
        if (free_children_too) {
            for (size_t i = 0; i < list->len; i++) {
                asm_insr_free(list_of_asm_insrs_get(list, i));
            }
    }
        free(list->values);
        free(list);
    }
}


hash_t list_of_asm_insrs_hash(const list_of_asm_insrs_t *list, hash_t seed) {
    hash_t hash[2] = {0};
    hash[0] = murmurhash(hash, sizeof(hash), seed);
    for (size_t i = 0; i < list->len; i++) {
        hash[1] = asm_insr_hash(list_of_asm_insrs_get_const(list, i), seed);
        hash[0] = murmurhash(hash, sizeof(hash), seed);
    }
    return hash[0];
}
