#include "gc.h"
#include "parsing.c"
#include "testing.h"
#include "stdio.h"


int tests_run = 0;

_test(fail){
    parsed f = fail("hi", 0);
    assert(f.flag == 0);
    assert(f.objs == NULL);
    assert(f.index == 0);

}_tset;

_test(any){
    parser x = any();
    parsed a = x.parse("hi", 0, x);
    assert(a.flag == 1);
    assert(*(char*)(a.objs[0].x) == 'h');
    assert(a.index == 1);
}_tset;

_test(character){
    parser x = character('h');
    parsed a = x.parse("hi", 0, x);
    assert(a.flag == 1);
    assert(*(char*)(a.objs[0].x) == 'h');
    assert(a.index == 1);

    a = x.parse("whi", 0, x);
    assert(a.flag == 0);    
}_tset;

_test(one_of){
    parser x = one_of("abcde");
    parsed a = x.parse("chi", 0, x);
    assert(a.flag == 1);
    assert(*(char*)(a.objs[0].x) == 'c');
    assert(a.index == 1);

    a = x.parse("whi", 0, x);
    assert(a.flag == 0);    
}_tset;

_test(many){
    parser x = many(character('h'));
    parsed a = x.parse("hhhhhchi", 0, x);
    assert(a.flag == 1);
    assert(a.count == 5);
    assert(a.index == 5);

    a = x.parse("whi", 0, x);
    assert(a.flag == 0); 
} _tset;

_test(not){
    parser x = parse_not(character('w'));
    parsed a = x.parse("hchi", 0, x);
    assert(a.flag == 1);

    a = x.parse("whi", 0, x);
    assert(a.flag == 0); 
} _tset;

_test(sequence){
    parser *ps = NEW(2, parser);
    ps[0] = character('a');
    ps[1] = parse_not(character('b'));
    parser x = sequence(ps, 2);
    parsed a = x.parse("alice", 0, x);
    assert(a.flag == 1, "didn't succeed");
    assert(a.count == 1, "didn't count.");
    
    a = x.parse("abby", 0, x);
    assert(a.flag == 0, "succeeded?"); 
}_tset;



_test(first_of){
    parser *ps = NEW(2, parser);
    ps[0] = character('a');
    ps[1] = character('b');
    parser x = first_of(ps, 2);
    parsed a = x.parse("betty", 0, x);
    assert(a.flag == 1, "no b?");
    
    a = x.parse("sandy", 0, x);
    assert(a.flag == 0, "succeeded?"); 
}_tset;

_test(to_string){
    parser x = many(character('h'));
    parsed a = x.parse("hhhhchi", 0, x);
    assert(parseds_to_string(a.objs, a.count)[3] == 'h');
}_tset;


_test(symbol){
    parser symb = symbol();
    parsed sym = symb.parse("hello these are symbols", 0, symb);
    assert(((char *)sym.objs->x)[2] == 'l');
    parser wh = character(' ');
    sym = wh.parse("hello these are symbols", sym.index, wh);
    sym = symb.parse("hello these are symbols", sym.index, symb);
    assert(((char *)sym.objs->x)[2] == 'e');
}_tset;

int main(){
    GC_INIT();
    run_test(fail);
    run_test(any);
    run_test(character);
    run_test(one_of);
    run_test(many);    
    run_test(not);
    run_test(sequence);
    run_test(first_of);
    run_test(to_string);
    run_test(symbol);
    printf("PASSED\n");
    return 0;
}
