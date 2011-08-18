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
    cons_cell *in = NEW(cons_cell);
    oyster_assign_value(ret, in);
    incref(in);

    in->car = car;
    incref(car);

    in->cdr = cdr;
    incref(cdr);

    return ret;
}

cons_cell *cons_of(oyster *o){
    return (cons_cell *)oyster_value(o);
}

oyster *cheap_car(oyster * cons)
{
    incref(cons);
    if (nilp(cons))
        return cons;
    oyster *ret = cons_of(cons)->car;
    decref(cons);
    return ret;
}

oyster *cheap_cdr(oyster * cons)
{
    incref(cons);
    if (nilp(cons))
        return cons;
    oyster *ret = cons_of(cons)->cdr;
    decref(cons);
    return ret;
}

void set_car(oyster *cons, oyster *value)
{
    oyster * t = cons_of(cons)->car;
    cons_of(cons)->car = value;
    incref(value);
    decref(t);
}

void set_cdr(oyster *cons, oyster *value)
{
    oyster * t = cons_of(cons)->cdr;
    cons_of(cons)->cdr = value;
    incref(value);
    decref(t);
}


oyster *cons(oyster * car, oyster * cdr)
{
    // There are many optimizations to be made here; but clarity, clarity.
    incref(car);
    incref(cdr);

    oyster *new_car = oyster_copy(car, NULL);
    oyster *new_cdr = oyster_copy(cdr, NULL);
    oyster *ret = make_cons(new_car, new_cdr);
    
    if (!oyster_bindings(car) || table_empty(oyster_bindings(car))) {

        decref(oyster_bindings(ret));
        oyster_assign_bindings(ret, oyster_bindings(cdr));
        incref(oyster_bindings(ret));

    } else if (!oyster_bindings(cdr) || table_empty(oyster_bindings(cdr))) {

        decref(oyster_bindings(ret));
        oyster_assign_bindings(ret, oyster_bindings(car));
        incref(oyster_bindings(ret));

    } else if (oyster_bindings(car) == oyster_bindings(cdr)) {

        decref(oyster_bindings(ret));
        oyster_assign_bindings(ret, oyster_bindings(car));
        incref(oyster_bindings(ret));

    } else {
        oyster_assign_bindings(new_car, oyster_bindings(car));
        incref(oyster_bindings(new_car));
        oyster_assign_bindings(new_cdr, oyster_bindings(cdr));
        incref(oyster_bindings(new_cdr));
    }

    decref(car);
    decref(cdr);
    return ret;
}

oyster *car(oyster * cons)
{
    oyster *ret;
    incref(cons);

    if (nilp(cons)) {
        ret = oyster_copy(cons, oyster_bindings(cons));
    }


    oyster *c = cheap_car(cons);
    if (oyster_bindings(c) && !table_empty(oyster_bindings(c))) {
        ret = oyster_copy(c, oyster_bindings(c));
    } else {
        ret = oyster_copy(c, oyster_bindings(cons));
    }

    decref(cons);
    return ret;
}


oyster *cdr(oyster * cons)
{
    oyster *ret;
    incref(cons);

    if (nilp(cons)) {
        ret = oyster_copy(cons, oyster_bindings(cons));
    }

    oyster *c = cheap_cdr(cons);
    if (oyster_bindings(c) && !table_empty(oyster_bindings(c))) {
        ret = oyster_copy(c, oyster_bindings(c));
    } else {
        ret = oyster_copy(c, oyster_bindings(cons));
    }

    decref(cons);
    return ret;
}

//------------------------- Convenience Functions --------------------------//

oyster *nil()
{
    oyster *ret = make_oyster(NIL);
    oyster_assign_gc(ret, 0);
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
        ret = oyster_copy(b, oyster_bindings(b));
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
