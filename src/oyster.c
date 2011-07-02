#ifndef OYSTER
#define OYSTER

#include "stdarg.h"

#include "oyster.h"
#include "parsing.h"

#include "stdio.h"

oyster *symbol_symbol;

void init_oyster()
{
    if(!symbol_symbol){
        symbol_symbol = make_untyped_oyster();
        symbol_symbol->in->type = SYMBOL;
        symbol_symbol->in->symbol_id = SYMBOL;
        table_put(TYPE, symbol_symbol, symbol_symbol->in->info);

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

void clean_up_oyster()
{
    free_symbol_table();
}

oyster *nil(){
    return make_oyster(NIL);
}


oyster *make_untyped_oyster()
{
    oyster *ret = NEW(oyster);
    ret->in = NEW(inner);
    ret->in->ref = 1;

    ret->in->type = -1;

    ret->in->info = make_table();
    table_ref(ret->in->info);
    
    ret->bindings = make_table();
    table_ref(ret->bindings);

    ret->ref = 0;
    return ret;
}

void inner_ref(inner *x){
    x->ref++;
}

void inner_free(inner *x){
    if(x->type == CONS){
        cons_unref(x->cons);
    }
    table_unref(x->info);
    free(x);
}

void inner_unref(inner *x){
    if(x){
        x->ref--;
        if(x->ref <= 0)
            inner_free(x);
    }
}

oyster *make_oyster(int type)
{
    oyster *ret = make_untyped_oyster();
    ret->in->type = type;
    table_put(TYPE, make_symbol(type), ret->in->info);
    return ret;
}

void oyster_ref(oyster *x){
    if(x) x->ref++;
}

void oyster_free(oyster *x){
     table_unref(x->bindings);
    inner_unref(x->in);
    free(x);
}

void oyster_unref(oyster *x){
    if(x){
        x->ref--;
        if (x->ref <= 0){
            oyster_free(x);
        }
    }
}

oyster *oyster_copy(oyster *x, table *new_bindings)
{
    oyster_ref(x);
    oyster *ret = NEW(oyster);
    ret->ref = 0;
    
    inner_ref(x->in);
    ret->in = x->in;

    ret->bindings = new_bindings;
    table_ref(ret->bindings);

    oyster_unref(x);
    return ret;
}

oyster *make_symbol(int symbol_id)
{
    oyster *ret = make_untyped_oyster();
    ret->in->type = SYMBOL;
    oyster_ref(symbol_symbol);
    table_put(TYPE, symbol_symbol, ret->in->info);
    ret->in->symbol_id = symbol_id;
    return ret;    
}

oyster *make_cons(oyster *car, oyster *cdr)
{
    oyster *ret = make_oyster(CONS);
    ret->in->cons = NEW(cons_cell);
    ret->in->cons->ref = 1;

    oyster_ref(car);
    ret->in->cons->car = car;

    oyster_ref(cdr);
    ret->in->cons->cdr = cdr;

    return ret;
}

void cons_ref(cons_cell *x){
    x->ref++;
}

void cons_unref(cons_cell *x){
    x->ref--;
    if (x->ref == 0){
        oyster_unref(x->car);
        oyster_unref(x->cdr);
        free(x);
    }
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
    oyster_ref(cons);
    if (nilp(cons)) return cons;
    oyster *ret = cons->in->cons->car;
    oyster_unref(cons);
    return ret;
}

oyster *cheap_cdr(oyster *cons)
{    
    oyster_ref(cons);
    if (nilp(cons)) return cons;
    oyster *ret = cons->in->cons->cdr;
    oyster_unref(cons);
    return ret;
}

oyster *cons(oyster *car, oyster *cdr)
{
    oyster_ref(car);
    oyster_ref(cdr);

    oyster *new_car = oyster_copy(car, make_table());
    oyster *new_cdr = oyster_copy(cdr, make_table());

    oyster *ret = make_cons(new_car, new_cdr);

    table_unref(ret->bindings);
    ret->bindings = binding_combine(car->bindings, cdr->bindings,
                                          new_car->bindings, new_cdr->bindings);
    table_ref(ret->bindings);

    oyster_unref(car);
    oyster_unref(cdr);
    return ret;
}

oyster *car(oyster *cons)
{
    oyster_ref(cons);
    oyster *ret;

    if (nilp(cons)){
        ret = nil();
    } else {
        ret = oyster_copy(cheap_car(cons), 
                          binding_union(cons->bindings, 
                                        cheap_car(cons)->bindings));
    }
    oyster_unref(cons);
    return ret;
}

oyster *cdr(oyster *cons)
{
    oyster_ref(cons);
    oyster *ret;

    if (nilp(cons)){
        ret = nil();
    } else {
        ret = oyster_copy(cheap_cdr(cons), 
                          binding_union(cons->bindings, 
                                        cheap_cdr(cons)->bindings));
    }
    oyster_unref(cons);
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

    oyster *ret = nil();

    for(i=count-1; i>=0; i--){
        ret = make_cons(els[i], ret);
    }

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

    oyster *ret = nil();

    for(i=count-1; i>=0; i--){
        ret = cons(els[i], ret);
    }
    free(els);
    return ret;
}

oyster *append(oyster *a, oyster *b)
{
    // Reduce this to a loop.
    oyster_ref(a);
    oyster_ref(b);
    oyster *ret;

    if(!nilp(a)){
        ret = cons(car(a), append(cdr(a), b));
    } else {
        ret = b; // Do I want to copy b? Relevant?
    }

    oyster_unref(a);
    oyster_unref(b);

    return ret;
}

oyster *reverse(oyster *xs)
{
    oyster_ref(xs);
    oyster *ret = nil();
    while (!nilp(xs)){
        oyster_ref(xs);
        ret = cons(car(xs), ret);
        oyster *xys = cdr(xs);
        oyster_unref(xs);
        xs = xys;
    }
    oyster_unref(xs);
    return ret;
}

//------------------------------ printing ---------------------------//

void list_print(oyster *o){
    oyster_ref(o);
    oyster_print(cheap_car(o));
    oyster *d = cheap_cdr(o);
    if (d->in->type == CONS){
        printf(" ");
        list_print(d);
    } else if (d->in->type == NIL){
        printf(")");
    } else {
        printf(" . ");
        oyster_print(d);
        oyster_unref(d);
        printf(")");
    }
    oyster_unref(o);
}

void oyster_print(oyster *o){
    oyster_ref(o);
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
    oyster_unref(o);
}


#endif
