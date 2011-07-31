#ifndef INTERPRETER
#define INTERPRETER

#include "oyster.h"
#include "machine.h"
#include "frame.h"

int DEBUG = 0;

void step_machine(machine * m)
{
    frame *instruct = machine_pop_stack(m);
    if (DEBUG)
        machine_print(m);     
    if (!instruct)
        return;

    switch (instruct->flag) {

    case EVALUATE:
        evaluate_oyster(instruct, m);
        break;

    case PREPARE_ARGUMENTS:
        {
            table *t = m->now->scope_to_be;
            m->now->scope_to_be = (m->accumulator->bindings ?
                                   table_copy(m->accumulator->bindings) :
                                   make_table());
            incref(m->now->scope_to_be);
            decref(t);
        }

        set_accumulator(m, oyster_copy(m->accumulator, make_table()));

        push_new_instruction(m, cdr(m->accumulator), APPLY_FUNCTION);
        argument_chain_link(car(m->accumulator), instruct->instruction, m);
        break;

    case CONTINUE:
        argument_chain_link(car(instruct->instruction),
                            car(cdr(instruct->instruction)), m);
        break;

    case ATPEND_CONTINUE:
        argument_chain_link(car(instruct->instruction),
                            append(unevaluate_list(m->accumulator),
                                   car(cdr(instruct->instruction))), m);
        break;

    case ASTERPEND_CONTINUE:
        argument_chain_link(car(instruct->instruction),
                            append(m->accumulator,
                                   car(cdr(instruct->instruction))), m);
        break;

    case ARGUMENT:
        table_put(instruct->instruction->in->symbol_id,
                  m->accumulator, m->current_frame->scope_to_be);
        break;

    case ELIPSIS_ARGUMENT:
        elipsis_argument(car(cdr(instruct->instruction)),
                         car(instruct->instruction),
                         cons(m->accumulator, 
                              car(cdr(cdr(instruct->instruction)))),
                         m); //unweildy!
        break;

    case ELIPSIS_ATPEND_CONTINUE:
        elipsis_argument(append(unevaluate_list(m->accumulator),
                                car(cdr(instruct->instruction))),
                         car(instruct->instruction),
                         car(cdr(cdr(instruct->instruction))),
                         m); //unweildy!
        break;

    case ELIPSIS_ASTERPEND_CONTINUE:
        elipsis_argument(append(m->accumulator,
                                car(cdr(instruct->instruction))),
                         car(instruct->instruction),
                         car(cdr(cdr(instruct->instruction))),
                         m); //unweildy!
        break;


    case APPLY_FUNCTION:
        push_instruction_list(m,
                              instruct->instruction,
                              instruct->scope_to_be, m->now->scope);
    }
}

void evaluate_oyster(frame * instruct, machine * m)
{
    oyster *object = instruct->instruction;
    incref(object);

    if (object->bindings && !table_empty(object->bindings)) {
        push_bindings_to_scope(m, object);

    } else {

        switch (oyster_type(object)) {

        case BUILT_IN_FUNCTION:
            set_accumulator(m, object->in->built_in(m));
            break;

        case SYMBOL:
            set_accumulator(m, look_up(object->in->symbol_id, instruct));
            if (!m->accumulator) {
                oyster *signal = list(2,
                                      make_symbol(sym_id_from_string
                                                  ("Lookup-fail-error")),
                                      object);
                toss_signal(make_signal(signal, m), m);
            }
            break;

        case CONS:
            if (car_is_sym(object, CLEAR)) {
                set_accumulator(m, car(cdr(object)));

            } else {
                push_new_instruction(m, cdr(object), PREPARE_ARGUMENTS);
                push_new_instruction(m, car(object), EVALUATE);
            }
            break;

        default:
            set_accumulator(m, object);
            break;
        }
    }

    decref(object);
}




//---------------------------- handling argument lists -----------------------//

