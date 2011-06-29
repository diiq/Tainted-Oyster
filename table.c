
#ifndef TABLE
#define TABLE

#include "oyster.h"
#include <glib.h>

table *make_table()
{
    table *ret = NEW(table);
    ret->it = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
    return ret;
}

void *table_get(int key, table *tab, int *err)
{
    void *ret = g_hash_table_lookup(tab->it, GINT_TO_POINTER(key));
    *err = 1;
    if (ret == NULL){
        *err = 0;
    }
    return ret;
}

void table_put(int key, void *entry, table *tab)
{
    int *poo = GINT_TO_POINTER(key);
    g_hash_table_insert(tab->it, poo, entry);
}

int table_empty(table *tab)
{
    return g_hash_table_size(tab->it) == 0;
}


#endif