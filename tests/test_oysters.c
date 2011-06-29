#include "oyster.c"
#include "testing.h"
#include "stdio.h"

extern int tests_run;
extern oyster *symbol_symbol;

_test(init_oyster){
    init_oyster();
    assert(symbol_symbol->in->type == SYMBOL);
}_tset;

_test(make_untyped_oyster){
    oyster *f = make_untyped_oyster();
    assert(f);
}_tset;

_test(make_oyster){
    oyster *f = make_oyster(SYMBOL);
    assert(f->in->type == SYMBOL, "wrong type");
    int i = 0;
    oyster *re = table_get(TYPE, f->in->info, &i);
    assert(i, "not there");
    assert(re->in->type == SYMBOL, "wrong type");
    assert(re->in->symbol_id == SYMBOL, "wrong id");
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
}_tset;

_test(make_cons){
    oyster *f = make_symbol(5);
    oyster *g = make_symbol(2);
    oyster *c = make_cons(f, g);
    assert(c->in->type == CONS);
    assert(c->in->cons->car->in->symbol_id == 5);
    assert(c->in->cons->cdr->in->symbol_id == 2);
}_tset;

_test(cons_car_cdr){
    oyster *f = make_symbol(5);
    oyster *g = make_symbol(2);
    oyster *c = cons(f, cons(g, nil));
    assert(car(c)->in->type == SYMBOL);
    assert(car(c)->in->symbol_id == 5);
    assert(car(cdr(c))->in->symbol_id == 2);
}_tset;

_test(oyster){
    printf("\nTesting: oyster\n");
    run_test(init_oyster);
    run_test(make_untyped_oyster);
    run_test(make_oyster);
    run_test(make_symbol);
    run_test(make_cons);
    run_test(cons_car_cdr);
}_tset;



