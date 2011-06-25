#include "stdio.h"

#include "testing.h"
#include "oyster.h"

#include "machine.h"
#include "machine.c"


_test(make_machine){
    machine *m = NULL;
    m = make_machine();
    assert(m);
}_tset;

_test(arg_list_tests){
    assert(asterix_p(cons(make_symbol(ASTERIX), nil)));
    assert(atpend_p(cons(make_symbol(ATPEND), nil)));
    assert(elipsis_p(make_symbol(ELIPSIS)));
}_tset;

_test(machine){
    printf("\nTesting machine:\n");
    run_test(make_machine);
    run_test(arg_list_tests);
}_tset;
