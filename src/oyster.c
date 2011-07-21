#ifndef OYSTER
#define OYSTER

// This file contains functions that create and manipulate the simplest units:
// oysters.

// An oyster is a scope wrapped around an object, like a blanket around a
// cold, wet, child.

#include "stdarg.h"
#include "stdio.h"

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

    return ret;
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
