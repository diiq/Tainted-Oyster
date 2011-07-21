#include "oyster.h"
#include "continuations.c"

_test(machine_copy)
{
    GScanner *in = string_scanner("(cons (' a) (' b))");

    oyster *func = next_oyster(in);
    machine *ma = make_machine();
    machine *m = machine_copy(ma);
    incref(m);

    while (func) {
        push_new_instruction(m, func, EVALUATE);

        while (!m->paused) {
            step_machine(m);
        }
        m->paused = 0;

        func = next_oyster(in);
    }

    g_scanner_destroy(in);

    assert(m->accumulator->in->type == CONS);
    decref(ma);
    decref(m);
}

_tset;

_test(make_continuation)
{
    // Probably should test this more thouroghly, but I AM A WILD MAN!
    // I sing to you the Ubermench, cutting values from the raw fabric.
    oyster *cont = make_continuation(make_machine());
    assert(cont->in->type == CONS);
    decref(cont);
}

_tset;


_test(continuations)
{
    printf("\nTesting continuations:\n");
    run_test(machine_copy);
    run_test(make_continuation);
}

_tset;
