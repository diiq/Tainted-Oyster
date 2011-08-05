#ifndef OYSTER
#define OYSTER

// This file contains functions that create and manipulate the simplest units:
// oysters.

// An oyster is a scope wrapped around an object, like a blanket around a
// cold, wet, child.

#include <stdio.h>
#include <stdlib.h>

#include "oyster.h"
#include "parsing.h"            // Not sure if this needs be here.


oyster *make_untyped_oyster()
{
    oyster *ret = NEW(oyster);
    ret->in = NEW(inner);
    incref(ret->in);

    ret->in->type = -1;

    ret->in->value = NULL;
    ret->bindings = NULL;
    ret->in->info = make_table();
    incref(ret->in->info);
    return ret;
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
        decref(x->info);
        free(x);
    }
}

oyster *make_oyster(int type)
{
    oyster *ret = make_untyped_oyster();
    ret->in->type = type;
    return ret;
}

int oyster_type(oyster * x)
{
    return x->in->type;
}

void *oyster_value(oyster * x)
{
    return x->in->value;
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


oyster *make_symbol(int symbol_id)
{
    oyster *ret = make_untyped_oyster();
    ret->in->type = SYMBOL;
    ret->in->symbol_id = symbol_id;
    return ret;
}

oyster *oyster_copy(oyster * x, table * new_bindings)
{
    incref(x);
    oyster *ret = NEW(oyster);

    ret->in = x->in;
    incref(ret->in);

    ret->bindings = new_bindings;
    incref(ret->bindings);

    decref(x);
    return ret;
}

void oyster_add_to_bindings(int sym_id, oyster * val, oyster * x)
{
    if (!x->bindings) {
        x->bindings = make_table();
        incref(x->bindings);
    }
    table_put(sym_id, val, x->bindings);
}

#endif

