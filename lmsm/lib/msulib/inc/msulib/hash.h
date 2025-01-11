#include <stdlib.h>
#include <stdint.h>

typedef uint32_t hash_t;
hash_t murmurhash(const void *key, size_t len, hash_t seed);


