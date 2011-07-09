#ifndef OYSTER
#define OYSTER

// This file contains functions that create and manipulate the simplest units:
// oysters.

// An oyster is a scope wrapped around an object, like a blanket around a
// cold, wet, child.

#include "stdarg.h"
#include "stdio.h"

#include "oyster.h"
#include "parsing.h"



void init_oyster()              // Where does this belong? Quo vadis, init?
{
    static int a = 0;
    if (!a) {

        init_symbol_table();

        add_symbol(TYPE, "type");
        add_symbol(SYMBOL, "symbol");
        add_symbol(CONS, "cons");
        add_symbol(NIL, "()");
        add_symbol(LEAKED, "LEAKED");
        add_symbol(ATPEND, "@");
        add_symbol(ELIPSIS, "...");
        add_symbol(ASTERIX, "*");
        add_symbol(COMMA, ",");
        add_symbol(CLEAR, "clear");
        add_symbol(BUILT_IN_FUNCTION, "built-in-function");

        a = 1;
    }
}

void clean_up_oyster()          // And can clean_up come with you?
{
    free_symbol_table();
}

oyster *nil()
{
    return make_oyster(NIL);
}



oyster *make_untyped_oyster()
{
    oyster *ret = NEW(oyster);
    ret->in = NEW(inner);
    ret->in->ref = 1;
    ret->in->incref = &inner_ref;
    ret->in->decref = &inner_unref;

    ret->in->type = -1;        

    //    ret->in->info = make_table(); 
    //    incref(ret->in->info);

    ret->in->value = NULL;

    ret->bindings = make_table();
    incref(ret->bindings);

    ret->ref = 0;
    ret->incref = &oyster_ref;
    ret->decref = &oyster_unref;
    return ret;
}

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



oyster *make_oyster(int type)
{
    oyster *ret = make_untyped_oyster();
    ret->in->type = type;
    return ret;
}

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
    ret->ref = 0;
    ret->incref = &oyster_ref;
    ret->decref = &oyster_unref;

    incref(x->in);
    ret->in = x->in;

    ret->bindings = new_bindings;
    incref(ret->bindings);

    decref(x);
    return ret;
}

oyster *make_cons(oyster * car, oyster * cdr)
{
    oyster *ret = make_oyster(CONS);
    ret->in->cons = NEW(cons_cell);
    ret->in->cons->ref = 1;
    ret->in->cons->incref = &cons_ref;
    ret->in->cons->decref = &cons_unref;

    incref(car);
    ret->in->cons->car = car;

    incref(cdr);
    ret->in->cons->cdr = cdr;

    return ret;
}

void cons_ref(cons_cell * x)
{
    x->ref++;
}

void cons_unref(cons_cell * x)
{
    x->ref--;
    if (x->ref == 0) {
        decref(x->car);
        decref(x->cdr);
        free(x);
    }
}



//-------------------------- Cons, car, and cdr --------------------------//

int nilp(oyster * x)
{
    return x->in->type == NIL;
}

oyster *cheap_car(oyster * cons)
{
    incref(cons);
    if (nilp(cons))
        return cons;
    oyster *ret = cons->in->cons->car;
    decref(cons);
    return ret;
}

oyster *cheap_cdr(oyster * cons)
{
    incref(cons);
    if (nilp(cons))
        return cons;
    oyster *ret = cons->in->cons->cdr;
    decref(cons);
    return ret;
}

oyster *cons(oyster * car, oyster * cdr)
{
    // There are many optimizations to be made here; but clarity, clarity.
    incref(car);
    incref(cdr);

    oyster *new_car = oyster_copy(car, make_table());
    oyster *new_cdr = oyster_copy(cdr, make_table());

    oyster *ret = make_cons(new_car, new_cdr);

    decref(ret->bindings);
    ret->bindings = binding_combine(car->bindings, cdr->bindings,
                                    new_car->bindings, new_cdr->bindings);
    incref(ret->bindings);

    decref(car);
    decref(cdr);
    return ret;
}

oyster *car(oyster * cons)
{
    incref(cons);
    oyster *ret;

    if (nilp(cons)) {
        ret = nil();
    } else {
        ret = oyster_copy(cheap_car(cons),
                          binding_union(cons->bindings,
                                        cheap_car(cons)->bindings));
    }
    decref(cons);
    return ret;
}

oyster *cdr(oyster * cons)
{
    incref(cons);
    oyster *ret;

    if (nilp(cons)) {
        ret = nil();
    } else {
        ret = oyster_copy(cheap_cdr(cons),
                          binding_union(cons->bindings,
                                        cheap_cdr(cons)->bindings));
    }
    decref(cons);
    return ret;
}

//------------------------- Convenience Functions --------------------------//

oyster *list(int count, ...)
{
    int i;
    oyster **els = malloc(sizeof(oyster *) * count);
    va_list xs;
    va_start(xs, count);
    for (i = 0; i < count; i++)
        els[i] = va_arg(xs, oyster *);
    va_end(xs);

    oyster *ret = nil();

    for (i = count - 1; i >= 0; i--) {
        ret = cons(els[i], ret);
    }
    free(els);
    return ret;
}

oyster *append(oyster * a, oyster * b)
{
    // Reduce this to a loop.
    incref(a);
    incref(b);
    oyster *ret;

    if (!nilp(a)) {
        ret = cons(car(a), append(cdr(a), b));
    } else {
        ret = b;
    }

    decref(a);
    decref(b);

    return ret;
}

oyster *reverse(oyster * xs)
{
    incref(xs);
    oyster *ret = nil();
    oyster *cur, *a;
    for (cur = xs, incref(cur); !nilp(cur); a = cur, cur = cdr(cur), incref(cur), decref(a)) {  // ew. gross.
        ret = cons(car(cur), ret);
    }
    decref(cur);
    return ret;
}

//------------------------------ printing ---------------------------//

void list_print(oyster * o)
{
    incref(o);
    oyster_print(cheap_car(o));
    oyster *d = cheap_cdr(o);
    if (d->in->type == CONS) {
        printf(" ");
        list_print(d);
    } else if (d->in->type == NIL) {
        printf(")");
    } else {
        printf(" . ");
        oyster_print(d);
        printf(")");
    }
    decref(o);
}

void oyster_print(oyster * o)
{
    incref(o);
    int type = o->in->type;
    switch (type) {
    case CONS:
        printf("(");
        list_print(o);
        break;
    case SYMBOL:
        printf("%s", string_from_sym_id(o->in->symbol_id));
        break;
    case NIL:
        printf("()");
        break;
    case BUILT_IN_FUNCTION:
        printf("[builtin]");
        break;
    default:
        printf("?(%d)", o->in->type);
        break;
    }
    decref(o);
}


#endif
