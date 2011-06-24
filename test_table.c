#include "gc.h"
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
    int *j = (int *)table_get(2, tab, &i);
    assert(i);
    assert(*j == 4);
}_tset;
    
_test(table_rev_lookup){
    int eq(void *i, void *j){
        return (*(int *)i == *(int *)j);
    }
    table *tab = make_table();
    int *val = NEW(int);
    *val = 4;
    table_put(2, val, tab);
    int i = 0;
    int q = 4;
    int j = table_rev_lookup(&q, eq, tab, &i);
    assert(i);
    assert(j == 2);
}_tset;

_test(table_remove){
    table *tab = make_table();
    int *val = NEW(int);
    *val = 4;
    table_put(2, val, tab);
    table_remove(2, tab);
    int i = 4;
    table_get(2, tab, &i);
    assert(i == 0);
}_tset;

_test(table_empty){
    table *tab = make_table();
    assert(table_empty(tab));
    int j = 5;
    table_put(2, &j, tab);
    assert(!table_empty(tab));
}_tset;

_test(table){
    printf("\nTesting table:\n");
    run_test(new_table);
    run_test(table_put_get);
    run_test(table_rev_lookup);
    run_test(table_remove);
    run_test(table_empty);
}_tset;

