#ifndef CONTINUATIONS
#define CONTINUATIONS

#include "oyster.h"
#include "machine.h"


oyster *call_continuation(machine * m)
{
    ARG(continuation);
    ARG(value);
    incref(continuation);
    incref(value);

    decref(m->current_frame);
    m->current_frame =
        ((machine *) (oyster_value(continuation)))->current_frame;
    decref(m->base_frame);
    m->base_frame = ((machine *) (oyster_value(continuation)))->base_frame;

    decref(continuation);
    decref(value);

    return value;
}

oyster *make_continuation(machine * m)
{
    incref(m);
    oyster *ret = list(2, list(1, arg("value")),
                       make_builtin(call_continuation));
    oyster *cont = make_oyster(sym_id_from_string("continuation"));
    oyster_assign_value(cont, machine_copy(m));
    incref(oyster_value(cont));
    oyster_add_to_bindings(sym_id_from_string("continuation"), cont, ret);
    decref(m);
    return ret;
}

#endif
