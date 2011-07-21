#ifndef MAHCINE
#define MACHINE

// This file concerns itself with the process of the interpreter itself.

#include "oyster.h"

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

    return ret;
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

void set_accumulator(machine * m, oyster * value)
{
    oyster *t = m->accumulator;
    m->accumulator = value;
    incref(m->accumulator);
    decref(t);
}



frame *make_frame(frame * below,
                  table * scope,
                  table * scope_to_be,
                  table * scope_below, oyster * instruction, int flag)
{
    frame *ret = NEW(frame);

    ret->below = below;
    incref(ret->below);

    ret->scope = scope;
    incref(ret->scope);

    ret->scope_to_be = scope_to_be;
    incref(ret->scope_to_be);

    ret->scope_below = scope_below;
    incref(scope_below);

    ret->instruction = instruction;
    incref(instruction);

    ret->flag = flag;

    return ret;
}

void push_new_instruction(machine * m, oyster * instruction, int flag)
{
    frame *t = m->current_frame;
    m->current_frame = make_frame(t,
                                  m->now->scope,
                                  m->now->scope_to_be,
                                  m->now->scope_below, instruction, flag);
    incref(m->current_frame);
    decref(t);
}

void push_instruction_list(machine * m,
                           oyster * ins,
                           table * scope, table * scope_below)
{
    incref(ins);

    frame *top = NULL;
    frame **cur = &top;
    while (!nilp(ins)) {
        (*cur) = make_frame(NULL,
                            scope, NULL, scope_below, car(ins), EVALUATE);
        incref(*cur);

        cur = &((*cur)->below);

        oyster *ins2 = cdr(ins);
        incref(ins2);
        decref(ins);
        ins = ins2;
    }

    decref(ins);
    (*cur) = m->current_frame;
    m->current_frame = top;
}

#endif
