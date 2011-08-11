#include "testing.h"
#include "stdio.h"
#include "oyster.h"



_test(builtin_cons)
{
    oyster *ret = evaluate_string("(cons (clear shi) (clear it))");
    assert(oyster_type(ret) == CONS);
    oyster *c = car(ret);
    assert(symbol_id(c) == sym_id_from_string("shi"));
    oyster *d = cdr(ret);
    assert(symbol_id(d) == sym_id_from_string("it"));

    decref(ret);
    decref(c);
    decref(d);
}

_tset;

_test(builtin_car)
{
    oyster *ret = evaluate_string("(car (cons (clear shi) (clear it)))");
    assert(oyster_type(ret) == SYMBOL);
    assert(symbol_id(ret) == sym_id_from_string("shi"));
    decref(ret);
}

_tset;

_test(builtin_cdr)
{
    oyster *ret = evaluate_string("(cdr (cons (clear shi) (clear it)))");
    assert(oyster_type(ret) == SYMBOL);
    assert(symbol_id(ret) == sym_id_from_string("it"));
    decref(ret);
}

_tset;


_test(builtin_set)
{
    oyster *ret = evaluate_string("(set a (clear b))\n"
                                  "a");
    assert(symbol_id(ret) == sym_id_from_string("b"));
    decref(ret);
}

_tset;

_test(builtin_current_scope)
{
    oyster *ret = evaluate_string("(current-scope)");
    assert(oyster_type(ret) == sym_id_from_string("table"));
    int i;
    table_get(CONS, oyster_value(ret), &i);
    assert(i);
    decref(ret);
} _tset;

_test(builtin_table_get)
{
    oyster *ret = evaluate_string("(table-get cons (current-scope))");
    oyster *m = car(car(ret));
    assert(symbol_id(m) == sym_id_from_string("car"));
    decref(m);
    decref(ret);
} _tset;

_test(builtins)
{
    printf("\nTesting builtins:\n");
    run_test(builtin_cons);
    run_test(builtin_car);
    run_test(builtin_cdr);
    run_test(builtin_set);
    run_test(builtin_current_scope);
    run_test(builtin_table_get);
} _tset;
