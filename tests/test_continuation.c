#include "oyster.h"

_test(make_continuation)
{
    // Probably should test this more thouroghly, but I AM A WILD MAN!
    // I sing to you the Ubermench, cutting values from the raw fabric.
    oyster *cont = make_continuation(make_machine());
    assert(oyster_type(cont) == CONS);
    decref(cont);
}

_tset;


_test(continuations)
{
    printf("\nTesting continuations:\n");
    run_test(make_continuation);
}

_tset;
