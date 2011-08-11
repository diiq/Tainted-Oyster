#include "stdio.h"


#include "oyster.h"
#include "testing.h"
#include "machine.h"
#include "frame.h"

_test(look_up)
{
    machine *m = make_machine();

    push_new_instruction(m, NULL, EVALUATE);

    table_put(2, make_symbol(2), m->base_frame->scope);
    oyster *ret = look_up(2, m->current_frame);
    assert(ret, "There was no return.");
    assert(symbol_id(ret) == 2, "It was the wrong symbol.");

    decref(m->current_frame->scope);
    m->current_frame->scope = make_table();
    incref(m->current_frame->scope);

    table_put(2, make_symbol(3), m->current_frame->scope);
    ret = look_up(2, m->current_frame);
    assert(ret, "There was no second return.");
    assert(symbol_id(ret) == 3,
           "The second return was the wrong symbol.");

    decref(m);
} _tset;


_test(set)
{
    machine *m = make_machine();
    push_new_instruction(m, NULL, EVALUATE);
    decref(m->current_frame->scope);
    m->current_frame->scope = make_table();
    incref(m->current_frame->scope);

    set(2, make_symbol(2), m->current_frame);
    oyster *ret = look_up(2, m->current_frame);
    assert(ret, "Not present");
    assert(symbol_id(ret) == 2, "Not 2.");

    table_put(2, make_symbol(1), m->current_frame->scope);
    ret = look_up(2, m->current_frame);
    assert(ret, "Not present 1a.");
    assert(symbol_id(ret) == 1, "Not 1. %d", symbol_id(ret));

    set(2, make_symbol(3), m->current_frame);
    ret = look_up(2, m->current_frame);
    assert(ret, "Not present 2.");
    assert(symbol_id(ret) == 3, "Not 3. %d", symbol_id(ret));

    decref(m);
} _tset;


_test(bindings)
{
    printf("\nTesting bindings:\n");
    run_test(look_up);
    run_test(set);
} _tset
