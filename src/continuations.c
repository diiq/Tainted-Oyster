#ifndef CONTINUATIONS
#define CONTINUATIONS

#include "oyster.h"

frame *frame_copy(frame *x, frame **base){
    incref(x);
    
    frame *ret;
    if(!x->below){
        *base = make_frame(x->scope, NULL);
        ret = *base;
    } else {
        ret = make_frame(x->scope, 
                         frame_copy(x->below, base));
    }
    ret->signal_handler = x->signal_handler;
    incref(x->signal_handler);

    decref(x);
    return ret;
}

machine *machine_copy(machine *m){
    machine *ret = make_machine();
    decref(ret->current_frame);
    decref(ret->base_frame);

    frame *base;
    ret->current_frame = frame_copy(m->current_frame,
                                    &base);
    incref(ret->current_frame);
    ret->base_frame = base;
    incref(ret->base_frame);

    return ret;
}

oyster *call_continuation(machine *m){
    ARG(continuation);
    ARG(value);
    incref(continuation);
    incref(value);

    decref(m->current_frame);
    m->current_frame = ((machine *)(continuation->in->value))->current_frame;
    decref(m->base_frame);
    m->base_frame = ((machine *)(continuation->in->value))->base_frame;

    decref(continuation);
    decref(value);

    return value;
}

oyster *make_continuation(machine *m){
    oyster *ret = list(2, list(1, arg("value")), 
                       make_builtin(call_continuation));
    oyster *cont = make_oyster(sym_id_from_string("continuation"));
    cont->in->value = machine_copy(m);
    incref(cont->in->value);
    table_put(sym_id_from_string("continuation"), 
              cont,
              ret->bindings);
    return ret;
}

#endif
