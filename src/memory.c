#ifndef MEMORY
#define MEMORY

// This file contains functions dealing with memory management.

#include "oyster.h"


void incref(void *x)
{
    if (x)
        ((struct memorable *)x)->inc(x);
}

void decref(void *x)
{
    if (x)
        ((struct memorable *)x)->dec(x);
}

#endif
