#ifndef INTERPRETER
#define INTERPRETER

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

int elipsis_p(oyster * x)
{
    return (x->in->type == SYMBOL && x->in->symbol_id == ELIPSIS);
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
void argument_chain_link(oyster * lambda_list,
                           oyster * arg_list, 
                           machine * m)
{
    incref(lambda_list);
    incref(arg_list);

    if (nilp(lambda_list)) {
        decref(lambda_list);
        decref(arg_list);
        return;
    }

    oyster *arg = car(arg_list);
    incref(arg);

    if (car_is_sym(arg, ASTERIX)) {
        push_new_instruction(m, 
                             list(2, lambda_list, cdr(arg_list)),
                             ASTERPEND_CONTINUE);
        push_new_instruction(m, car(cdr(arg)), EVALUATE);

    } else if (car_is_sym(arg, ATPEND)) {
        push_new_instruction(m, 
                             list(2, lambda_list, cdr(arg_list)),
                             ATPEND_CONTINUE);
        push_new_instruction(m, car(cdr(arg)), EVALUATE);

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
                return;
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

        if (lambda->in->type == CONS) {
            name = car(cdr(lambda));
            if (car_is_sym(arg, COMMA)) {
                func = car(cdr(arg));
            } else {
                func = list(2, car(lambda), arg);
            }
        } else {
            name = lambda;
            func = arg;
        }
        push_new_instruction(m, 
                             list(2, cdr(lambda_list), cdr(arg_list)),
                             CONTINUE);
        push_new_instruction(m, name, flag); 
        push_new_instruction(m, func, EVALUATE);
                                 
        decref(lambda);
    }
    decref(arg);
    decref(arg_list);
    decref(lambda_list);
    return;
}


//------------------------- Machine Operation --------------------------------//

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
                                  binding_union(m->now->scope,
                                                o->bindings),
                                  m->now->scope_to_be,
                                  t,
                                  next,
                                  EVALUATE);
    decref(t);
}


void evaluate_oyster(frame * instruct, machine * m)
{
    oyster *object = instruct->instruction;
    incref(object);
    int object_type = oyster_type(object);

    if (object_type == BUILT_IN_FUNCTION) {

        set_accumulator(m, object->in->built_in(m));

    } else if (!table_empty(object->bindings)) {

        push_bindings_to_scope(m, object);

    } else if (object_type == SYMBOL) {

        set_accumulator(m, look_up_symbol(object, instruct));

    } else if (object_type == CONS) {

        if (car_is_sym(object, CLEAR)) {
            set_accumulator(m, car(cdr(object)));

        } else {
            push_new_instruction(m, cdr(object), PREPARE_ARGUMENTS);
            push_new_instruction(m, car(object), EVALUATE);

        }

    } else {

        set_accumulator(m, object);
    }

    decref(object);
}


void step_machine(machine * m)
{
 
    frame *instruct = machine_pop_stack(m);
    //        machine_print(m); // Should be some kind of debug-mode to turn this on.
    if(!instruct) return;

    if (instruct->flag == EVALUATE) {

        evaluate_oyster(instruct, m);

    } else if (instruct->flag == PREPARE_ARGUMENTS) {
        push_new_instruction(m, cdr(m->accumulator), APPLY_FUNCTION);
        argument_chain_link(car(m->accumulator), instruct->instruction, m);

    } else if (instruct->flag == CONTINUE) {
        argument_chain_link(car(instruct->instruction),
                            car(cdr(instruct->instruction)),
                            m);

    } else if (instruct->flag == ATPEND_CONTINUE) {
        argument_chain_link(car(instruct->instruction),
                                append(unevaluate_list(m->accumulator),
                                       car(cdr(instruct->instruction))),
                                m);

    } else if (instruct->flag == ASTERPEND_CONTINUE) {
        argument_chain_link(car(instruct->instruction),
                            append(m->accumulator,
                                   car(cdr(instruct->instruction))),
                            m);

    } else if (instruct->flag == ARGUMENT) {
        table_put(instruct->instruction->in->symbol_id,
                  m->accumulator, m->current_frame->scope_to_be);

    } else if (instruct->flag == ELIPSIS_ARGUMENT) {
        // This MUST be modified. Continuations occuring during an e-argument will 
        // be totally clobbered when that argument is re-evaluated. Superbad.

        int sym = instruct->instruction->in->symbol_id;
        int i = 0;
        oyster *so_far = table_get(sym, m->current_frame->scope_to_be, &i);
        if (i) {
            oyster *new = append(so_far, list(1, m->accumulator));

            table_put(sym, new, m->current_frame->scope_to_be);

        } else {
            table_put(sym,
                      list(1, m->accumulator),
                      m->current_frame->scope_to_be);
        }


    } else if (instruct->flag == APPLY_FUNCTION) {
        // This is super ridiculous and must be trimmed. MUST.
        oyster *ins = instruct->instruction;
        incref(ins);
        frame *top = make_frame(NULL,
                                instruct->scope_to_be,
                                make_table(),
                                m->now,
                                car(ins),
                                EVALUATE);
        frame *cur = top;
        oyster *ins2 = cdr(ins);
        decref(ins);
        ins = ins2;
        incref(ins);
        while (!nilp(ins)) {
            cur->below = make_frame(NULL,
                                    instruct->scope_to_be,
                                    make_table(),
                                    m->now,
                                    car(ins),
                                    EVALUATE);
            cur = cur->below;
            incref(cur->below);

            oyster *ins2 = cdr(ins);
            decref(ins);
            ins = ins2;
            incref(ins);
        }
        decref(ins);
        cur->below = m->current_frame;
        m->current_frame = top;
        incref(top);
    }
}



// ---------------------- Convenience --------------------------//

#include "parsing.h"
#include "string.h"

oyster *evaluate_scan(GScanner * in, int print)
{
    oyster *func = next_oyster(in);
    machine *m = make_machine();
    add_builtins(m);
    incref(m);
    while (func) {
        frame *t = m->current_frame;
        m->current_frame = make_frame(t, 
                                      t->scope,
                                      make_table(), 
                                      t,
                                      func, 
                                      EVALUATE);
        incref(m->current_frame);
        decref(t);

        incref(func);
        while (!m->paused) {
            step_machine(m);
        }
        m->paused = 0;
        decref(func);
        func = next_oyster(in);
    }
    g_scanner_destroy(in);
    oyster *ret = m->accumulator;
    incref(ret);

    decref(m);
    return ret;
}


oyster *evaluate_string(char *str)
{
    return evaluate_scan(string_scanner(str), 0);
}

#endif
