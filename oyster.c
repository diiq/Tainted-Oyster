#ifndef OYSTER
#define OYSTER

#include "stdarg.h"

#include "oyster.h"
#include "parsing.h"

#include "stdio.h"

oyster *symbol_symbol;
oyster *nil;
extern struct symbol_table *symbol_table;

void init_oyster()
{
    symbol_symbol = make_untyped_oyster();
    symbol_symbol->in->type = SYMBOL;
    symbol_symbol->in->symbol_id = SYMBOL;
    table_put(TYPE, symbol_symbol, symbol_symbol->in->info);

    nil = make_oyster(NIL);

    if(!symbol_table){
    init_symbol_table();

    add_symbol(TYPE, "type");
    add_symbol(SYMBOL, "symbol");
    add_symbol(CONS, "cons");
    add_symbol(NIL, "nil");
    add_symbol(LEAKED, "leaked");
    add_symbol(ATPEND, "@");
    add_symbol(ELIPSIS, "...");
    add_symbol(ASTERIX, "*");
    add_symbol(COMMA, ",");
    add_symbol(CLEAR, "clear");
    add_symbol(BUILT_IN_FUNCTION, "built-in-function");
    }
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

int oyster_type(oyster *x)
{
    return x->in->type;
}

int nilp(oyster *x)
{
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
    return cons->in->cons->cdr;
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

oyster *car(oyster *cons)
{
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


oyster *cdr(oyster *cons)
{
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

// I don't know if I've tested these?

oyster *cheap_list(int count, ...)
{
    int i;
    oyster **els = malloc(sizeof(oyster*)*count);
    va_list xs;
    va_start(xs, count);
    for(i=0; i<count; i++)
        els[i] = va_arg(xs, oyster*);
    va_end(xs);
    oyster *ret = nil;
    for(i=count-1; i>=0; i--)
        ret = make_cons(els[i], ret);
    free(els);
    return ret;
}

oyster *list(int count, ...)
{
    int i;
    oyster **els = malloc(sizeof(oyster*)*count);
    va_list xs;
    va_start(xs, count);
    for(i=0; i<count; i++)
        els[i] = va_arg(xs, oyster*);
    va_end(xs);
    oyster *ret = nil;
    for(i=count-1; i>=0; i--)
        ret = cons(els[i], ret);
    free(els);
    return ret;
}

oyster *cheap_append(oyster *a, oyster *b)
{
    // stack heavy for now. Reduce to loop later.
    if(!nilp(a))
        return make_cons(cheap_car(a), cheap_append(cheap_cdr(a), b));
    return b;
}

oyster *append(oyster *a, oyster *b)
{
    // Reduce this to a loop.
    if(!nilp(a))
        return cons(car(a), append(cdr(a), b));
    return b; // Do I want to copy b? Relevant?
}

oyster *reverse(oyster *xs)
{
    oyster *ret = nil;
    while (!nilp(xs)){
        ret = cons(car(xs), ret);
        xs = cdr(xs);
    }
    return ret;
}

//------------------------------ printing ---------------------------//

void list_print(oyster *o){
    oyster_print(cheap_car(o));
    if (cheap_cdr(o)->in->type == CONS){
        printf(" ");
        list_print(cheap_cdr(o));
    } else if (cheap_cdr(o)->in->type == NIL){
        printf(")");
    } else {
        printf(" . ");
        oyster_print(cheap_cdr(o));
        printf(")");
    }
}
void oyster_print(oyster *o){
    int type = o->in->type;
    switch(type) {
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
    }
}


#endif
