
// This file handles tables specifically as they apply to scopes
// in stack frames.

#include "oyster.h"
#include "machine.h"

table_entry *look_up_entry(int sym, frame * cur)
{
    int i;
    return table_get_entry(sym, cur->scope, &i);
}

oyster *look_up(int sym, frame * cur)
{
    table_entry *e = look_up_entry(sym, cur);
    if (e)
        return e->it;
    return NULL;
}

void set(int sym, oyster * val, frame * f)
{
    table_entry *ret = look_up_entry(sym, f);

    if (ret) {
        oyster *t = ret->it;
        ret->it = val;
        incref(val);
        decref(t);
    } else {
        table_put(sym, val, f->scope);
    }
}



table *reify_scope(table * t, frame * f)
{
    table *ret = table_copy(t);
    table_entry *entry;
    int k;
    table_loop(k, entry, t->leaked) {
        table_entry *real = look_up_entry(k, f);
        if (real) {
            table_put_entry(k, real, ret);
        }
    }
    table_end_loop;
    return ret;
}


