#include "oyster.h"
#include "parsing.c"
#include "testing.h"
#include "stdio.h"

extern int tests_run;

_test(next_oyster){
    char *str = "and (... never (@ and never) to be) my love";

    GScanner *a = string_scanner(str);

    oyster *ret = next_oyster(a);
    assert(ret->in->type == SYMBOL, "wrong type");
    assert(ret->in->symbol_id == sym_id_from_string("and"), 
           "wrong string %d %d", sym_id_from_string("and"), ret->in->symbol_id);

    oyster_unref(ret);
    ret = next_oyster(a);

    assert(ret->in->type == CONS, "wrong type");

    oyster_unref(ret);
    g_scanner_destroy(a);
}_tset;

_test(parsing){
    printf("\nTesting parsing:\n");
    run_test(next_oyster);
}_tset;

