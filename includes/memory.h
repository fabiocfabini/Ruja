#ifndef RUJA_MEMORY_H
#define RUJA_MEMORY_H

#include "common.h"
#include <stdlib.h>

//TODO: This should throw an error if the allocation fails not just exit
#define REALLOC_DA(type, da) \
    do { \
        size_t new_capacity = da->capacity == 0 ? 8 : da->capacity * 2; \
        da->items = realloc(da->items, sizeof(type) * new_capacity); \
        if (da->items == NULL) { \
            fprintf(stderr, "Out of memory. Could not allocate %"PRIu64" bytes. for %s\n", sizeof(type) * new_capacity, #da); \
            exit(1); \
        } \
        da->capacity = new_capacity; \
    } while (0)

#endif // RUJA_MEMORY_H