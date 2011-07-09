#ifndef TABLE
#define TABLE

// This file contains functions that wrap the hash-tables of glib, to make them friendly
// to my memory managment, and replaceable --- they'll be the first with their backs
// against the wall when the revolution comes.

#include "oyster.h"
#include <glib.h>

table *make_table()
{
    table *ret = NEW(table);
    ret->ref = 0;
    ret->incref = &table_ref;
    ret->decref = &table_unref;

    ret->it =
        g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                              (GDestroyNotify) oyster_unref); 
 
   ret->leaked =
        g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL); 
 
   return ret;
}

oyster *table_get(int key, table * tab, int *flag)
{
    int leaked = GPOINTER_TO_INT(g_hash_table_lookup(tab->leaked, GINT_TO_POINTER(key)));
    if(leaked){
        *flag = 2;
        return NULL;
    }
    oyster *ret = g_hash_table_lookup(tab->it, GINT_TO_POINTER(key));
    if (ret == NULL) {
        *flag = 0;
        return NULL;
    }
    *flag = 1;
    return ret;
}

void table_put(int key, oyster * entry, table * tab)
{
    int *poo = GINT_TO_POINTER(key);
    incref(entry);
    g_hash_table_insert(tab->it, poo, entry);
}

// Leaks poke holes in the oyster shells; when a variable is bound to
// LEAKED in the current scope, the value from the scope *below* is used
// instead. The sea water seeps in, see? Don't seal the hole.

void leak(int sym, table *tab)
{
    g_hash_table_insert(tab->leaked, GINT_TO_POINTER(sym), GINT_TO_POINTER(1)); 
}

int table_empty(table * tab)
{
    return g_hash_table_size(tab->it) == 0;
}

void table_ref(table * x)
{
    x->ref++;
}

void table_unref(table * x)
{
    x->ref--;
    if (x->ref <= 0) {
        g_hash_table_unref(x->it);
        free(x);
    }
}

void table_print(table * x)
{
    int key;
    oyster *value;
    table_loop(key, value, x){
        printf("    %s : ", string_from_sym_id(key));
        oyster_print(value);
        printf("\n");
    } table_end_loop;
    printf("\n");
}
#endif
