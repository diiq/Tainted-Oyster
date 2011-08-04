// This file concerns itself with the process of the interpreter itself.

#include "oyster.h"
#include "machine.h"
#include "frame.h"
#include <stdlib.h>

machine *make_machine()
{
    machine *ret = NEW(machine);
    ret->base_frame = make_frame(NULL,
                                 make_table(),
                                 make_table(), make_table(), NULL, PAUSE);
    incref(ret->base_frame);

    ret->current_frame = ret->base_frame;
    incref(ret->current_frame);

    ret->now = ret->base_frame;
    incref(ret->now);

    ret->accumulator = NULL;
    ret->paused = 0;

    add_builtins(ret);
    add_builtin_numbers(ret);

    return ret;
}

oyster *machine_accumulator(machine *m){
    return m->accumulator;
}

int machine_paused(machine *m){
    return m->paused;
}

void machine_unpause(machine *m){
    m->paused = 0;
}

frame *machine_active_frame(machine *m){
    return m->now;
}

frame *machine_pop_stack(machine * m)
{
    if (m->current_frame->flag != PAUSE) {
        decref(m->now);
        m->now = m->current_frame;

        m->current_frame = m->current_frame->below;
        incref(m->current_frame);

        return m->now;

    } else {

        decref(m->now);
        m->now = m->current_frame;
        incref(m->now);

        m->paused = 1;
        return NULL;
    }
}

machine *machine_copy(machine * m)
{
    machine *ret = make_machine();
    decref(ret->current_frame);
    decref(ret->base_frame);
    decref(ret->now);

    ret->current_frame = m->current_frame;
    incref(ret->current_frame);

    ret->base_frame = m->base_frame;
    incref(ret->base_frame);

    ret->now = m->now;
    incref(ret->now);

    return ret;
}

void set_accumulator(machine * m, oyster * value)
{
    oyster *t = m->accumulator;
    m->accumulator = value;
    incref(m->accumulator);
    decref(t);
}

//-------------------------- Printing ----------------------------//

void machine_print(machine * m)
{
    frame *f = m->current_frame;
    printf("Now: ");
    frame_print(m->now, 0);
    if (m->now->instruction && !table_empty(m->now->instruction->bindings)) {
        printf(" with the bindings: \n");
        table_print(m->now->instruction->bindings);
    }
    while (f) {
        printf("frame: ");
        frame_print(f, 0);
        f = f->below;
    }
    if (m->accumulator) {
        printf("accum: ");
        oyster_print(m->accumulator);
    }
    printf
        ("\n--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---\n\n\n\n");
}

//------------------------- Memory -------------------------------//

void machine_ref(machine * x)
{
    x->ref++;
}

void machine_unref(machine * x)
{
    x->ref--;
    if (x->ref <= 0) {
        decref(x->base_frame);
        decref(x->current_frame);
        decref(x->now);
        decref(x->accumulator);
        free(x);
    }
}
