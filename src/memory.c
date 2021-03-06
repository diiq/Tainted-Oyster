// This file contains functions dealing with memory management.
#include <stdlib.h>

#include "oyster.h"

struct memorable {
    void (*free) (void *);
    int ref;
};

inline void * initialize_memory_object(size_t size, void *freer)
{
    struct memorable *obj = malloc(size);
    obj->free = freer;
    obj->ref = 0;
    return obj;
}

inline void incref(void *x)
{
    if (x)
        ((struct memorable *) x)->ref++;
}

inline void decref(void *x)
{
    if (x) {
        ((struct memorable *) x)->ref--;
        if (((struct memorable *) x)->ref <= 0)
            ((struct memorable *) x)->free(x);
    }
}


