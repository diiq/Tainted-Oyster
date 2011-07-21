#ifndef INTERPRETER
#define INTERPRETER

int DEBUG = 0;

void step_machine(machine * m)
{
    frame *instruct = machine_pop_stack(m);
    if (DEBUG)
        machine_print(m);       // Should be some kind of debug-mode to turn this on.
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
        // This MUST be modified. Continuations occuring during an e-argument will 
        // be totally clobbered when that argument is re-evaluated. Superbad.
        // (Plus, its pretty damn inefficient, too .)

        {
            int sym = instruct->instruction->in->symbol_id;
            int i = 0;
            oyster *so_far =
                table_get(sym, m->current_frame->scope_to_be, &i);
            if (i) {
                oyster *new = append(so_far, list(1, m->accumulator));

                table_put(sym, new, m->current_frame->scope_to_be);

            } else {
                table_put(sym,
                          list(1, m->accumulator),
                          m->current_frame->scope_to_be);
            }
        }
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

void push_normal_argument(oyster * arg, oyster * lambda_list,
                          oyster * arg_list, machine * m)
{
    oyster *lambda, *name, *func;
    int flag;

    if (car_is_sym(lambda_list, ELIPSIS)) {
        lambda = car(cdr(lambda_list));
        lambda_list = cons(nil(), lambda_list);
        flag = ELIPSIS_ARGUMENT;

    } else {
        lambda = car(lambda_list);
        flag = ARGUMENT;
    }

    incref(lambda);

    if (lambda->in->type == CONS) {
        // The argument name is wrapped in a function call
        name = car(cdr(lambda));

        if (car_is_sym(arg, JUST)) {
            // but the argument is wrapped in JUST! Do nothing.
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

    push_argument(func, name, flag,
                  list(2, cdr(lambda_list), cdr(arg_list)), m);

    decref(lambda);
}


void argument_chain_link(oyster * lambda_list,
                         oyster * arg_list, machine * m)
{
    incref(lambda_list);
    incref(arg_list);

    oyster *arg = car(arg_list);
    incref(arg);

    if (nilp(lambda_list)) {
        // DO NOTHING!

    } else if (car_is_sym(lambda_list, ELIPSIS) && nilp(arg_list)) {
        // DO NOTHING!

    } else if (car_is_sym(arg, ASTERIX)) {
        // * arguments are lists of arguments that should be stiched in place.
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
