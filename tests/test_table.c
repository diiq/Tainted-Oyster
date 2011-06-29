#include "table.c"
#include "testing.h"
#include "stdio.h"
 
_test(new_table){
    table *tab = make_table();
    assert(tab);
}_tset;

_test(table_put_get){
    table *tab = make_table();
    int *val = NEW(int);
    *val = 4;
    table_put(2, val, tab);
    int i = 0;
    int *j = table_get(2, tab, &i);
    assert(i);
    assert(*j == 4);

    int *val2 = NEW(int);
    *val2 = 3;
    table_put(2, val2, tab);
    i = 0;
    j = table_get(2, tab, &i);
    assert(i);
    assert(*j == 3);

    free(val);
    free(val2);
}_tset;

_test(table_empty){
    table *tab = make_table();
    assert(table_empty(tab));
    int j = 5;
    table_put(2, &j, tab);
    assert(!table_empty(tab));
}_tset;

_test(table_loop){
    table *tab = make_table();
    int j = 5;
    int k = 4;
    table_put(1, &j, tab);
    table_put(4, &k, tab);
    int i = 0;
    int *f;
    int q;
    table_loop(q, f, tab){
        assert((*f == 5 && q == 1) || (*f == 4 && q == 4), "Hmmm. %d, %d", *f, q);
        i++;
    } table_end_loop;
    assert(i == 2);
}_tset;
    
_test(table){
    printf("\nTesting table:\n");
    run_test(new_table);
    run_test(table_put_get);
    run_test(table_empty);
    run_test(table_loop);
}_tset;

