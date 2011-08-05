// This file contains functions dealing with memory management.
#include <stdlib.h>

#include "oyster.h"

void *initialize_memory_object(size_t size, void *dec)
{
    struct memorable *obj = malloc(size);
    obj->dec = dec;
    obj->ref = 0;
    return obj;
}

void incref(void *x)
{
    if (x)
        ((struct memorable *) x)->ref++;
}

void decref(void *x)
{
    if (x)
        ((struct memorable *) x)->dec(x);
}

//

void table_entry_unref(table_entry * x)
{
    x->ref--;
    if (x->ref == 0) {
        decref(x->it);
        free(x);
    }
}


void table_unref(table * x)
{
    x->ref--;
    if (x->ref <= 0) {
        g_hash_table_unref(x->it);
        g_hash_table_unref(x->leaked);
        free(x);
    }
}

//


//


//

void cons_cell_ref(cons_cell * x)
{
    x->ref++;
}

void cons_cell_unref(cons_cell * x)
{
    x->ref--;
    if (x->ref == 0) {
        decref(x->car);
        decref(x->cdr);
        free(x);
    }
}
