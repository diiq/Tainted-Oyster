#ifndef MEMORY
#define MEMORY

// This file contains functions dealing with memory management.
#include <stdlib.h>

#include "oyster.h"

void *initialize_memory_object(size_t size, void *inc, void *dec)
{
    struct memorable *obj = malloc(size);
    obj->inc = inc;
    obj->dec = dec;
    obj->ref = 0;
    return obj;
}

void incref(void *x)
{
    if (x)
        ((struct memorable *) x)->inc(x);
}

void decref(void *x)
{
    if (x)
        ((struct memorable *) x)->dec(x);
}

//

void table_entry_ref(table_entry * x)
{
    x->ref++;
}

void table_entry_unref(table_entry * x)
{
    x->ref--;
    if (x->ref == 0) {
        decref(x->it);
        free(x);
    }
}


void table_ref(table * x)
{
    x->ref++;
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

void oyster_ref(oyster * x)
{
    x->ref++;
}

void oyster_unref(oyster * x)
{
    x->ref--;
    if (x->ref <= 0) {
        decref(x->bindings);
        decref(x->in);
        free(x);
    }
}

//

void inner_ref(inner * x)
{
    x->ref++;
}

void inner_unref(inner * x)
{
    x->ref--;
    if (x->ref <= 0) {
        if (x->type != SYMBOL &&
            x->type != NIL &&
            x->type != BUILT_IN_FUNCTION && x->type != -1) {
            decref(x->value);
        }
        //        decref(x->info);
        free(x);
    }
}

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

//

void machine_ref(machine * x)
{
    x->ref++;
}

void machine_unref(machine * x)
{
    x->ref--;
    if (x->ref <= 0) {
        decref(x->base_frame);
        decref(x->current_frame);
        decref(x->now);
        decref(x->accumulator);
        free(x);
    }
}

//

void frame_ref(frame * x)
{
    x->ref++;
}

void frame_unref(frame * x)
{
    x->ref--;
    if (x->ref <= 0) {
        decref(x->below);

        decref(x->scope);
        decref(x->scope_to_be);
        decref(x->scope_below);

        decref(x->instruction);
        free(x);
    }
}

//

void number_ref(number * n)
{
    n->ref++;
}

void number_unref(number * n)
{
    n->ref--;
    if (n->ref <= 0) {
        free(n);
    }
}

//

#endif
