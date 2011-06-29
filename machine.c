#ifndef MAHCINE
#define MACHINE

#include "oyster.h"


frame *make_frame(table *scope, frame* below)
{
    frame *ret = NEW(frame);
    ret->scope = scope;
    ret->scope_to_be = make_table();
    ret->current_instruction = NULL;
    ret->below = below;
    return ret;
}

machine *make_machine()
{
    machine *ret = NEW(machine);
    ret->base_frame = make_frame(make_table(), NULL);
    ret->current_frame = ret->base_frame;
    ret->accumulator = NULL;
    ret->paused = 0;
    return ret;
}

instruction *make_instruction(oyster *ins, int flag, instruction *next)
{
    instruction *ret = NEW(instruction);
    ret->instruction = ins;
    ret->flag = flag;
    ret->next = next;
    return ret;
}

oyster *instruction_object(instruction *i)
{
    return i->instruction;
}

//---------------------------- handling argument lists -----------------------//

int car_is_sym(oyster *x, int sym)
{
    return (x->in->type == CONS &&
            cheap_car(x)->in->type == SYMBOL &&
            cheap_car(x)->in->symbol_id  == sym);
}

int elipsis_p(oyster *x)
{
    return (x->in->type == SYMBOL &&
            x->in->symbol_id == ELIPSIS);
}


// This one is overdue for a cleanup. Too long, too abstruse.
instruction *argument_chain_link(oyster *lambda_list, 
                            oyster *arg_list, 
                            instruction *chain)
{
    if (nilp(lambda_list))
        return chain;
    
    oyster *arg = car(arg_list);

    if (car_is_sym(arg, ASTERIX)){
        chain = make_instruction(list(2, lambda_list, cdr(arg_list)),
                                ASTERPEND_CONTINUE,
                                chain);
        chain = make_instruction(car(cdr(arg)), 
                                 EVALUATE,
                                 chain);
        return chain;
    }
    if (car_is_sym(arg, ATPEND)){
        chain = make_instruction(list(2, lambda_list, cdr(arg_list)),
                                ATPEND_CONTINUE,
                                chain);
        chain = make_instruction(car(cdr(arg)), 
                                 EVALUATE,
                                 chain);
        return chain;
    }
    oyster *lambda = car(lambda_list);
    oyster *name, *func;
    int flag = ARGUMENT;

    if (elipsis_p(lambda)) {
        if (nilp(arg_list)) {
            return chain;
        }
        lambda = car(cdr(lambda_list));
        lambda_list = cons(nil, lambda_list);
        flag = ELIPSIS_ARGUMENT;
    }
	    
    if (lambda->in->type == CONS){
        name = car(cdr(lambda));
        if (car_is_sym(arg, COMMA)){            
            func = car(cdr(arg));
        } else {
            func = list(2, car(lambda), arg);
        }
    } else {
        name = lambda;
        func = arg;
    }

    chain = make_instruction(list(2, cdr(lambda_list), cdr(arg_list)),
                         CONTINUE,
                         chain);
    chain = make_instruction(func,
                             EVALUATE,
                             make_instruction(name, flag, chain));
    return chain;
}

oyster *unevaluate_list(oyster *xs)
{
    // Stack dependant --- convert to loop, please.
    if (nilp(xs)){
        return xs;
    }
    return cons(list(2, make_symbol(CLEAR), car(xs)), unevaluate_list(cdr(xs)));
}

//------------------------- Machine Operation --------------------------------//

instruction *machine_pop_current_instruction(machine *m)
{
    instruction *i = m->current_frame->current_instruction;
    if (i)
        m->current_frame->current_instruction = i->next;
    return i;
}

void machine_pop_stack(machine *m)
{ 
    if (m->current_frame->below != NULL){
        m->current_frame = m->current_frame->below;
    } else {
        m->paused = 1;
    }
}

void run_built_in_function(oyster *o, machine *m)
{
    o->in->built_in(m);
}

void push_bindings_to_scope(machine *m, oyster *o)
{
    // I do believe that this is WRONG?!
    // Doesn't this need a new frame? Devise a failure.
    m->current_frame->scope = binding_union(m->current_frame->scope, 
                                            o->bindings);
    oyster *next = oyster_copy(o);
    next->bindings = make_table();
    m->current_frame->current_instruction = 
        make_instruction(next, EVALUATE, m->current_frame->current_instruction);
}

oyster *look_up_symbol(oyster *sym, machine *m)
{
    oyster *ret = look_up(sym->in->symbol_id, m);
    if (!ret) {
        printf("Error: apologies, dear sir, but I have no recollection of such a variable as %d\n", sym->in->symbol_id);
        return NULL;
    }
    return ret;
}

