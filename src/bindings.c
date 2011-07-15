#ifndef BINDINGS
#define BINDINGS

// This file contains function dealing with scopes; 
// combining s., looking up in s., and leaking through s.

#include "oyster.h"


//--------------------- Scope lookups -------------------------//

table_entry *look_up_entry(int sym, frame *cur)
{
    table_entry *ret;
    int i = 0;

    ret = table_get_entry(sym, cur->scope, &i);
    if (i == 2)
        ret = table_get_entry(sym, cur->scope_below, &i);

    return ret;
}

oyster *look_up(int sym, frame *cur)
{
    table_entry *e = look_up_entry(sym, cur);
    if (e)
        return e->it;
    return NULL;
}

// Not consing the entries in the scopes leaves me having to duplicate
// the whole procedure in order to set. 

void set(int sym, oyster * val, frame * f)
{
    table_entry *ret;
    int i = 0;

    ret = table_get_entry(sym, f->scope, &i);
    if (i == 2)
        ret = table_get_entry(sym, f->scope_below, &i);

    if (i == 1){
        ret->it = val;
        incref(val);

    } else { 
        table_put(sym, val, f->scope);
    }    
}


table *table_copy(table *t)
{
    table *ret = make_table();
    table_entry *entry;
    int k;
    table_loop(k, entry, t->it){
        table_put_entry(k, entry, ret);
    } table_end_loop;
    return ret;
}

table *reify_scope(table *t, frame *f)
{
    table *ret = table_copy(t);
    table_entry *entry;
    int k;
    table_loop(k, entry, t->leaked){
        table_entry *real = look_up_entry(k, f);
        if (real) {
            table_put_entry(k, real, ret);
        }
    } table_end_loop;
    return ret;
}







//------------------ Convenience ------------------------//


oyster *look_up_symbol(oyster * sym, frame * f)
{
    oyster *ret = look_up(sym->in->symbol_id, f);
    if (!ret) {
        printf("Error: apologies, dear sir, "
               "but I have no recollection of such a variable "
               "as %s\n",
             string_from_sym_id(sym->in->symbol_id));
        return NULL;
    }
    return ret;
}


#endif
