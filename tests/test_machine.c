#include "stdio.h"

#include "testing.h"
#include "oyster.h"

#include "machine.c"


_test(make_machine){
    machine *m = NULL;
    m = make_machine();
    assert(m);
}_tset;

_test(arg_list_tests){
    assert(car_is_sym(cons(make_symbol(ASTERIX), nil), ASTERIX));
}_tset;

_test(argument_chain_link){
    // Test boring lists:
    oyster *lambda_list = list(3, make_symbol(10), make_symbol(20), make_symbol(30));
    oyster *arg_list = list(3, make_symbol(30), make_symbol(40), make_symbol(50));
    instruction *i = argument_chain_link(lambda_list, arg_list, NULL);
    assert(i->flag == EVALUATE);
    assert(i->instruction->in->symbol_id == 30);
    i = i->next;
    assert(i->flag == ARGUMENT);
    assert(i->instruction->in->symbol_id == 10);
    i = i->next;
    assert(i->flag == CONTINUE);

    // Atpending:
    arg_list = list(2, list(2, make_symbol(ATPEND), make_symbol(30)), 
                    make_symbol(40), make_symbol(50));
    i = argument_chain_link(lambda_list, arg_list, NULL);
    assert(i->flag == EVALUATE);
    assert(i->instruction->in->symbol_id == 30);
    i = i->next;
    assert(i->flag == ATPEND_CONTINUE);
    assert(i->instruction->in->type == CONS);

    // Asterpending:
    arg_list = list(2, list(2, make_symbol(ASTERIX), make_symbol(30)), 
                    make_symbol(40), make_symbol(50));
    i = argument_chain_link(lambda_list, arg_list, NULL);
    assert(i->flag == EVALUATE);
    assert(i->instruction->in->symbol_id == 30);
    i = i->next;
    assert(i->flag == ASTERPEND_CONTINUE);
    assert(i->instruction->in->type == CONS);

    // Atpending:
    arg_list = list(4, make_symbol(60), make_symbol(30), 
                    make_symbol(40), make_symbol(50));
    lambda_list = list(2, make_symbol(ELIPSIS), make_symbol(10));
    i = argument_chain_link(lambda_list, arg_list, NULL);
    assert(i->flag == EVALUATE, "flag");
    assert(i->instruction->in->symbol_id == 60, "symbol id");
    i = i->next;
    assert(i->flag == ELIPSIS_ARGUMENT, "flag2");
    assert(i->instruction->in->symbol_id == 10);
    i = i->next;
    assert(i->flag == CONTINUE);
    assert(i->instruction->in->type == CONS);

}_tset;

_test(basic_step){
    init_oyster();
    machine *m = make_machine();
    oyster *fun = list(2, // (((a) a) b) -> b
                       list(2,
                            make_symbol(CLEAR),
                            list(2, // function
                                 list(1, make_symbol(55)), //lambda list
                                 make_symbol(55))), // expression
                       list(2, // argument
                            make_symbol(CLEAR),
                            make_symbol(157)));
    m->current_frame->current_instruction = 
        make_instruction(fun, EVALUATE, NULL);
    while(!m->paused){
        step_machine(m);
    }
    assert(m->accumulator, "no answer");
    assert(m->accumulator->in->type == SYMBOL, "Wrong answer type");
    assert(m->accumulator->in->symbol_id == 157, "Wrong answer, %d",
           m->accumulator->in->symbol_id == 157);
}_tset;

_test(elipsis){
    init_oyster();
    oyster *ret = evaluate_string("((clear ((... foo) foo)) (clear bar) (clear bash))");
    assert(ret->in->type == CONS);
}_tset;

_test(atpend){
    init_oyster();
    oyster *ret = evaluate_string("((clear ((... foo) foo)) (@ (clear (bar bash))))");
    assert(ret->in->type == CONS);
}_tset;

_test(asterpend){
    init_oyster();
    oyster *ret = evaluate_string("((clear ((... foo) foo)) (* (clear ((clear bar) (clear bash)))))");
    assert(ret->in->type == CONS);
}_tset;

_test(frame_stack){
    init_oyster();
    oyster *ret = evaluate_string("(((clear ((foo) (clear ((quiz) quiz)))) (clear bar)) (clear baz))");
    assert(ret->in->type == SYMBOL && 
           ret->in->symbol_id == sym_id_from_string("baz"));
}_tset;

_test(fun_arg){
    init_oyster();
    oyster *ret = evaluate_string("((clear (((clear foo)) foo)) bar)");
    assert(ret->in->type == SYMBOL && 
           ret->in->symbol_id == sym_id_from_string("bar"));
    // free_symbol_table();
}_tset;

_test(machine){
    printf("\nTesting machine:\n");
    run_test(make_machine);
    run_test(arg_list_tests);
    run_test(argument_chain_link);
    run_test(basic_step);
    run_test(elipsis);
    run_test(atpend);
    run_test(asterpend);
    run_test(frame_stack);
    run_test(fun_arg);
}_tset;
