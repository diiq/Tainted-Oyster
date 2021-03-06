#include <stdio.h>

#include "testing.h"
#include "oyster.h"
#include "machine.h"
#include "interpreter.h"
#include "frame.h"


_test(make_machine)
{
    machine *m = NULL;
    m = make_machine();
    assert(m);
    decref(m);
}

_tset;

_test(arg_list_tests)
{
    oyster *c = cons(make_symbol(ASTERIX), cons(make_symbol(55), nil()));
    assert(car_is_sym(c, ASTERIX));
}

_tset;

_test(argument_chain_link_boring)
{
    // Test boring lists:
    oyster *lambda_list =
        list(3, make_symbol(10), make_symbol(20), make_symbol(30));
    oyster *arg_list =
        list(3, make_symbol(30), make_symbol(40), make_symbol(50));

    machine *m = make_machine();
    argument_chain_link(lambda_list, arg_list, m);

    assert(m->current_frame->flag == EVALUATE);
    assert(symbol_id(m->current_frame->instruction) == 30);

    assert(m->current_frame->below->flag == ARGUMENT);
    assert(symbol_id(m->current_frame->below->instruction) == 10);

    assert(m->current_frame->below->below->flag == CONTINUE);

    decref(m);
}

_tset;


_test(argument_chain_link_atpend)
{
    // Atpending:
    oyster *lambda_list =
        list(3, make_symbol(10), make_symbol(20), make_symbol(30));
    oyster *arg_list =
        list(3, list(2, make_symbol(ATPEND), make_symbol(60)),
             make_symbol(40), make_symbol(50));

    machine *m = make_machine();
    argument_chain_link(lambda_list, arg_list, m);

    assert(m->current_frame->flag == EVALUATE);
    assert(symbol_id(m->current_frame->instruction) == 60);

    assert(m->current_frame->below->flag == ATPEND_CONTINUE);
    assert(oyster_type(m->current_frame->below->instruction) == CONS);

    decref(m);
}

_tset;

_test(argument_chain_link_asterpend)
{
    // Asterpending:
    oyster *lambda_list =
        list(3, make_symbol(10), make_symbol(20), make_symbol(30));
    oyster *arg_list =
        list(3, list(2, make_symbol(ASTERIX), make_symbol(30)),
             make_symbol(40), make_symbol(50));

    machine *m = make_machine();
    argument_chain_link(lambda_list, arg_list, m);

    assert(m->current_frame->flag == EVALUATE);
    assert(symbol_id(m->current_frame->instruction) == 30);

    assert(m->current_frame->below->flag == ASTERPEND_CONTINUE);
    assert(oyster_type(m->current_frame->below->instruction) == CONS);

    decref(m);
}

_tset;

_test(argument_chain_link_elipsis)
{
    // Elipsis args
    oyster *lambda_list =
        list(2, make_symbol(ELIPSIS),
             list(2, make_symbol(5), make_symbol(10)));
    oyster *arg_list = list(4, make_symbol(60), make_symbol(30),
                            make_symbol(40), make_symbol(50));

    machine *m = make_machine();
    argument_chain_link(lambda_list, arg_list, m);

    assert(m->current_frame->flag == EVALUATE, "flag");
    assert(oyster_type(m->current_frame->instruction) == CONS, "fcall");
    assert(symbol_id(cheap_car(m->current_frame->instruction)) ==
           5, "function");

    assert(m->current_frame->below->flag == ELIPSIS_ARGUMENT, "flag2");

    decref(m);
}

_tset;


_test(basic_step)
{
    machine *m = make_machine();
    oyster *fun = list(2,       // (((a) a) 'b) -> b
                       list(2,
                            make_symbol(CLEAR),
                            list(2,     // function
                                 list(1, make_symbol(TYPE)),    //lambda list
                                 make_symbol(TYPE))),   // expression
                       list(2,  // argument
                            make_symbol(CLEAR),
                            make_symbol(BUILT_IN_FUNCTION)));

    push_new_instruction(m, fun, EVALUATE);

    while (!m->paused) {
        step_machine(m);
    }
    assert(m->accumulator, "no answer");
    assert(oyster_type(m->accumulator) == SYMBOL, "Wrong answer type");
    assert(symbol_id(m->accumulator) == BUILT_IN_FUNCTION,
           "Wrong answer, %d", symbol_id(m->accumulator));

    decref(m);

}_tset;

_test(elipsis)
{
    oyster *ret = evaluate_string("((clear ((... foo) foo))"
                                  "    (clear foo) (clear (sipp))"
                                  "    (clear bar) (clear bash))\n");
    assert(oyster_type(ret) == CONS);
    decref(ret);
}_tset;

_test(atpend)
{
    oyster *ret = evaluate_string("((clear ((... foo) foo))"
                                  "    (unary-@ (clear (bar bash))))");
    assert(oyster_type(ret) == CONS);
    decref(ret);
}_tset;

_test(asterpend)
{
    oyster *ret = evaluate_string("((clear ((... foo) foo))"
                                  "    (unary-* (clear ((clear bar) (clear bash)))))");
    assert(oyster_type(ret) == CONS);
    decref(ret);
}

_tset;

_test(frame_stack)
{
    oyster *ret = evaluate_string("(((clear ((foo) (clear ((quiz) quiz))))"
                                  "    (clear bar)) (clear baz))");
    assert(oyster_type(ret) == SYMBOL &&
           symbol_id(ret) == sym_id_from_string("baz"));
    decref(ret);
}

_tset;

_test(fun_arg)
{
    oyster *ret = evaluate_string("((clear (((clear foo)) foo)) bar)");
    assert(oyster_type(ret) == SYMBOL &&
           symbol_id(ret) == sym_id_from_string("bar"));
    decref(ret);
}

_tset;

_test(no_arg)
{
    oyster *ret = evaluate_string("((clear (() (clear foo))))");
    assert(oyster_type(ret) == SYMBOL &&
           symbol_id(ret) == sym_id_from_string("foo"));
    decref(ret);
}

_tset;

_test(cons_from_cont)
{
    oyster *ret = evaluate_string("(cons (unary-' a) (unary-' b))");
    decref(ret);
}

_tset;

_test(machine)
{
    printf("\nTesting machine:\n");
    run_test(make_machine);

    run_test(arg_list_tests);
    run_test(argument_chain_link_boring);
    run_test(argument_chain_link_atpend);
    run_test(argument_chain_link_asterpend);
    run_test(argument_chain_link_elipsis);
    run_test(basic_step);
    run_test(elipsis);
    run_test(atpend);
    run_test(asterpend);
    run_test(frame_stack);
    run_test(fun_arg);
    run_test(no_arg);
    run_test(cons_from_cont);
}

_tset;
