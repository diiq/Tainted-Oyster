#include "gc.h"
#include "parsing.c"
#include "testing.h"
#include "stdio.h"


int tests_run = 0;

void print_parsed(parsed x){
    printf("%d, %s, %d\n", x.flag, (char *)x.obj, x.index);
}

_test(fail){
    parsed f = fail("hi", 0);
    assert(f.flag == 0);
    assert(((char*)f.obj)[0] == 0);
    assert(f.index == 0);
}_tset;

_test(any){
    parser any = make_any();
    parsed a = any.parse("hi", 0, any);
    assert(a.flag == 1);
    assert(((char*)a.obj)[0] == 'h');
    assert(a.index == 1);
}_tset;

_test(character){
    parser x = make_character('h');
    parsed a = x.parse("hi", 0, x);
    assert(a.flag == 1);
    assert(((char*)a.obj)[0] == 'h');
    assert(a.index == 1);

    a = x.parse("whi", 0, x);
    assert(a.flag == 0);    
}_tset;

_test(one_of){
    parser x = make_one_of("abcde");
    parsed a = x.parse("chi", 0, x);
    assert(a.flag == 1);
    assert(((char*)a.obj)[0] == 'c');
    assert(a.index == 1);

    a = x.parse("whi", 0, x);
    assert(a.flag == 0);    
}_tset;

_test(many){
    parser y = make_character('h');
    parser x = make_many(y);
    parsed a = x.parse("hhhhhchi", 0, x);
    assert(a.flag == 1);
    assert(a.size == 5);
    assert(a.index == 5);

    a = x.parse("whi", 0, x);
    assert(a.flag == 0); 
} _tset;

int main(){
    GC_INIT();
    run_test(fail);
    run_test(any);
    run_test(character);
    run_test(one_of);
    run_test(many);
    printf("PASSED\n");
    return 0;
}
