#include "oyster.h"
#include "bindings.c"
#include "testing.h"
#include "stdio.h"

_test(binding_combine){
    table *a = make_table();
    table *b = make_table();
    oyster *match = make_symbol(1);
    table_put(2, match, a);
    table_put(2, match, b); // shared
    table_put(3, make_symbol(2), a);
    table_put(3, make_symbol(3), b); // conflicting
    table_put(4, make_symbol(4), a);
    table_put(5, make_symbol(5), b); // individual

    table *newa = make_table();
    table *newb = make_table();
    table *comb = binding_combine(a, b, newa, newb);

    int i = 0;
    oyster *ret;
    
    // shared:
    table_get(2, newa, &i);
    assert(!i, "2 present in newa");

    table_get(2, newb, &i);
    assert(!i);

    ret = table_get(2, comb, &i);
    assert(i);
    assert(ret->in->symbol_id == 1);

    // conflicing:
    ret = table_get(3, newa, &i);
    assert(i);
    assert(ret->in->symbol_id == 2);

    ret = table_get(3, newb, &i);
    assert(i);
    assert(ret->in->symbol_id == 3);

    table_get(3, comb, &i);
    assert(!i);

    // individual:
    table_get(4, newa, &i);
    assert(!i, "present in a");

    table_get(5, newb, &i);
    assert(!i, "present in b");

    ret = table_get(4, comb, &i);
    assert(i, "not there");
    assert(ret->in->symbol_id == 4, "not 4");

    ret = table_get(5, comb, &i);
    assert(i, "5 not there");
    assert(ret->in->symbol_id == 5, "not 5");
    
    decref(newa);
    decref(newb);
    decref(comb);

}_tset

_test(binding_union){
    table *a = make_table();
    table *b = make_table();
    table_put(3, make_symbol(2), a);
    table_put(3, make_symbol(3), b); // conflicting
    table_put(4, make_symbol(1), a);
    table_put(5, make_symbol(2), b); // individual
    table *u = binding_union(a, b);
    
    int i = 0;
    oyster *ret;

    ret = table_get(3, u, &i);
    assert(i);
    assert(ret->in->symbol_id == 3, "a");
    
    ret = table_get(4, u, &i);
    assert(i, "b1");
    assert(ret->in->symbol_id == 1, "b");

    ret = table_get(5, u, &i);
    assert(i, "c1");
    assert(ret->in->symbol_id == 2, "c");

    decref(u);
    
}_tset;

_test(look_up){
    machine *m = make_machine();

    set_current_frame(m, make_frame(make_table(), m->current_frame));
        
    table_put(2, make_symbol(2), m->base_frame->scope);
    oyster *ret = look_up(2, m);
    assert(ret);
    assert(ret->in->symbol_id == 2);

    table_put(2, make_symbol(3), m->current_frame->scope);
    ret = look_up(2, m);
    assert(ret);
    assert(ret->in->symbol_id == 3);

    decref(m);
}_tset;


_test(set){
    machine *m = make_machine();
    set_current_frame(m, make_frame(make_table(), m->current_frame));

    set(2, make_symbol(2), m, m->current_frame);
    oyster *ret = look_up(2, m);
    assert(ret, "Not present");
    assert(ret->in->symbol_id == 2, "Not 2.");

    table_put(2, make_symbol(1), m->current_frame->scope);
    ret = look_up(2, m);
    assert(ret, "Not present 1a.");
    assert(ret->in->symbol_id == 1, "Not 1. %d", ret->in->symbol_id);

    set(2, make_symbol(3), m, m->current_frame);
    ret = look_up(2, m);
    assert(ret, "Not present 2.");
    assert(ret->in->symbol_id == 3, "Not 3. %d", ret->in->symbol_id);

    decref(m);
}_tset;


_test(bindings){
    printf("\nTesting bindings:\n");
    run_test(binding_combine);
    run_test(binding_union);
    run_test(look_up);
    run_test(set);
}_tset