int car_is_sym(oyster * x, int sym)
{
    // Sometimes reference counting makes a simple thing so complicated.
    incref(x);
    int ret;
    if (x->in->type != CONS) {
        ret = 0;
    } else {
        oyster *c = cheap_car(x);
        ret = (c->in->type == SYMBOL && c->in->symbol_id == sym);
    }
    decref(x);
    return ret;
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

void push_argument(oyster * argument, oyster * name, int flag,
                   oyster * continu, machine * m)
{
    push_new_instruction(m, continu, CONTINUE);
    push_new_instruction(m, name, flag);
    push_new_instruction(m, argument, EVALUATE);
}

void elipsis_argument(oyster *arg_list, oyster * lambda,
                      oyster *so_far, machine * m)
{
    incref(lambda);
    incref(arg_list);
 
    //    oyster_print(arg_list); printf("\n");

    if(nilp(arg_list)){
        oyster *name = lambda;
        if (lambda->in->type == CONS) {
            name = car(cdr(lambda));
        } 
        push_new_instruction(m, name, ARGUMENT);
        push_new_instruction(m, list(2, make_symbol(CLEAR), reverse(so_far)), EVALUATE);
        decref(lambda);
        decref(arg_list);
        return;
    }
    
    oyster *arg = car(arg_list); // CODE DUPLICATION! todo remove
    incref(arg);
    
    if (car_is_sym(arg, ASTERIX)) {
        // * arguments are lists of arguments that should be stiched in place.
        push_new_instruction(m,
                             list(3, lambda, cdr(arg_list), so_far),
                             ELIPSIS_ASTERPEND_CONTINUE);
        push_new_instruction(m, car(cdr(arg)), EVALUATE);
        
    } else if (car_is_sym(arg, ATPEND)) {
        // @ arguments are lists of data that should be stiched in but not evaluated
        push_new_instruction(m,
                             list(3, lambda, cdr(arg_list), so_far),
                             ELIPSIS_ATPEND_CONTINUE);
        push_new_instruction(m, car(cdr(arg)), EVALUATE);
        
    } else {

        oyster *func = arg;
        if (lambda->in->type == CONS) {
            if (car_is_sym(arg, REALLY)) {
                func = car(cdr(arg));
            } else {
                func = append(list(2, car(lambda), arg), cdr(cdr(lambda)));
            }
        }

        push_new_instruction(m, list(3, lambda, cdr(arg_list), so_far), ELIPSIS_ARGUMENT);
        push_new_instruction(m, func, EVALUATE);
    }
    decref(lambda);
    decref(arg);
    decref(arg_list);
}


void push_normal_argument(oyster * arg, oyster * lambda_list,
                          oyster * arg_list, machine * m)
{

    oyster *lambda = car(lambda_list);
    oyster *name, *func;
    
    incref(lambda);
    
    if (lambda->in->type == CONS) {
            // The argument name is wrapped in a function call
        name = car(cdr(lambda));
        
        if (car_is_sym(arg, REALLY)) {
            // but the argument is wrapped in really! Do nothing.
            func = car(cdr(arg));
            
        } else {
            // The argument is normal --- wrap that sucker!
            func = append(list(2, car(lambda), arg), cdr(cdr(lambda)));
        }
    } else {
        // it's a boring, every-dey argument/name pai--- I mean, it's an 
            // extra special snowflake, just like you.
        name = lambda;
        func = arg;
    }
    
    push_argument(func, name, ARGUMENT,
                  list(2, cdr(lambda_list), cdr(arg_list)), m);
    
    decref(lambda);
}

void argument_chain_link(oyster * lambda_list,
                         oyster * arg_list, machine * m)
{
    incref(lambda_list);
    incref(arg_list);

    if(!nilp(lambda_list) && lambda_list->in->type != CONS){
        oyster *signal = list(2, make_symbol(sym_id_from_string
                                             ("Bad lambda list?")),
                              lambda_list);
        toss_signal(make_signal(signal, m), m);
        return;
    }

    oyster *arg = car(arg_list);
    incref(arg);

    if (nilp(lambda_list)) {
        // DO NOTHING!

    } else if (car_is_sym(arg, ASTERIX)) {
        // * arguments are lists of arguments that should be stitched in place.
        push_new_instruction(m,
                             list(2, lambda_list, cdr(arg_list)),
                             ASTERPEND_CONTINUE);
        push_new_instruction(m, car(cdr(arg)), EVALUATE);

    } else if (car_is_sym(arg, ATPEND)) {
        // @ arguments are lists of data that should be stiched in but not evaluated
        push_new_instruction(m,
                             list(2, lambda_list, cdr(arg_list)),
                             ATPEND_CONTINUE);
        push_new_instruction(m, car(cdr(arg)), EVALUATE);

    } else if (car_is_sym(lambda_list, ELIPSIS)) {
        elipsis_argument(arg_list, car(cdr(lambda_list)), nil(), m);

    } else {
        // It's a normal argument.
        push_normal_argument(arg, lambda_list, arg_list, m);
    }

    decref(arg);
    decref(arg_list);
    decref(lambda_list);
    return;
}



oyster *unevaluate_list(oyster * xs)
{
    // CLEAR leaves an argument unevaluated. This is sort of a cheat,
    // to make @pending work. A part of me wonders: would quote do as well,
    // or even better? Write some sample cases.
    // Stack dependant --- convert to loop, please.
    incref(xs);
    oyster *ret;

    if (nilp(xs)) {
        ret = nil();
    } else {
        ret =
            cons(list(2, make_symbol(CLEAR), car(xs)),
                 unevaluate_list(cdr(xs)));
    }

    decref(xs);
    return ret;
}


void push_bindings_to_scope(machine * m, oyster * o)
{
    frame *t = m->current_frame;
    oyster *next = oyster_copy(o, make_table());
    m->current_frame = make_frame(t,
                                  reify_scope(o->bindings, m->now),
                                  m->now->scope_to_be,
                                  m->now->scope, next, EVALUATE);
    decref(t);
    incref(m->current_frame);
}




#endif
