#ifndef OYSTER
#define OYSTER

// This file contains functions that create and manipulate the simplest units:
// oysters.

// An oyster is a scope wrapped around an object, like a blanket around a
// cold, wet, child.

#include <stdio.h>
#include <stdlib.h>

#include "oyster.h"


oyster *make_untyped_oyster()
{
    oyster *ret = NEW(oyster);
    ret->in = NEW(inner);
    incref(ret->in);

    ret->in->type = NULL;
    ret->in->gc_type = 0;

    ret->in->value = NULL;
    ret->bindings = NULL;
    ret->in->info = make_table();
    incref(ret->in->info);
    return ret;
}

void inner_free(inner * x)
{
    if (x->gc_type == 1)
         decref(x->value);

    if (x->gc_type == 2)
        free(x->value);
    
    decref(x->type);
    decref(x->info);
    free(x);
}

oyster *make_oyster(int type)
{
    oyster *ret = make_untyped_oyster();
    oyster_set_type(ret, type);
    ret->in->gc_type = 1;
    return ret;
}

int oyster_type(oyster * x)
{
    return x->in->type->it->in->symbol_id; // FOOLISH MORTALS
}

void oyster_set_type_o(oyster *o, oyster *type)
{
    table_entry *e = NEW(table_entry);
    e->it = type;
    incref(type);
    table_put_entry(TYPE, e, o->in->info);
    o->in->type = e;
    incref(e);
}

void oyster_set_type(oyster *o, int type)
{
    oyster *otype = make_symbol(type);
    oyster_set_type_o(o, otype);
}

void *oyster_value(oyster * x)
{
    return x->in->value;
}

void oyster_free(oyster * x)
{
    decref(x->bindings);
    decref(x->in);
    free(x);
}

oyster* symbol_type_symbol(){
    static oyster *ret = NULL;
    if (!ret){
        ret = make_untyped_oyster();
        ret->in->symbol_id = SYMBOL;
        oyster_set_type_o(ret, ret);
        incref(ret);
    }
    return ret;
}

oyster *make_symbol(int symbol_id)
{
    oyster *ret = make_untyped_oyster();
    oyster_set_type_o(ret, symbol_type_symbol());
    ret->in->symbol_id = symbol_id;
    ret->in->gc_type = 0;
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

