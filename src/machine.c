#ifndef MAHCINE
#define MACHINE

// This file concerns itself with the process of the interpreter itself.

#include "oyster.h"

machine *make_machine()
{
    machine *ret = NEW(machine);
    ret->base_frame = make_frame(make_table(), NULL);
    incref(ret->base_frame);

    ret->current_frame = ret->base_frame;
    incref(ret->current_frame);

    ret->accumulator = NULL;
    ret->paused = 0;

    ret->ref = 0;
    ret->incref = &machine_ref;
    ret->decref = &machine_unref;


    return ret;
}

void machine_ref(machine * x)
{
    x->ref++;
}

void machine_unref(machine * x)
{
    x->ref--;
    if (x->ref <= 0) {
        decref(x->current_frame);
        decref(x->base_frame);
        decref(x->accumulator);
        free(x);
    }
}

instruction *machine_pop_current_instruction(machine * m)
{
    instruction *i = m->current_frame->current_instruction;
    if (i) {
        m->current_frame->current_instruction = i->next;
        incref(i->next);
    }
    return i;
}

void machine_pop_stack(machine * m)
{
    if (m->current_frame->below) {
        frame *t = m->current_frame;
        m->current_frame = m->current_frame->below;
        incref(m->current_frame);
        decref(t);
    } else {
        m->paused = 1;
    }
}

void set_accumulator(machine * m, oyster * value)
{
    oyster *t = m->accumulator;
    m->accumulator = value;
    incref(m->accumulator);
    decref(t);
}


frame *make_frame(table * scope, frame * below)
{
    frame *ret = NEW(frame);

    ret->scope = scope;
    incref(ret->scope);

    ret->scope_to_be = make_table();
    incref(ret->scope_to_be);

    ret->current_instruction = NULL;
    ret->signal_handler = NULL;

    ret->below = below;
    incref(ret->below);

    ret->ref = 0;
    ret->incref = &frame_ref;
    ret->decref = &frame_unref;

    return ret;
}

void frame_ref(frame * x)
{
    x->ref++;
}

void frame_unref(frame * x)
{
    x->ref--;
    if (x->ref <= 0) {
        decref(x->scope);
        decref(x->scope_to_be);
        decref(x->current_instruction);
        decref(x->below);
        free(x);
    }
}

void frame_set_scope(frame * x, table * scope)
{
    decref(x->scope);
    x->scope = scope;
    incref(x->scope);
}

void frame_set_instruction(frame * x, instruction * i)
{
    instruction *t = x->current_instruction;
    x->current_instruction = i;
    incref(x->current_instruction);
    decref(t);
}

void push_current_frame(machine * m, table * scope)
{
    incref(scope);
    if (0 &&                    // Sheee-it. TCO is tricky with 'leak. 
        m->current_frame->current_instruction == NULL &&
        m->current_frame->below &&
        m->current_frame->below != m->base_frame) {
        frame *t = m->current_frame;
        m->current_frame = m->current_frame->below;
        incref(m->current_frame);
        decref(t);
        push_current_frame(m, scope);
    } else {
        frame *f = make_frame(scope, m->current_frame);
        incref(f);
        decref(m->current_frame);
        m->current_frame = f;
    }
    decref(scope);
}



instruction *make_instruction(oyster * ins, int flag, instruction * next)
{
    instruction *ret = NEW(instruction);

    ret->instruction = ins;
    incref(ret->instruction);

    ret->flag = flag;

    ret->next = next;
    incref(ret->next);

    ret->ref = 0;
    ret->incref = &instruction_ref;
    ret->decref = &instruction_unref;
    return ret;
}

void instruction_ref(instruction * x)
{
    if (x)
        x->ref++;
}

void instruction_unref(instruction * x)
{
    x->ref--;
    if (x->ref <= 0) {
        decref(x->instruction);
        decref(x->next);
        free(x);
    }
}

#endif
