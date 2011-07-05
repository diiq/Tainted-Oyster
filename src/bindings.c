#ifndef BINDINGS
#define BINDINGS

// This file contains function dealing with scopes; 
// combining s., looking up in s., and leaking through s.

#include "oyster.h"


// Leaks poke holes in the oyster shells; when a variable is bound to
// LEAKED in the current scope, the value from the scope *below* is used
// instead. The sea water seeps in, see? Don't seal the hole.
 
int leaked_p(oyster *x)
{
    return (x &&
            x->in->type == SYMBOL &&
            x->in->symbol_id == LEAKED);
}

// The ability to sensibly combine two scopes is what makes oyster's
// scoping magic possible. It's worth understanding what's going on here.

table *binding_combine(table *a, table *b, 
                        table *newa, table *newb)
{
    incref(a);
    incref(b);

    // newa and newb are the new bindings of the component parts.
    table *ret = make_table(); // this is the binding for the combination;

    int key;
    oyster *avalue;
    oyster *bvalue;
    table_loop(key, avalue, a){
        int in_b = 0;
        // We must examine every symbol-value pair (slowslowslow)
        bvalue = table_get(key, b, &in_b);
        if (in_b){
            if (bvalue == avalue){ 
                // If the values are the same, move the pair to the larger scope.
                // This ensures that x <=> (cons (car x) (cdr x))  
                table_put(key, avalue, ret);
            } else {
                // A symbol that's bound differently must remain in the sub-bindings
                table_put(key, avalue, newa);
                table_put(key, bvalue, newb);
            }
        } else { // A pair only in one scope moves to the combined binding.
            table_put(key, avalue, ret);
        }
    } table_end_loop;


    table_loop(key, bvalue, b){
        int in_a = 0;
        avalue = table_get(key, a, &in_a);
        if (!in_a){ // A pair only in one scope moves to the combined binding.
            table_put(key, bvalue, ret);
        }
    } table_end_loop;

    decref(a);
    decref(b);

    return ret;
}

// The other binary operation, unioning, is much more obvious. 
// All conflicts are resolved in b's favor.
table *binding_union(table *a, table *b)
{
    incref(a);
    incref(b);

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

    decref(a);
    decref(b);
    return ret;
}

table *binding_copy(table *x)
{
    incref(x);

    table *ret = make_table();
    oyster *value;
    int key;
    table_loop(key, value, x){
        table_put(key, value, ret);
    } table_end_loop;

    decref(x);
    return ret;
}


//--------------------- Scope lookups -------------------------//

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
// the whole procedure in order to set. 

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


oyster *look_up_symbol(oyster *sym, machine *m)
{
    oyster *ret = look_up(sym->in->symbol_id, m);
    if (!ret) {
        printf("Error: apologies, dear sir, but I have no recollection of such a variable as %s\n", string_from_sym_id(sym->in->symbol_id));
        return NULL;
    }
    return ret;
}


#endif
