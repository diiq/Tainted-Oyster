#ifndef MAHCINE
#define MACHINE

// This file concerns itself with the process of the interpreter itself.

#include "oyster.h"

machine *make_machine()
{
    machine *ret = NEW(machine);
    ret->base_frame = make_frame(NULL, make_table(), make_table(), NULL, NULL, PAUSE);
    incref(ret->base_frame);

    ret->current_frame = ret->base_frame;
    incref(ret->current_frame);

    ret->now = ret->current_frame;
    incref(ret->now);

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
        decref(x->now);
        decref(x->accumulator);
        free(x);
    }
}

frame *machine_pop_stack(machine * m)
{
    if (m->current_frame &&
        m->current_frame->below) {
        frame *t = m->current_frame;
        m->current_frame = m->current_frame->below;
        incref(m->current_frame);
        
        frame *f = m->now;
        m->now = t;
        incref(t);
        decref(f);
        return t;
    } else {
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
                  frame * scope_below, 
                  oyster *instruction,
                  int flag)
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

    ret->signal_handler = NULL;

    ret->instruction = instruction;
    incref(instruction);
    ret->flag = flag;

    ret->ref = 0;
    ret->incref = &frame_ref;
    ret->decref = &frame_unref;

    return ret;
}

frame *new_instruction(frame *now, frame *below, oyster *instruction, int flag)
{
    if (below)
        return make_frame(below, 
                          below->scope, 
                          below->scope_to_be, 
                          below->scope_below, 
                          instruction, 
                          flag);
    return  make_frame(below, 
                       make_table(), 
                       make_table(), 
                       NULL, 
                       instruction, 
                       flag);
}

void push_new_instruction(machine *m, oyster *instruction, int flag){
    frame *t = m->current_frame;
    table *scope_to_be;
    if(flag == EVALUATE)
        scope_to_be = make_table();
    else
        scope_to_be = m->now->scope_to_be;
    m->current_frame = make_frame(t, 
                                  m->now->scope, 
                                  scope_to_be, 
                                  m->now->scope_below, 
                                  instruction, 
                                  flag);
    incref(m->current_frame);
    decref(t);
}


void frame_ref(frame * x)
{
    x->ref++;
}

void frame_unref(frame * x)
{
    x->ref--;
    if (x->ref <= 0) {
        decref(x->below);

        decref(x->scope);
        decref(x->scope_to_be);
        decref(x->scope_below);

        decref(x->signal_handler);

        decref(x->instruction);
        free(x);
    }
}


//------------------------ Printing ---------------------------//

void machine_print(machine * m)
{
    frame *f = m->current_frame;
    printf("Now: ");
    frame_print(m->now);
    if(m->now->instruction && !table_empty(m->now->instruction->bindings)){
        printf(" with the bindings: \n");
        table_print(m->now->instruction->bindings);
    }
    while (f) {
        printf("vvv vvv vvv\n\n");
        printf("frame: ");
        frame_print(f);
        f = f->below;
    }
    if (m->accumulator) {
        printf("accum: ");oyster_print(m->accumulator);
    }
    printf("\n--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---\n\n\n\n");
}

void frame_print(frame * i)
{
    char *flags[] = {"ASTERPEND_CONTINUE",
                     "ATPEND_CONTINUE",
                     "ARGUMENT",
                     "ELIPSIS_ARGUMENT",
                     "EVALUATE",
                     "CONTINUE",
                     "APPLY_FUNCTION",
                     "PREPARE_ARGUMENTS",
                     "PAUSE"};

    printf(" --> ");
    printf("%s, ", flags[i->flag]);
    if (i->instruction)
        oyster_print(i->instruction);
    printf("\n Scope:\n");
    table_print(i->scope);
    printf("Upcoming scope:\n");
    table_print(i->scope_to_be);
}







#endif
