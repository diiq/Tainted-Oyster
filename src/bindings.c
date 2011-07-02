#ifndef BINDINGS
#define BINDINGS

#include "oyster.h"

oyster *leak()
{
    return make_symbol(LEAKED);
}

int leaked_p(oyster *x)
{
    return (x &&
            x->in->type == SYMBOL &&
            x->in->symbol_id == LEAKED);
}

table *binding_combine(table *a, table *b, 
                        table *newa, table *newb)
{
    table_ref(a);
    table_ref(b);

    table *ret = make_table();
    int key;
    oyster *avalue;
    oyster *bvalue;
    table_loop(key, avalue, a){
        int in_b = 0;
        bvalue = table_get(key, b, &in_b);
        if (in_b){
            if (bvalue == avalue){ // Do I mean equal()?
                table_put(key, avalue, ret);
            } else {
                table_put(key, avalue, newa);
                table_put(key, bvalue, newb);
            }
        } else {
            table_put(key, avalue, ret);
        }
    } table_end_loop;

    table_loop(key, bvalue, b){
        int in_a = 0;
        avalue = table_get(key, a, &in_a);
        if (!in_a){
            table_put(key, bvalue, ret);
        }
    } table_end_loop;

    table_unref(a);
    table_unref(b);

    return ret;
}

table *binding_union(table *a, table *b)
{
    table_ref(a);
    table_ref(b);

    table *ret = make_table();
    oyster *value;
    int key;
    table_loop(key, value, a){
        table_put(key, value, ret);
    } table_end_loop;
    table_loop(key, value, b){
        if(!leaked_p(value)){
            table_put(key, value, ret);
        }
    } table_end_loop;

    table_unref(a);
    table_unref(b);
    return ret;
}


//--------------------- Scope lookups -------------------------//

// I have in the past used cons-pairs or even lists to make this
// slightly neater (you can't destructively modify the scopey tables
// without refinding the scope from which it came.
oyster *look_up(int sym, machine *m)
{
    oyster *ret;
    frame *cur;
    int i = 0; 
    
    for(cur = m->current_frame; cur; cur = cur->below){
        ret = table_get(sym, cur->scope, &i);
        if (i && leaked_p(ret))
            continue;
        if (i)
            break;
    }
    
	if (!i){
	    ret = NULL; // hmmmm.
	}
    
	return ret;
}

// Not consing the entries in the scopes leaves me having to duplicate
// the whole procedure.

void set(int sym, oyster *val, machine *m, frame *f)
{
    oyster *ret;
    frame *cur;
    int i = 0; 
    
    for(cur = f; cur; cur = cur->below){
        ret = table_get(sym, cur->scope, &i);
        if (i && leaked_p(ret))
            continue;
        if (i)
            break;
    }


	if (!cur){ 
        cur = f;
	}

    table_put(sym, val, cur->scope);
}



#endif
