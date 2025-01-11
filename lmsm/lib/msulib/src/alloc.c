#include "msulib/alloc.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <memory.h>

void *msu_malloc(allocator_t a, size_t size) { (void) a; return malloc(size); }
void *msu_realloc(allocator_t a, void *ptr, size_t size) { (void) a; return realloc(ptr, size); }
void msu_free(allocator_t a, void *ptr) { (void) a; free(ptr); }

const allocator_t DEFAULT_ALLOCATOR = (allocator_t) {
        .alloc = msu_malloc,
        .realloc = msu_realloc,
        .free = msu_free,
        .state = NULL,
};

typedef struct mha_var {
    void *ptr;
    struct mha_var *next;
} mha_var_t;

typedef struct managed_heap_alloc_state {
    mha_var_t *allocated;
} managed_heap_alloc_state_t;

void *managed_alloc(allocator_t alloc, size_t size) {
    void *data = malloc(size);
    if (!data) return NULL;

    mha_var_t *new_var = malloc(sizeof(mha_var_t));
    if (!new_var) {
        free(data);
        return NULL;
    }
    new_var->ptr = data;
    new_var->next = NULL;

    managed_heap_alloc_state_t *state = alloc.state;
    new_var->next = state->allocated;
    state->allocated = new_var;

    fprintf(stderr, " ALLOCATED : %#zx\n", (size_t) data);
    fflush(stderr);
    return data;
}

void *managed_realloc(allocator_t alloc, void *ptr, size_t size) {
    managed_heap_alloc_state_t *state = alloc.state;
    mha_var_t *entry = state->allocated;
    while (entry) {
        if (entry->ptr == ptr) {
            break;
        }
        entry = entry->next;
    }
    assert(entry && "pointer does not belong to this allocator!");

    void *data = realloc(ptr, size);
    if (!data) {
        entry->ptr = data;
        return NULL;
    }

    entry->ptr = data;
    fprintf(stderr, "REALLOCATED: %#zx\n", (size_t) data);
    return data;
}

void managed_free(allocator_t alloc, void *ptr) {
    managed_heap_alloc_state_t *state = alloc.state;
    mha_var_t *prev = NULL;
    mha_var_t *entry = state->allocated;
    while (entry) {
        if (entry->ptr == ptr) {
            break;
        }
        prev = entry;
        entry = entry->next;
    }
    assert(entry && "pointer does not belong to this allocator!");

    free(entry->ptr);
    if (prev) {
        prev->next = entry->next;
    } else {
        state->allocated = entry->next;
    }
    free(entry);

    fprintf(stderr, "DEALLOCATED: %zx\n", (size_t) ptr);
    fflush(stderr);
}

allocator_t managed_heap_alloc_new() {
    managed_heap_alloc_state_t *state = malloc(sizeof(managed_heap_alloc_state_t));
    assert(state && "out of memory!\n");
    state->allocated = NULL;

    allocator_t out;
    out.alloc = managed_alloc;
    out.realloc = managed_realloc;
    out.free = managed_free;
    out.state = state;
    return out;
}

void managed_heap_alloc_free(allocator_t *alloc) {
    if (!alloc) return;
    managed_heap_alloc_state_t *state = alloc->state;
    mha_var_t *var = state->allocated;
    while (var) {
        mha_var_t *next = var->next;
        free(var->ptr);
        free(var);
        var = next;
    }
    free(state);
    memset(alloc, 0, sizeof(allocator_t));
}

size_t managed_heap_alloc_debug(allocator_t *alloc) {
    managed_heap_alloc_state_t *state = alloc->state;
    mha_var_t *var = state->allocated;

    size_t n = 0;
    while (var) {
        fprintf(stderr, "pointer was never freed: %#zx\n", (size_t) var->ptr);
        var = var->next;
        n += 1;
    }
    fflush(stderr);
    return n;
}