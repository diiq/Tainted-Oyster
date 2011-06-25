#ifndef OYSTER
#define OYSTER

#include "gc.h"
#include "stdarg.h"

#include "oyster.h"
#include "table.c"
#include "bindings.c"



#ifndef _TEST
int main() {
    return UNTO_DUST;
}
#endif

oyster *symbol_symbol;
oyster *nil;

void init_oyster()
{
    symbol_symbol = make_untyped_oyster();
    symbol_symbol->in->type = SYMBOL;
    symbol_symbol->in->symbol_id = SYMBOL;
    table_put(TYPE, symbol_symbol, symbol_symbol->in->info);

    nil = make_oyster(NIL);
}

oyster *make_untyped_oyster()
{
    oyster *ret = NEW(oyster);
    ret->in = NEW(inner);
    ret->in->info = make_table();
    ret->bindings = make_table();
    return ret;
}

oyster *make_oyster(int type)
{
    oyster *ret = make_untyped_oyster();
    ret->in->type = type;
    table_put(TYPE, make_symbol(type), ret->in->info);
    return ret;
}

oyster *oyster_copy(oyster *x)
{
    oyster *ret = NEW(oyster);
    ret->in = NEW(inner);
    *(ret->in) = *(x->in);
    ret->bindings = x->bindings;
    return ret;
}

oyster *make_symbol(int symbol_id)
{
    oyster *ret = make_untyped_oyster();
    ret->in->type = SYMBOL;
    table_put(TYPE, symbol_symbol, ret->in->info);
    ret->in->symbol_id = symbol_id;
    return ret;    
}

oyster *make_cons(oyster *car, oyster *cdr)
{
    oyster *ret = make_oyster(CONS);
    ret->in->cons = NEW(cons_cell);
    ret->in->cons->car = car;
    ret->in->cons->cdr = cdr;
    return ret;
}

int nilp(oyster *x){
    return x->in->type == NIL;
}

//-------------------------- Cons, car, and cdr --------------------------//

oyster *cheap_car(oyster *cons)
{
    if (nilp(cons)) return cons;
    return cons->in->cons->car;
}

oyster *cheap_cdr(oyster *cons)
{
    if (nilp(cons)) return cons;
    return cons->in->cons->car;
}

oyster *cons(oyster *car, oyster *cdr)
{
    oyster *new_car = oyster_copy(car);
    oyster *new_cdr = oyster_copy(cdr);

    if (nilp(car)){
        oyster *ret = make_cons(nil, new_cdr);
        ret->bindings = new_cdr->bindings;
        new_cdr->bindings = make_table();
        return ret;
    }

    if (nilp(cdr)){
        oyster *ret = make_cons(new_car, nil);
        ret->bindings = new_car->bindings;
        new_car->bindings = make_table();
        return ret;
    }
    
    new_car->bindings = make_table();
    new_cdr->bindings = make_table();
    oyster *ret = make_cons(new_car, new_cdr);
    ret->bindings = binding_combine(car->bindings, cdr->bindings,
                                    new_car->bindings, new_cdr->bindings);
    return ret;
}

oyster *car(oyster *cons){
    if (nilp(cons))
        return nil;

    if (cons->in->type != CONS){
        // ERROR!
        return NULL;
    }

    oyster *ret = oyster_copy(cheap_car(cons));
	ret->bindings = binding_union(cons->bindings, ret->bindings);
	return ret;
}


oyster *cdr(oyster *cons){
    if (nilp(cons))
        return nil;

    if (cons->in->type != CONS){
        // ERROR!
        return NULL;
    }

    oyster *ret = oyster_copy(cheap_cdr(cons));
	ret->bindings = binding_union(cons->bindings, ret->bindings);
	return ret;
}

//------------------------- Convenience Functions --------------------------//

oyster *cheap_list(int count, ...){
    int i;
    oyster **els = GC_MALLOC(sizeof(oyster*)*count);
    va_list xs;
    va_start(xs, count);
    for(i=0; i<count; i++)
        els[i] = va_arg(xs, oyster*);
    va_end(xs);
    oyster *ret = nil;
    for(i=count-1; i>=0; i--)
        ret = make_cons(els[i], ret);
    return ret;
}

oyster *list(int count, ...){
    int i;
    oyster **els = GC_MALLOC(sizeof(oyster*)*count);
    va_list xs;
    va_start(xs, count);
    for(i=0; i<count; i++)
        els[i] = va_arg(xs, oyster*);
    va_end(xs);
    oyster *ret = nil;
    for(i=count-1; i>=0; i--)
        ret = cons(els[i], ret);
    return ret;
}

oyster *cheap_append(oyster *a, oyster *b){
    // stack heavy for now.
    if(!nilp(a))
        return make_cons(cheap_car(a), cheap_append(cheap_cdr(a), b));
    return b;
}

oyster *append(oyster *a, oyster *b){
    // stack heavy for now.
    if(!nilp(a))
        return cons(car(a), append(cdr(a), b));
    return b; // Do I want to copy b? Relevant?
}


#endif
