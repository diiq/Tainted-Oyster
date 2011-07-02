#ifndef MAHCINE
#define MACHINE

#include "oyster.h"


frame *make_frame(table *scope, frame* below)
{
    frame *ret = NEW(frame);

    ret->scope = scope;
    table_ref(ret->scope);

    ret->scope_to_be = make_table();
    table_ref(ret->scope_to_be);

    ret->current_instruction = NULL;

    ret->below = below;
    frame_ref(ret->below);

    ret->ref = 0;
    return ret;
}

void frame_ref(frame *x){
    if(x) x->ref++;
}
void frame_free(frame *x)
{
    table_unref(x->scope);
    table_unref(x->scope_to_be);
    instruction_unref(x->current_instruction);
    frame_unref(x->below);
    free(x);
}
void frame_unref(frame *x){
    if(x){
        x->ref--;
        if(x->ref <= 0)
            frame_free(x);
    }
}

void frame_set_scope(frame *x, table *scope){
    table_unref(x->scope);
    x->scope = scope;
    table_ref(x->scope);
}

void frame_set_instruction(frame *x,instruction *i){
    instruction_unref(x->current_instruction);
    x->current_instruction = i;
    instruction_ref(x->current_instruction);
}

machine *make_machine()
{
    machine *ret = NEW(machine);
    ret->base_frame = make_frame(make_table(), NULL);
    frame_ref(ret->base_frame);

    ret->current_frame = make_frame(make_table(), ret->base_frame);
    frame_ref(ret->current_frame);

    ret->accumulator = NULL;
    ret->paused = 0;

    ret->ref = 0;

    return ret;
}

void machine_ref(machine *x)
{
    x->ref++;
}

void machine_free(machine *x){
    frame_unref(x->current_frame);
    frame_unref(x->base_frame);
    oyster_unref(x->accumulator);
    free(x);
}

void machine_unref(machine *x)
{
    x->ref--;
    if(x->ref <= 0)
        machine_free(x);
}

void set_current_frame(machine *m, frame *f)
{
    frame_unref(m->current_frame);
    m->current_frame = f;
    frame_ref(m->current_frame);
}

instruction *make_instruction(oyster *ins, int flag, instruction *next)
{
    instruction *ret = NEW(instruction);

    ret->instruction = ins;
    oyster_ref(ret->instruction);

    ret->flag = flag;

    ret->next = next;
    instruction_ref(ret->next);

    ret->ref = 0;
    return ret;
}

void instruction_ref(instruction *x){
    if(x) x->ref++;
}

void instruction_unref(instruction *x){
    if(x != NULL){
        x->ref--;
        if(x->ref <= 0){
            oyster_unref(x->instruction);
            instruction_unref(x->next);
            free(x);
        }
    }
}


//---------------------------- handling argument lists -----------------------//

int car_is_sym(oyster *x, int sym)
{
    oyster_ref(x);
    int ret;
    if (x->in->type != CONS) {
        ret = 0;
    } else {
        oyster *c = cheap_car(x);
        ret = (c->in->type == SYMBOL &&
                   c->in->symbol_id  == sym);
    }
    oyster_unref(x);
    return ret;
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
    oyster_ref(lambda_list);
    oyster_ref(arg_list);

    if (nilp(lambda_list)) {
        oyster_unref(lambda_list);
        oyster_unref(arg_list);
        return chain;
    }
    
    oyster *arg = car(arg_list);
    oyster_ref(arg);

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
        oyster_ref(lambda);
        
        oyster *name, *func;
        int flag = ARGUMENT;
        
        if (elipsis_p(lambda)) {
            if (nilp(arg_list)) {
                oyster_unref(arg);
                oyster_unref(arg_list);
                oyster_unref(lambda_list);
                oyster_unref(lambda);
                return chain; 
            }
            oyster_unref(lambda);
            lambda = car(cdr(lambda_list));
            oyster_ref(lambda);

            oyster *lambda2 = cons(nil(), lambda_list);
            oyster_unref(lambda_list);
            lambda_list = lambda2;
            oyster_ref(lambda_list);

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
        oyster_unref(lambda);
    }
    oyster_unref(arg);
    oyster_unref(arg_list);
    oyster_unref(lambda_list);
    return chain;
}

