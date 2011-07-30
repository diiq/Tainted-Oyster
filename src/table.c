// This file contains functions that wrap the hash-tables of glib, to
// make them friendly to my memory managment, and to make them
// replaceable --- they'll be the first with their backs against the wall
// when the revolution comes.

#ifndef TABLE
#define TABLE

#include "oyster.h"
#include <glib.h>



table_entry *make_table_entry(oyster * it)
{
    table_entry *ret = NEW(table_entry);
    ret->it = it;
    incref(it);
    return ret;
}



table *make_table()
{
    table *ret = NEW(table);
    ret->it = g_hash_table_new_full(g_direct_hash,
                                    g_direct_equal, NULL, decref);

    ret->leaked = g_hash_table_new_full(g_direct_hash,
                                        g_direct_equal, NULL, NULL);
    return ret;
}

table *table_copy(table * t)
{
    table *ret = make_table();
    table_entry *entry;
    int k;
    table_loop(k, entry, t->it) {
        table_put_entry(k, entry, ret);
    } table_end_loop;
    return ret;
}


void table_put_entry(int key, table_entry * entry, table * tab)
{
    g_hash_table_insert(tab->it, GINT_TO_POINTER(key), entry);
    incref(entry);
}

table_entry *table_get_entry(int key, table * tab, int *flag)
{
    // Check if symbol is leaked.
    if (GPOINTER_TO_INT(g_hash_table_lookup(tab->leaked,
                                            GINT_TO_POINTER(key)))) {
        *flag = TABLE_ENTRY_LEAKED;
        return NULL;
    }
    // otherwise, grab it.
    table_entry *ret = g_hash_table_lookup(tab->it, GINT_TO_POINTER(key));
    if (ret == NULL) {
        *flag = TABLE_ENTRY_NOT_FOUND;
        return NULL;
    }
    *flag = TABLE_ENTRY_FOUND;
    return ret;
}


void table_put(int key, oyster * entry, table * tab)
{
    table_entry *entree = make_table_entry(entry);
    table_put_entry(key, entree, tab);
}

oyster *table_get(int key, table * tab, int *flag)
{
    table_entry *ret = table_get_entry(key, tab, flag);
    if (ret) {
        // Due to the reification of leaks, it is possible to have an
        // entry with no oyster inside.
        if (!ret->it) {
            *flag = TABLE_ENTRY_NOT_FOUND;
            return NULL;
        }
        return ret->it;
    }
    return NULL;
}



int table_empty(table * tab)
{
    return tab && g_hash_table_size(tab->it) == 0;
}



// Leaks poke holes in the oyster shells; when a variable is bound to
// LEAKED in the current scope, the value from the scope *below* is used
// instead. The sea water seeps in, see? Don't seal the hole.

void leak(int sym, table * tab)
{
    g_hash_table_insert(tab->leaked, GINT_TO_POINTER(sym),
                        GINT_TO_POINTER(1));
}

int leaked_p(int sym, table * tab)
{
    return
        GPOINTER_TO_INT(g_hash_table_lookup
                        (tab->leaked, GINT_TO_POINTER(sym)));
}



#endif
