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

    ret->trace = make_stack_trace(arg("bottom-of-the-barrel"),
                                        NULL, 
                                        NULL);
    incref(ret->trace);


    add_builtins(ret);
    add_builtin_numbers(ret);
    //    add_builtin_strings(ret);
    //    add_builtin_files(ret);

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
    machine *ret = NEW(machine);
    ret->current_frame = m->current_frame;
    incref(ret->current_frame);

    ret->base_frame = m->base_frame;
    incref(ret->base_frame);

    ret->now = m->now;
    incref(ret->now);

    ret->trace= m->trace;
    incref(ret->trace);

    ret->accumulator = m->accumulator;
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
    if (m->now->instruction && !table_empty(oyster_bindings(m->now->instruction))) {
        printf(" with the bindings: \n");
        table_print(oyster_bindings(m->now->instruction));
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

void machine_free(machine * x)
{
    decref(x->base_frame);
    decref(x->current_frame);
    decref(x->now);
    decref(x->accumulator);
    free(x);
}

//------------------------------ Stack traces ------------------------//

stack_trace *make_stack_trace(oyster *function, frame *remove_when, stack_trace *below)
{
    stack_trace *ret = NEW(stack_trace);

    ret->function = function;
    incref(ret->function);

    ret->count = 1;

    ret->remove_when = remove_when;
    incref(ret->remove_when);

    ret->below = below;
    incref(ret->below);

    return ret;
}

void stack_trace_free(stack_trace *t)
{
    decref(t->function);
    decref(t->remove_when);
    decref(t->below);
    free(t);
}

void push_stack_trace(oyster *function, frame *remove_when, machine *m)
{
    if (remove_when == m->trace->remove_when){
        stack_trace *t = m->trace;
        while(remove_when == t->remove_when){
            if (oyster_value(t->function) == oyster_value(function)){
                t->count++;
                return;
            }
            t = t->below;
        }
    }
    stack_trace *t = m->trace;
    m->trace = make_stack_trace(function, remove_when, m->trace);
    incref(m->trace);
    decref(t);

}

void stack_trace_update(machine *m){
    while(machine_active_frame(m) == m->trace->remove_when){
        stack_trace *t = m->trace;
        m->trace = m->trace->below;
        incref(m->trace);
        decref(t);
    }
}


void print_stack_trace(machine * m)
{
    printf("\n\n\nNow: ");
    frame_print(m->now, 0);

    stack_trace *t = m->trace;
    while (t) {
        oyster_print(t->function); 
        if (t->count > 1)
            printf(" (recursed %d times)", t->count);
        printf("\n");
        t = t->below;
    }
    if (m->accumulator) {
        printf("value: ");
        oyster_print(m->accumulator);
    }
    printf
        ("\n--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---\n\n\n");
}
