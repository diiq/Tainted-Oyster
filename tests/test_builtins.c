#include "testing.h"
#include "stdio.h"
#include "oyster.h"
#include "builtins.c"



_test(builtin_cons){
    oyster *ret = evaluate_string("(cons (clear shi) (clear it))");
    assert(ret->in->type == CONS);
    oyster *c = car(ret);
    assert(c->in->symbol_id == sym_id_from_string("shi"));
    oyster *d = cdr(ret);
    assert(d->in->symbol_id == sym_id_from_string("it"));

    decref(ret);
    decref(c);
    decref(d);
}_tset;

_test(builtin_car){
    oyster *ret = evaluate_string("(car (cons (clear shi) (clear it)))");
    assert(ret->in->type == SYMBOL);
    assert(ret->in->symbol_id == sym_id_from_string("shi"));
    decref(ret);
}_tset;

_test(builtin_cdr){
    oyster *ret = evaluate_string("(cdr (cons (clear shi) (clear it)))");
    assert(ret->in->type == SYMBOL);
    assert(ret->in->symbol_id == sym_id_from_string("it"));
    decref(ret);
}_tset;


_test(builtin_current_scope){
    oyster *ret = evaluate_string("(current-scope)");
    assert(ret->in->type == sym_id_from_string("table"));
    int i;
    table_get(CONS, ret->in->value, &i);
    assert(i);
    printf("%d, ", ((table *)(ret->in->value))->ref);
    decref(ret);
}_tset;

_test(builtins){
    printf("\nTesting builtins:\n");
    run_test(builtin_cons);
    run_test(builtin_car);
    run_test(builtin_cdr);
    run_test(builtin_current_scope);
}_tset;
