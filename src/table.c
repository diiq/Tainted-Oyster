#ifndef TABLE
#define TABLE

// This file contains functions that wrap the hash-tables of glib, to make them friendly
// to my memory managment, and replaceable --- they'll be the first with their backs
// against the wall when the revolution comes.

#include "oyster.h"
#include <glib.h>

void table_entry_ref(table_entry *x)
{
    x->ref++;
}

void table_entry_unref(table_entry *x)
{
    x->ref--;
    if(x->ref == 0){
        decref(x->it);
        free(x);
    }
}

table_entry *make_table_entry(oyster *it)
{
    table_entry *ret = NEW(table_entry);
    ret->incref = &table_entry_ref;
    ret->decref = &table_entry_unref;
    ret->ref = 0;
    ret->it = it;
    incref(it);
    return ret;
}

table *make_table()
{
    table *ret = NEW(table);
    ret->ref = 0;
    ret->incref = &table_ref;
    ret->decref = &table_unref;

    ret->it =
        g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                              (GDestroyNotify) table_entry_unref); 
 
   ret->leaked =
        g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL); 
 
   return ret;
}

oyster *table_get(int key, table * tab, int *flag)
{
    table_entry * ret = table_get_entry(key, tab, flag);
    if (ret) {
        if(!ret->it){
            *flag = 0;
            return NULL;
        }
        return ret->it;
    }
    return NULL;
}

table_entry *table_get_entry(int key, table * tab, int *flag)
{
    int leaked = GPOINTER_TO_INT(g_hash_table_lookup(tab->leaked, GINT_TO_POINTER(key)));
    if(leaked){
        *flag = 2; // magic number. fix, or use to produce rabbits.
        return NULL;
    }
    table_entry *ret = g_hash_table_lookup(tab->it, GINT_TO_POINTER(key));
    if (ret == NULL) {
        *flag = 0;
        return NULL;
    }
    *flag = 1;
    return ret;
}

void table_put(int key, oyster * entry, table * tab)
{
    table_entry *entree = make_table_entry(entry);
    table_put_entry(key, entree, tab);
}

void table_put_entry(int key, table_entry *entry, table *tab){
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


int leaked_p(int sym, table *tab)
{
    return GPOINTER_TO_INT(g_hash_table_lookup(tab->leaked, GINT_TO_POINTER(sym)));
}


void table_print(table * x)
{
    if(!x) {printf("NONE\n"); return;}
    int key;
    table_entry *value;
    table_loop(key, value, x->it){
        printf("    %s : ", string_from_sym_id(key));
        oyster_print(value->it);
        printf("\n");
    } table_end_loop;
    printf("\n");
}
#endif
