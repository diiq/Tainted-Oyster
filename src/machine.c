#ifndef MAHCINE
#define MACHINE

// This file concerns itself with the process of the interpreter itself.

#include "oyster.h"



machine *make_machine()
{
    machine *ret = NEW(machine);
    ret->base_frame = make_frame(make_table(), NULL);
    incref(ret->base_frame);

    ret->current_frame = make_frame(make_table(), ret->base_frame);
    incref(ret->current_frame);

    ret->accumulator = NULL;
    ret->paused = 0;

    ret->ref = 0;
    ret->incref = &machine_ref;
    ret->decref = &machine_unref;


    return ret;
}

void machine_ref(machine *x)
{
    x->ref++;
}

void machine_unref(machine *x)
{
    x->ref--;
    if(x->ref <= 0){
        decref(x->current_frame);
        decref(x->base_frame);
        decref(x->accumulator);
        free(x);
    }
}



frame *make_frame(table *scope, frame* below)
{
    frame *ret = NEW(frame);

    ret->scope = scope;
    incref(ret->scope);

    ret->scope_to_be = make_table();
    incref(ret->scope_to_be);

    ret->current_instruction = NULL;

    ret->below = below;
    incref(ret->below);

    ret->ref = 0;
    ret->incref = &frame_ref;
    ret->decref = &frame_unref;

    return ret;
}

void frame_ref(frame *x){
    x->ref++;
}

void frame_unref(frame *x){
    x->ref--;
    if(x->ref <= 0) {
        decref(x->scope);
        decref(x->scope_to_be);
        decref(x->current_instruction);
        decref(x->below);
        free(x);
    }
}

void frame_set_scope(frame *x, table *scope){
    decref(x->scope);
    x->scope = scope;
    incref(x->scope);
}

void frame_set_instruction(frame *x,instruction *i){
    instruction *t = x->current_instruction;
    x->current_instruction = i;
    incref(x->current_instruction);
    decref(t);
}

void set_current_frame(machine *m, frame *f)
{
    // Add TCO here!
    decref(m->current_frame);
    m->current_frame = f;
    incref(m->current_frame);
}



instruction *make_instruction(oyster *ins, int flag, instruction *next)
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

void instruction_ref(instruction *x){
    if(x) x->ref++;
}

void instruction_unref(instruction *x){
    x->ref--;
    if(x->ref <= 0){
        decref(x->instruction);
        decref(x->next);
        free(x);
    }
}


//---------------------------- handling argument lists -----------------------//

int car_is_sym(oyster *x, int sym)
{
    // Sometimes reference counting makes a simple thing so complicated.
    incref(x);
    int ret;
    if (x->in->type != CONS) {
        ret = 0;
    } else {
        oyster *c = cheap_car(x);
        ret = (c->in->type == SYMBOL &&
                   c->in->symbol_id  == sym);
    }
    decref(x);
    return ret;
}

int elipsis_p(oyster *x)
{
    return (x->in->type == SYMBOL &&
            x->in->symbol_id == ELIPSIS);
}


