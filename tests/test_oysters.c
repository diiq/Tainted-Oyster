#include "oyster.c"
#include "testing.h"
#include "stdio.h"

extern int tests_run;
extern oyster *symbol_symbol;


_test(make_untyped_oyster){
    oyster *f = make_untyped_oyster();
    assert(f);
    oyster_unref(f);
}_tset;

_test(make_oyster){
    oyster *f = make_oyster(SYMBOL);
    assert(f->in->type == SYMBOL, "wrong type");
    int i = 0;
    oyster *re = table_get(TYPE, f->in->info, &i);
    assert(i, "not there");
    assert(re->in->type == SYMBOL, "wrong type");
    assert(re->in->symbol_id == SYMBOL, "wrong id");

    oyster_unref(f);

}_tset;

_test(make_symbol){
    oyster *f = make_symbol(5);
    assert(f->in->type == SYMBOL);
    assert(f->in->symbol_id == 5);
    int i = 0;
    oyster *re = table_get(TYPE, f->in->info, &i);
    assert(i);
    assert(re->in->type == SYMBOL);
    assert(re->in->symbol_id == SYMBOL);

    oyster_unref(f);

}_tset;

_test(make_cons){
    oyster *c = make_cons(make_symbol(5), make_symbol(2));
    assert(c->in->type == CONS);
    assert(c->in->cons->car->in->symbol_id == 5);
    assert(c->in->cons->cdr->in->symbol_id == 2);
    oyster_unref(c);
}_tset;

_test(cons_car_cdr){
    oyster *c = cons(make_symbol(55), cons(make_symbol(22), nil()));
    oyster_ref(c);
    
    oyster *x = car(cdr(c));

    oyster *y = car(c);

    assert(y->in->type == SYMBOL);
    assert(y->in->symbol_id == 55);
    assert(x->in->symbol_id == 22);

    oyster_unref(c);
    oyster_unref(x);
    oyster_unref(y);
}_tset;

_test(oyster){
    printf("\nTesting: oyster\n");
    run_test(make_untyped_oyster);
    run_test(make_oyster);
    run_test(make_symbol);
    run_test(make_cons);
    run_test(cons_car_cdr);
}_tset;



