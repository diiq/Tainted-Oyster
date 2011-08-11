#ifndef OYSTER
#define OYSTER

// This file contains functions that create and manipulate the simplest units:
// oysters.

// An oyster is a scope wrapped around an object, like a blanket around a
// cold, wet, child.

#include <stdio.h>
#include <stdlib.h>

#include "oyster.h"

typedef struct inner inner;

struct oyster {
    void (*decref) (oyster * x);
    int ref;

    table *bindings;
    inner *in;
};


struct inner {
    void (*decref) (inner * x);
    int ref;
    int gc_type;

    table *info;
    int type;
    union {
        int symbol_id;
        oyster *(*built_in) (machine * m);
        void *value;
    };
};

void inner_free(inner * x);




oyster *make_untyped_oyster()
{
    oyster *ret = NEW(oyster);
    ret->in = NEW(inner);
    incref(ret->in);

    ret->in->type = -1;
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
    
    //    decref(x->type);
    decref(x->info);
    free(x);
}

oyster *make_oyster(int type)
{
    oyster *ret = make_untyped_oyster();
    oyster_assign_type(ret, type);
    ret->in->gc_type = 1;
    return ret;
}

table *oyster_info(oyster *o)
{
    return o->in->info;
}

inline int oyster_type(oyster * x)
{
    return x->in->type;//->it->in->symbol_id; // FOOLISH MORTALS
}

void oyster_assign_type_o(oyster *o, oyster *type)
{
    oyster_assign_type(o, type->in->symbol_id);
    /* table_entry *e = NEW(table_entry); */
    /* e->it = type; */
    /* incref(type); */
    /* table_put_entry(TYPE, e, o->in->info); */
    /* o->in->type = e; */
    /* incref(e); */
}

void oyster_assign_type(oyster *o, int type)
{
    o->in->type = type;
}
 
table *oyster_bindings(oyster * x)
{
    return x->bindings;
}

void oyster_assign_bindings(oyster * x, table *value)
{
    // fix memory
    x->bindings = value;
}

void *oyster_value(oyster * x)
{
    return x->in->value;
}

void oyster_assign_gc(oyster *o, int type)
{
    o->in->gc_type = type;
}

void oyster_assign_value(oyster * x, void *value)
{
    // fix memory
    x->in->value = value;
}

oyster * (*oyster_built_in(oyster *o)) (machine * m)
{
    return o->in->built_in;
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
        oyster_assign_type_o(ret, ret);
        incref(ret);
    }
    return ret;
}

oyster *make_symbol(int symbol_id)
{
    oyster *ret = make_untyped_oyster();
    oyster_assign_type_o(ret, symbol_type_symbol());
    ret->in->symbol_id = symbol_id;
    ret->in->gc_type = 0;
    return ret;
}

int symbol_id(oyster *sym){
    return sym->in->symbol_id;
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


//---------------------- Printing ------------------------//

void list_print(oyster * o)
{
    incref(o);
    oyster *t;
    for(; !nilp(o); t = cheap_cdr(o), decref(o), o = t, incref(o)){
        oyster *c = cheap_car(o);
        incref(c);
        if (oyster_type(c) == CONS){
            printf("(");
            oyster_print(c);
            printf(")");
        } else {
            oyster_print(c);
        }
        if (oyster_type(cheap_cdr(o)) == CONS) //unacceptable!
            printf(" ");

        decref(c);
    }
    decref(o);
}

void oyster_print(oyster * o)
{
    incref(o);
    int type = oyster_type(o);
    switch (type) {
    case CONS:
        list_print(o);
        break;
    case SYMBOL:
        printf("%s", string_from_sym_id(o->in->symbol_id));
        break;
    case NIL:
        printf("nil");
        break;
    case BUILT_IN_FUNCTION:
        printf("[builtin]");
        break;
    default:

        // Eventually this will, instead, test the info table for a print
        // function, or the table of the type-symbol, or sommat. Dunno yet. 
        if(type == sym_id_from_string("table")){
            table_print(o->in->value);

        } else if(type == sym_id_from_string("number")){
            printf("%lf", number_of(o));

            // } else if(type == sym_id_from_string("character")){
            //printf("%c", (char)o->in->value);

            //        } else if(type == sym_id_from_string("c-string")){
            //printf("%s", string_of(o));

        } else {
            printf("?(%d)", oyster_type(o));
        }
    }
    decref(o);
}


#endif