void step_machine(machine *m)
{
    instruction *instruct = machine_pop_current_instruction(m);

    if (instruct == NULL) {
        machine_pop_stack(m);
        return;
    }

	if (instruct->flag == EVALUATE){

        oyster *object = instruction_object(instruct);
        int object_type = oyster_type(object);

        if(object_type == BUILT_IN_FUNCTION){
            
            run_built_in_function(object, m);

        } else if (!table_empty(object->bindings)){
            
            m->current_frame = make_frame(m->current_frame->scope, m->current_frame); 
            push_bindings_to_scope(m, object);

        } else if (object_type == SYMBOL) {
            //printf("Lookup symbol. %d\n", object->in->symbol_id);
            m->accumulator = look_up_symbol(object, m);

        } else if (object_type == CONS) {
            //printf("Cons, ");
            if (car_is_sym(object, CLEAR)) {
                //printf("Clear.\n");
                m->accumulator = car(cdr(object));

            } else {     // object is a function call!
                //printf("Function call.\n");
                m->current_frame = make_frame(m->current_frame->scope, m->current_frame); 
                // TCO here
                m->current_frame->current_instruction = 
                    make_instruction(cdr(object), PREPARE_ARGUMENTS, NULL);
                m->current_frame->current_instruction = 
                    make_instruction(car(object), 
                                     EVALUATE, 
                                     m->current_frame->current_instruction);                
            }

        } else {

            m->accumulator = object;

        }


    } else if (instruct->flag == PREPARE_ARGUMENTS){
        // This step is unnecessary and should be removed. -s
        //printf("Preparing arguments.\n");
        m->current_frame->current_instruction = 
            make_instruction(cdr(m->accumulator),
                             APPLY_FUNCTION,
                             m->current_frame->current_instruction);

        m->current_frame->current_instruction = 
            argument_chain_link(car(m->accumulator),
                                instruct->instruction,
                                m->current_frame->current_instruction);                    

    } else if (instruct->flag == CONTINUE){
        //printf("Continuing arguments.\n");
        m->current_frame->current_instruction =
            argument_chain_link(car(instruct->instruction),
                                car(cdr(instruct->instruction)),
                                m->current_frame->current_instruction);                    

    } else if (instruct->flag == ATPEND_CONTINUE){

        m->current_frame->current_instruction =
            argument_chain_link(car(instruct->instruction),
                                append(unevaluate_list(m->accumulator),
                                       car(cdr(instruct->instruction))),
                                m->current_frame->current_instruction);

    } else if (instruct->flag == ASTERPEND_CONTINUE){

        m->current_frame->current_instruction =
            argument_chain_link(car(instruct->instruction),
                                append(m->accumulator,
                                       car(cdr(instruct->instruction))),
                                m->current_frame->current_instruction);

    } else if (instruct->flag == ARGUMENT){
        //printf("Argument.\n");
		table_put(instruct->instruction->in->symbol_id,
                  m->accumulator,
                  m->current_frame->scope_to_be);

    } else if (instruct->flag == ELIPSIS_ARGUMENT){
       
        int sym = instruct->instruction->in->symbol_id;
        int i = 0;
        oyster *so_far = table_get(sym, m->current_frame->scope_to_be, &i);
        if (i){
            table_put(sym, 
                      append(so_far, list(1, m->accumulator)),
                      m->current_frame->scope_to_be);
        } else {
            table_put(sym, 
                      list(1, m->accumulator),
                      m->current_frame->scope_to_be);
        }	


    } else if (instruct->flag == APPLY_FUNCTION) {
        //printf("applying function.\n");
        m->current_frame->scope = m->current_frame->scope_to_be;

        instruction *handle = m->current_frame->current_instruction;
        if (!handle) 
            handle = make_instruction(NULL, EVALUATE, NULL);
        instruction *chain = handle;
        oyster *procs = instruct->instruction;
        while(!nilp(procs)){
            chain->next = make_instruction(car(procs), EVALUATE, NULL);
            chain = chain->next;
            procs = cdr(procs);
        }
        if(handle->instruction == NULL &&
           handle->flag == EVALUATE)
            handle = handle->next;
        m->current_frame->current_instruction = handle;
    }
}


//------------------------ Printing ---------------------------//

void machine_print(machine *m)
{
    frame *f = m->current_frame;
    while(f){
        printf("frame: ");
        frame_print(f);
        f = f->below;
        printf("\n");
    }
    if(m->accumulator)
        oyster_print(m->accumulator);
    printf("\n---\n\n");
}

void frame_print(frame *f)
{
    instruction *i = f->current_instruction;
    while(i){
        printf(" --> ");
        instruction_print(i);
        i = i->next;
    }
    //    table_print(f->scope);
    //    table_print(f->scope_to_be);
}

void instruction_print(instruction *i)
{
    if (i->flag == EVALUATE){
        oyster_print(i->instruction);
    } else {
        printf("%d, ", i->flag);
        if(i->instruction)
            oyster_print(i->instruction);
    }
}



// ---------------------- Convenience --------------------------//

#include "parsing.h"
#include "string.h"

oyster *evaluate_scan(GScanner *in){
    oyster *func = next_oyster(in);
    machine *m = make_machine();
    while(func){
        m->current_frame->current_instruction = 
            make_instruction(func, EVALUATE, NULL);
        while(!m->paused){
            step_machine(m);
        }
        func = next_oyster(in);
        if(m->accumulator)
            oyster_print(m->accumulator);
    }
    return m->accumulator;
}


oyster *evaluate_string(char *str)
{
    return evaluate_scan(string_scanner(str));
}

#endif
