#include "stdio.h"

#include "testing.h"
#include "oyster.h"

#include "machine.c"


_test(make_machine){
    machine *m = NULL;
    m = make_machine();
    assert(m);
    decref(m);
}_tset;

_test(arg_list_tests){
    oyster *c  = cons(make_symbol(ASTERIX), cons(make_symbol(55), nil()));
    assert(car_is_sym(c, ASTERIX));
}_tset;

_test(argument_chain_link_boring){
    // Test boring lists:
    oyster *lambda_list = list(3, make_symbol(10), make_symbol(20), make_symbol(30));
    oyster *arg_list = list(3, make_symbol(30), make_symbol(40), make_symbol(50));

    instruction *i = argument_chain_link(lambda_list, arg_list, NULL);
 
    assert(i->flag == EVALUATE);
    assert(i->instruction->in->symbol_id == 30);
 
    assert(i->next->flag == ARGUMENT);
    assert(i->next->instruction->in->symbol_id == 10);

    assert(i->next->next->flag == CONTINUE);

    decref(i);
}_tset;


_test(argument_chain_link_atpend){
    // Atpending:
    oyster *lambda_list = list(3, make_symbol(10), make_symbol(20), make_symbol(30));
    oyster *arg_list = list(3, list(2, make_symbol(ATPEND), make_symbol(60)),
                            make_symbol(40), make_symbol(50));

    instruction *i = argument_chain_link(lambda_list, arg_list, NULL);

    assert(i->flag == EVALUATE);
    assert(i->instruction->in->symbol_id == 60);

    assert(i->next->flag == ATPEND_CONTINUE);
    assert(i->next->instruction->in->type == CONS);

    decref(i);
}_tset;

_test(argument_chain_link_asterpend){
    // Asterpending:
    oyster *lambda_list = list(3, make_symbol(10), make_symbol(20), make_symbol(30));
    oyster *arg_list = list(3, list(2, make_symbol(ASTERIX), make_symbol(30)),
                    make_symbol(40), make_symbol(50));

    instruction *i = argument_chain_link(lambda_list, arg_list, NULL);

    assert(i->flag == EVALUATE);
    assert(i->instruction->in->symbol_id == 30);

    assert(i->next->flag == ASTERPEND_CONTINUE);
    assert(i->next->instruction->in->type == CONS);

    decref(i);
}_tset;

_test(argument_chain_link_elipsis){
    // Elipsis args
    oyster *lambda_list = list(2, make_symbol(ELIPSIS), make_symbol(10));
    oyster *arg_list = list(4, make_symbol(60), make_symbol(30),
                    make_symbol(40), make_symbol(50));

    instruction *i = argument_chain_link(lambda_list, arg_list, NULL);

    assert(i->flag == EVALUATE, "flag");
    assert(i->instruction->in->symbol_id == 60, "symbol id");
    
    assert(i->next->flag == ELIPSIS_ARGUMENT, "flag2");
    assert(i->next->instruction->in->symbol_id == 10);
    
    assert(i->next->next->flag == CONTINUE);
    assert(i->next->next->instruction->in->type == CONS);

    decref(i);
}_tset;


_test(basic_step){
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
    decref(m->current_frame->current_instruction);
    m->current_frame->current_instruction = 
        make_instruction(fun, EVALUATE, NULL);

    while(!m->paused){
        step_machine(m);
    }
    assert(m->accumulator, "no answer");
    assert(m->accumulator->in->type == SYMBOL, "Wrong answer type");
    assert(m->accumulator->in->symbol_id == 157, "Wrong answer, %d",
           m->accumulator->in->symbol_id);

    decref(m);

}_tset;

_test(elipsis){
    oyster *ret = evaluate_string("((clear ((... foo) foo)) (clear foo) (clear (sipp)) (clear bar) (clear bash))");
    assert(ret->in->type == CONS);
    decref(ret);
}_tset;

_test(atpend){
    oyster *ret = evaluate_string("((clear ((... foo) foo)) (@ (clear (bar bash))))");
    assert(ret->in->type == CONS);
    decref(ret);
}_tset;

_test(asterpend){
    oyster *ret = evaluate_string("((clear ((... foo) foo)) (* (clear ((clear bar) (clear bash)))))");
    assert(ret->in->type == CONS);
    decref(ret);
}_tset;

_test(frame_stack){
    oyster *ret = evaluate_string("(((clear ((foo) (clear ((quiz) quiz)))) (clear bar)) (clear baz))");
    assert(ret->in->type == SYMBOL && 
           ret->in->symbol_id == sym_id_from_string("baz"));
    decref(ret);
}_tset;

_test(fun_arg){
    oyster *ret = evaluate_string("((clear (((clear foo)) foo)) bar)");
    assert(ret->in->type == SYMBOL && 
           ret->in->symbol_id == sym_id_from_string("bar"));
    decref(ret);
}_tset;

_test(no_arg){
    oyster *ret = evaluate_string("((clear (() (clear foo))))");
    assert(ret->in->type == SYMBOL && 
           ret->in->symbol_id == sym_id_from_string("foo"));
    decref(ret);
}_tset;

_test(machine){
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
}_tset;
