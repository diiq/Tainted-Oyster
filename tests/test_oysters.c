#include "oyster.h"
#include "testing.h"
#include "stdio.h"

extern int tests_run;
extern oyster *symbol_symbol;


_test(make_untyped_oyster)
{
    oyster *f = make_untyped_oyster();
    assert(f);
    decref(f);
} _tset;

_test(make_oyster)
{
    oyster *f = make_oyster(SYMBOL);
    assert(oyster_type(f) == SYMBOL, "wrong type");
    decref(f);

} _tset;

_test(make_symbol)
{
    oyster *f = make_symbol(5);
    assert(oyster_type(f) == SYMBOL);
    assert(symbol_id(f) == 5);

    decref(f);

} _tset;

_test(make_cons)
{
    oyster *c = make_cons(make_symbol(5), make_symbol(2));
    incref(c);
    assert(oyster_type(c) == CONS);
    assert(symbol_id(cheap_car(c)) == 5);
    assert(symbol_id(cheap_cdr(c)) == 2);
    decref(c);
} _tset;

_test(cons_car_cdr)
{
    oyster *c = cons(make_symbol(55), cons(make_symbol(22), nil()));
    incref(c);

    oyster *x = car(cdr(c));

    oyster *y = car(c);

    assert(oyster_type(y) == SYMBOL);
    assert(symbol_id(y) == 55);
    assert(symbol_id(x) == 22);

    decref(c);
    decref(x);
    decref(y);
} _tset;

_test(oyster)
{
    printf("\nTesting: oyster\n");
    run_test(make_untyped_oyster);
    run_test(make_oyster);
    run_test(make_symbol);
    run_test(make_cons);
    run_test(cons_car_cdr);
} _tset;
