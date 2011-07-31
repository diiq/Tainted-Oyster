#include "oyster.h"
#include "machine.h"
#include "frame.h"
#include <stdlib.h>

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


frame *frame_below(frame *f){
    return f->below;
}

table *frame_scope(frame *f){
    return f->scope;
}

table *frame_scope_below(frame *f){
    return f->scope_below;
}

oyster *frame_instruction(frame *f){
    return f->instruction;
}

int frame_flag(frame *f){
    return f->flag;
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

//--------------------------- Memory -----------------------------//

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

        decref(x->instruction);
        free(x);
    }
}

//-------------------------- Printing ----------------------------//

void print_stack_trace(machine * m)
{
    printf("Now: ");
    frame_print(m->now, 1);

    frame *f = m->current_frame;
    while (f) {
        frame_print(f, 1);
        f = f->below;
    }
    if (m->accumulator) {
        printf("accum: ");
        oyster_print(m->accumulator);
    }
    printf
        ("\n--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---\n\n\n\n");
}

void frame_print(frame * i, int print_scope)
{
    char *flags[] = { 
        "ASTERPEND_CONTINUE",
        "ATPEND_CONTINUE",
        "ARGUMENT",
        "ELIPSIS_ARGUMENT",
        "ELIPSIS_ASTERPEND_CONTINUE",
        "ELIPSIS_ATPEND_CONTINUE",
        "EVALUATE",
        "CONTINUE",
        "APPLY_FUNCTION",
        "PREPARE_ARGUMENTS",
        "PAUSE",
        "HANDLE_SIGNALS"
    };

    printf(" --> ");
    printf("%s, ", flags[i->flag]);
    if (i->instruction)
        oyster_print(i->instruction);
    if (print_scope) {
        printf("\n Scope:\n");
        table_print(i->scope);
        printf("Upcoming scope:\n");
        table_print(i->scope_to_be);
    }
    printf("\n");
}
