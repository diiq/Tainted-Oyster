#include "testing.h"
#include "oyster.h"
#include "table.h"
#include "stdio.h"

_test(new_table)
{
    table *tab = make_table();
    assert(tab);
    decref(tab);
} _tset;

_test(table_put_get)
{
    table *tab = make_table();
    table_put(2, make_symbol(4), tab);
    int i = 0;
    oyster *j = table_get(2, tab, &i);
    assert(i);
    assert(j->in->symbol_id == 4);

    table_put(2, make_symbol(3), tab);
    i = 0;
    j = table_get(2, tab, &i);
    assert(i);
    assert(j->in->symbol_id == 3);

    decref(tab);
} _tset;

_test(table_empty)
{
    table *tab = make_table();
    assert(table_empty(tab));
    table_put(2, make_symbol(1), tab);
    assert(!table_empty(tab));
    decref(tab);
} _tset;

_test(table_loop)
{
    table *tab = make_table();
    table_put(1, make_symbol(5), tab);
    table_put(4, make_symbol(4), tab);
    int i = 0;
    table_entry *entry;
    int key;
    table_loop(key, entry, tab->it) {
        assert((entry->it->in->symbol_id == 5 && key == 1) ||
               (entry->it->in->symbol_id == 4 && key == 4));
        i++;
    } table_end_loop;
    assert(i == 2);

    decref(tab);
} _tset;

_test(table)
{
    printf("\nTesting table:\n");
    run_test(new_table);
    run_test(table_put_get);
    run_test(table_empty);
    run_test(table_loop);
} _tset;
