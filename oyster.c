#include "gc.h"
#include "oyster.h"
#include "table.c"

#ifndef _TEST
int main() {
    return UNTO_DUST;
}
#endif

oyster *symbol_symbol;

void init_oyster()
{
    symbol_symbol = make_untyped_oyster();
    symbol_symbol->in->type = SYMBOL;
    symbol_symbol->in->symbol_id = SYMBOL;
    table_put(TYPE, symbol_symbol, symbol_symbol->in->info);
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
    ret->in->cons = NEW(cons);
    ret->in->cons->car = car;
    ret->in->cons->cdr = cdr;
    return ret;
}

int nilp(oyster *x){
    return x->in->type == NIL;
}