// This one is overdue for a cleanup. Too long, too abstruse.
// Each time argument_chain_link is called, it returns a chain of instructions
// that will evaluate and bind a single argument. The process is then repeated.
// This is unusual, but allows for several tricksy tricks: 
// a lambda-list-element wrapped in a function: (car x), the argument will
// be bound to x only after being passed through car. 
// an argument prefaced by , will NOT be passed through the associated function,
// and * and @ allow lists to be inserted into the argument chain without recourse
// to (apply).
instruction *argument_chain_link(oyster *lambda_list, 
                            oyster *arg_list, 
                            instruction *chain)
{
    incref(lambda_list);
    incref(arg_list);

    if (nilp(lambda_list)) {
        decref(lambda_list);
        decref(arg_list);
        return chain;
    }
    
    oyster *arg = car(arg_list);
    incref(arg);

    if (car_is_sym(arg, ASTERIX)){
        chain = make_instruction(list(2, lambda_list, cdr(arg_list)),
                                 ASTERPEND_CONTINUE,
                                 chain);
        chain = make_instruction(car(cdr(arg)), 
                                 EVALUATE,
                                 chain);

    } else if (car_is_sym(arg, ATPEND)){
        chain = make_instruction(list(2, lambda_list, cdr(arg_list)),
                                 ATPEND_CONTINUE,
                                 chain);
        chain = make_instruction(car(cdr(arg)), 
                                 EVALUATE,
                                 chain);

    } else {
        oyster *lambda = car(lambda_list);
        incref(lambda);
        
        oyster *name, *func;
        int flag = ARGUMENT;
        
        if (elipsis_p(lambda)) {
            if (nilp(arg_list)) {
                decref(arg);
                decref(arg_list);
                decref(lambda_list);
                decref(lambda);
                return chain; 
            }
            decref(lambda);
            lambda = car(cdr(lambda_list));
            incref(lambda);

            oyster *lambda2 = cons(nil(), lambda_list);
            decref(lambda_list);
            lambda_list = lambda2;
            incref(lambda_list);

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
        decref(lambda);
    }
    decref(arg);
    decref(arg_list);
    decref(lambda_list);
    return chain;
}

oyster *unevaluate_list(oyster *xs)
{
    // CLEAR leaves an argument unevaluated. This is sort of a cheat,
    // to make @pending work. A part of me wonders: would quote do as well,
    // or even better? Write some sample cases.
    // Stack dependant --- convert to loop, please.
    incref(xs);
    oyster *ret;

    if (nilp(xs)){
        ret = nil();
    } else {
        ret = cons(list(2, make_symbol(CLEAR), car(xs)), unevaluate_list(cdr(xs)));
    }
    
    decref(xs);
    return ret;
}

//------------------------- Machine Operation --------------------------------//

instruction *machine_pop_current_instruction(machine *m)
{
    instruction *i = m->current_frame->current_instruction;
    if (i) {
        m->current_frame->current_instruction = i->next;
        incref(i->next);
    }
    return i;
}

void machine_pop_stack(machine *m)
{ 
    if (m->current_frame->below){
        incref(m->current_frame->below);
        set_current_frame(m, m->current_frame->below);
        decref(m->current_frame);
    } else {
        m->paused = 1;
    }
}

void push_bindings_to_scope(machine *m, oyster *o)
{
    set_current_frame(m, make_frame(binding_union(m->current_frame->scope, 
                                                  o->bindings), 
                                    m->current_frame)); 
    oyster *next = oyster_copy(o, make_table());
    frame_set_instruction(m->current_frame,
                          make_instruction(next, EVALUATE, 
                                           m->current_frame->current_instruction));
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

void set_accumulator(machine *m, oyster *value){
    decref(m->accumulator);
    m->accumulator = value;
    incref(m->accumulator);
}

void step_machine(machine *m)
{
    instruction *instruct = machine_pop_current_instruction(m);

    if (instruct == NULL) {
        machine_pop_stack(m);
        return;
    }

	if (instruct->flag == EVALUATE){

        oyster *object = instruct->instruction;
        incref(object);

        int object_type = oyster_type(object);

        if(object_type == BUILT_IN_FUNCTION){
            
            set_accumulator(m, object->in->built_in(m));

        } else if (!table_empty(object->bindings)){
            
            push_bindings_to_scope(m, object);

        } else if (object_type == SYMBOL) {

            set_accumulator(m, look_up_symbol(object, m));

        } else if (object_type == CONS) {
            
            if (car_is_sym(object, CLEAR)) {
                
                set_accumulator(m, car(cdr(object)));

            } else { 

                set_current_frame(m, make_frame(m->current_frame->scope, m->current_frame));
                frame_set_instruction(m->current_frame,
                                      make_instruction(cdr(object), 
                                                       PREPARE_ARGUMENTS, 
                                                       NULL));
                frame_set_instruction(m->current_frame,
                                      make_instruction(car(object), 
                                                       EVALUATE, 
                                                       m->current_frame->current_instruction));
              }

        } else {
               
                set_accumulator(m, object);

        }

        decref(object);


    } else if (instruct->flag == PREPARE_ARGUMENTS){
        // We now know what the function is, we can begin to evaluate args.

        frame_set_instruction(m->current_frame, 
                                make_instruction(cdr(m->accumulator),
                                                 APPLY_FUNCTION,
                                                 m->current_frame->current_instruction));

        frame_set_instruction(m->current_frame, 
                              argument_chain_link(car(m->accumulator),
                                                  instruct->instruction,
                                                  m->current_frame->current_instruction));

    } else if (instruct->flag == CONTINUE){

        // Each time we hit here, that's an argument prepped.
        frame_set_instruction(m->current_frame,
                              argument_chain_link(car(instruct->instruction),
                                                  car(cdr(instruct->instruction)),
                                                  m->current_frame->current_instruction));     
        
    } else if (instruct->flag == ATPEND_CONTINUE){
        
        frame_set_instruction(m->current_frame,
                              argument_chain_link(car(instruct->instruction),
                                                  append(unevaluate_list(m->accumulator),
                                                         car(cdr(instruct->instruction))),
                                                  m->current_frame->current_instruction));
        
    } else if (instruct->flag == ASTERPEND_CONTINUE){
        
        frame_set_instruction(m->current_frame,
                              argument_chain_link(car(instruct->instruction),
                                                  append(m->accumulator,
                                                         car(cdr(instruct->instruction))),
                                                  m->current_frame->current_instruction));

    } else if (instruct->flag == ARGUMENT){

        // Each time ya' get here, that's an argument evaluated and bound.
		table_put(instruct->instruction->in->symbol_id,
                  m->accumulator,
                  m->current_frame->scope_to_be);

    } else if (instruct->flag == ELIPSIS_ARGUMENT){
       
        int sym = instruct->instruction->in->symbol_id;
        int i = 0;
        oyster *so_far = table_get(sym, m->current_frame->scope_to_be, &i);
        if (i){
            oyster *new = append(so_far, list(1, m->accumulator));

            table_put(sym, 
                      new,
                      m->current_frame->scope_to_be);

        } else {
            table_put(sym, 
                      list(1, m->accumulator),
                      m->current_frame->scope_to_be);
        }	


    } else if (instruct->flag == APPLY_FUNCTION) {

        // This is super ridiculous and must be trimmed. MUST.
        frame_set_scope(m->current_frame,  m->current_frame->scope_to_be);

        decref(m->current_frame->scope_to_be);
        m->current_frame->scope_to_be = make_table();
        incref(m->current_frame->scope_to_be);

        instruction *handle = m->current_frame->current_instruction;
        if (!handle) 
            handle = make_instruction(NULL, EVALUATE, NULL);
        instruction *chain = handle;
        incref(handle);
        oyster *procs = instruct->instruction;
        while(!nilp(procs)){
            incref(procs);
            decref(chain->next);
            chain->next = make_instruction(car(procs), EVALUATE, NULL);
            incref(chain->next);
            chain = chain->next;
            oyster *procs2 = cdr(procs); 
            decref(procs);
            procs = procs2;
        }
        decref(procs);
        if(handle->instruction == NULL &&
           handle->flag == EVALUATE){
            instruction *thandle = handle->next;
            incref(thandle);
            decref(handle);
            handle = thandle;
        }
        frame_set_instruction(m->current_frame,  handle);
        decref(handle);

    }

    decref(instruct);
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
    add_builtins(m);
    while(func){
        m->current_frame->current_instruction = 
            make_instruction(func, EVALUATE, NULL);
        while(!m->paused){
            step_machine(m);
        }
        func = next_oyster(in);
        //if(m->accumulator)
            //            oyster_print(m->accumulator);
    }
    g_scanner_destroy(in);
    oyster *ret = m->accumulator;
    incref(ret);

    decref(m);
    return ret;
}


oyster *evaluate_string(char *str)
{
    return evaluate_scan(string_scanner(str));
}

#endif