oyster *unevaluate_list(oyster *xs)
{
    // Stack dependant --- convert to loop, please.
    oyster_ref(xs);
    oyster *ret;

    if (nilp(xs)){
        ret = nil();
    } else {
        ret = cons(list(2, make_symbol(CLEAR), car(xs)), unevaluate_list(cdr(xs)));
    }
    
    oyster_unref(xs);
    return ret;
}

//------------------------- Machine Operation --------------------------------//

instruction *machine_pop_current_instruction(machine *m)
{
    instruction *i = m->current_frame->current_instruction;
    if (i) {
        m->current_frame->current_instruction = i->next;
        instruction_ref(i->next);
    }
    return i;
}

void machine_pop_stack(machine *m)
{ 
    if (m->current_frame->below){
        frame_ref(m->current_frame->below);
        set_current_frame(m, m->current_frame->below);
        frame_unref(m->current_frame);
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
    frame_set_scope(m->current_frame, binding_union(m->current_frame->scope, 
                                                    o->bindings));
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
    oyster_unref(m->accumulator);
    m->accumulator = value;
    oyster_ref(m->accumulator);
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
        oyster_ref(object);

        int object_type = oyster_type(object);

        if(object_type == BUILT_IN_FUNCTION){
            
            run_built_in_function(object, m);

        } else if (!table_empty(object->bindings)){
            
            set_current_frame(m,  make_frame(m->current_frame->scope, m->current_frame)); 
            push_bindings_to_scope(m, object);

        } else if (object_type == SYMBOL) {
            //printf("Lookup symbol. %d\n", object->in->symbol_id);
            set_accumulator(m, look_up_symbol(object, m));

        } else if (object_type == CONS) {
            //printf("Cons, ");
            if (car_is_sym(object, CLEAR)) {
                //printf("Clear.\n");
                set_accumulator(m, car(cdr(object)));

            } else {     // object is a function call!
                //printf("Function call.\n");
                set_current_frame(m, make_frame(m->current_frame->scope, m->current_frame)); 
                // TCO here
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

        oyster_unref(object);


    } else if (instruct->flag == PREPARE_ARGUMENTS){
        // This step is unnecessary and should be removed. -s
        //printf("Preparing arguments.\n");
        frame_set_instruction(m->current_frame, 
                                make_instruction(cdr(m->accumulator),
                                                 APPLY_FUNCTION,
                                                 m->current_frame->current_instruction));

        frame_set_instruction(m->current_frame, 
                              argument_chain_link(car(m->accumulator),
                                                  instruct->instruction,
                                                  m->current_frame->current_instruction));

    } else if (instruct->flag == CONTINUE){
        //printf("Continuing arguments.\n");
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
        //printf("Argument.\n");
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
        //printf("applying function.\n");
        // This is super ridiculous and must be trimmed. MUST.
        frame_set_scope(m->current_frame,  m->current_frame->scope_to_be);

        table_unref(m->current_frame->scope_to_be);
        m->current_frame->scope_to_be = make_table();
        table_ref(m->current_frame->scope_to_be);

        instruction *handle = m->current_frame->current_instruction;
        if (!handle) 
            handle = make_instruction(NULL, EVALUATE, NULL);
        instruction *chain = handle;
        instruction_ref(handle);
        oyster *procs = instruct->instruction;
        while(!nilp(procs)){
            oyster_ref(procs);
            instruction_unref(chain->next);
            chain->next = make_instruction(car(procs), EVALUATE, NULL);
            instruction_ref(chain->next);
            chain = chain->next;
            oyster *procs2 = cdr(procs); 
            oyster_unref(procs);
            procs = procs2;
        }
        oyster_unref(procs);
        if(handle->instruction == NULL &&
           handle->flag == EVALUATE){
            instruction *thandle = handle->next;
            instruction_ref(thandle);
            instruction_unref(handle);
            handle = thandle;
        }
        frame_set_instruction(m->current_frame,  handle);
        instruction_unref(handle);

    }

    instruction_unref(instruct);
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
        //if(m->accumulator)
            //            oyster_print(m->accumulator);
    }
    g_scanner_destroy(in);
    oyster *ret = m->accumulator;
    oyster_ref(ret);

    machine_unref(m);
    return ret;
}


oyster *evaluate_string(char *str)
{
    return evaluate_scan(string_scanner(str));
}

#endif
