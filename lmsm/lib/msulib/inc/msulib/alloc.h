#ifndef MSU_ALLOC_H
#define MSU_ALLOC_H

#include <stddef.h>

typedef struct allocator allocator_t;

typedef void *(*alloc_fn)(allocator_t, size_t);
typedef void *(*realloc_fn)(allocator_t, void *, size_t);
typedef void (*free_fn)(allocator_t, void *);

struct allocator {
    alloc_fn alloc;
    realloc_fn realloc;
    free_fn free;
    void *state;
};

#define MSU_ALLOC(allocator, size) ((allocator).alloc((allocator), (size)))
#define MSU_REALLOC(allocator, ptr, size) (allocator).realloc((allocator), (ptr), (size))
#define MSU_FREE(allocator, ptr) (allocator).free((allocator), (ptr))

extern const allocator_t DEFAULT_ALLOCATOR;
allocator_t managed_heap_alloc_new();
void managed_heap_alloc_free(allocator_t *alloc);
size_t managed_heap_alloc_debug(allocator_t *alloc); // prints out which objects were not deallocated, returns # alive

#endif // MSU_ALLOC_H