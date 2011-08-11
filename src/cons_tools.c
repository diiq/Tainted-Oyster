#ifndef CONSTOOLS
#define CONSTOOLS

#include "oyster.h"
#include <stdlib.h>
//-------------------------- Cons, car, and cdr --------------------------//


struct cons_cell {
    void (*decref) (cons_cell * x);
    int ref;

    oyster *car;
    oyster *cdr;
};


oyster *make_cons(oyster * car, oyster * cdr)
{
    oyster *ret = make_oyster(CONS);
    ret->in->cons = NEW(cons_cell);
    incref(ret->in->cons);

    incref(car);
    ret->in->cons->car = car;

    incref(cdr);
    ret->in->cons->cdr = cdr;

    return ret;
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

void set_car(oyster *cons, oyster *value)
{
    oyster * t = cons->in->cons->car;
    cons->in->cons->car = value;
    incref(value);
    decref(t);
}

void set_cdr(oyster *cons, oyster *value)
{
    oyster * t = cons->in->cons->cdr;
    cons->in->cons->cdr = value;
    incref(value);
    decref(t);
}


oyster *cons(oyster * car, oyster * cdr)
{
    // There are many optimizations to be made here; but clarity, clarity.
    incref(car);
    incref(cdr);
    oyster *ret;
    if (nilp(car)) {
        oyster *new_cdr = oyster_copy(cdr, make_table());
        ret = make_cons(nil(), new_cdr);

        decref(ret->bindings);
        ret->bindings = cdr->bindings;
        incref(ret->bindings);

    } else if (nilp(cdr)) {
        oyster *new_car = oyster_copy(car, make_table());
        ret = make_cons(new_car, nil());

        decref(ret->bindings);
        ret->bindings = car->bindings;
        incref(ret->bindings);

    } else if (car->bindings == cdr->bindings) {
        oyster *new_car = oyster_copy(car, make_table());
        oyster *new_cdr = oyster_copy(cdr, make_table());

        ret = make_cons(new_car, new_cdr);

        decref(ret->bindings);
        ret->bindings = car->bindings;
        incref(ret->bindings);
    } else {
        // I do believe that this copy-on-write buisness works;
        // but not copying car and cdr is a culprit in future snafu
        ret = make_cons(car, cdr);
    }

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
        oyster *c = cheap_car(cons);
        if (c->bindings && !table_empty(c->bindings)) {
            return oyster_copy(c, c->bindings);
        } else {
            ret = oyster_copy(c, cons->bindings);
        }
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
        oyster *c = cheap_cdr(cons);
        if (c->bindings && !table_empty(c->bindings)) {
            return oyster_copy(c, c->bindings);
        } else {
            ret = oyster_copy(c, cons->bindings);
        }
    }
    decref(cons);
    return ret;
}

//------------------------- Convenience Functions --------------------------//

oyster *nil()
{
    oyster *ret = make_oyster(NIL);
    ret->in->gc_type = 0;
    return ret;
}

int nilp(oyster * x)
{
    return oyster_type(x) == NIL;
}

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
        ret = oyster_copy(b, b->bindings);
    }

    decref(a);
    decref(b);

    return ret;
}

oyster *reverse(oyster * xs)
{
    oyster *ret = nil();
    oyster *cur, *a;
    for (cur = xs, incref(cur); 
         !nilp(cur); 
         a = cur, cur = cdr(cur), incref(cur), decref(a)) {  // ew. gross.
        ret = cons(car(cur), ret);
    }
    decref(cur);
    return ret;
}

int oyster_length(oyster *xs)
{
    int i;
    for(i=0; !nilp(xs); i++){
        xs = cdr(xs);
    }
    return i;
}

oyster *ensure_list(oyster *xs)
{
    if(oyster_type(xs) == CONS || nilp(xs))
        return xs;
    return list(1, xs);
}



//------------------------------- Memory --------------------------//

void cons_cell_free(cons_cell * x)
{
    decref(x->car);
    decref(x->cdr);
    free(x);
}


#endif
