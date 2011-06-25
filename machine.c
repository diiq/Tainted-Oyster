#ifndef MAHCINE
#define MACHINE

#include "machine.h"
#include "oyster.h"


frame *make_frame(table *scope)
{
    frame *ret = NEW(frame);
    ret->scope = scope;
    ret->scope_to_be = make_table();
    ret->current_instruction = NULL;
    ret->below = NULL;
    return ret;
}

machine *make_machine()
{
    machine *ret = NEW(machine);
    ret->base_frame = make_frame(make_table());
    ret->current_frame = ret->base_frame;
    ret->accumulator = NULL;
    return ret;
}

instruction *make_instruction(oyster *ins, int flag)
{
    instruction *ret = NEW(instruction);
    ret->instruction = ins;
    ret->flag = flag;
    ret->next = NULL;
    return ret;
}


int asterix_p(oyster *x)
{
    return (x->in->type == CONS &&
            cheap_car(x)->in->type == SYMBOL &&
            cheap_car(x)->in->symbol_id  == ASTERIX);
}


int atpend_p(oyster *x)
{
    return (x->in->type == CONS &&
            cheap_car(x)->in->type == SYMBOL &&
            cheap_car(x)->in->symbol_id  == ATPEND);
}

int elipsis_p(oyster *x){
    return (x->in->type == SYMBOL &&
            x->in->symbol_id == ELIPSIS);
}




#endif
