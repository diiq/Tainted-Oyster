#ifndef BUILTINS
#define BUILTINS

// This file contains basic builtin functions like cons, car, cdr, quote, and so on.

#include "oyster.h"
#include "parsing.h"

#define ARG(a) oyster *a = look_up(sym_id_from_string(#a), m)
#define sARG(a, an) oyster *a = look_up(sym_id_from_string(an), m)

// ------------------ The Functions -------------------//

oyster *builtin_cons(machine *m){
    ARG(car);
    ARG(cdr);
    return cons(car, cdr);
}

oyster *builtin_car(machine *m){
    ARG(cons);
    return car(cons);
}

oyster *builtin_cdr(machine *m){
    ARG(cons);
    return cdr(cons);
}

oyster *builtin_current_scope(machine *m){
    oyster *ret = make_oyster(sym_id_from_string("table"));
    ret->in->value = m->current_frame->below->scope;
    incref(ret->in->value);
    return ret;
}

oyster *builtin_table_get(machine *m){
    // in put val id ay shun
    sARG(tab, "table");
    ARG(sym);
    int i = 0;
    oyster *ret = table_get(sym->in->symbol_id, tab->in->value, &i);
    if (!i) ret = nil();
    return list(1, ret);
}



// ------------------ The Apparatus -------------------//



oyster *make_builtin(oyster *(*func)(machine *m)){
    oyster *ret = make_oyster(BUILT_IN_FUNCTION);
    ret->in->built_in = func;
    return ret;
}

oyster *arg(char *name){
    return make_symbol(sym_id_from_string(name));
}

void add_builtin(char *name, oyster *lambda_list, oyster *(*func)(machine *m), machine *m)
{
    oyster *ofunc = list(2, lambda_list, make_builtin(func));
    set(sym_id_from_string(name), ofunc, m, m->current_frame); // should be base frame later.
}

void add_builtins(machine *m){
    add_builtin("cons", list(2, arg("car"), arg("cdr")), builtin_cons, m);
    add_builtin("car", list(1, arg("cons")), builtin_car, m);
    add_builtin("cdr", list(1, arg("cons")), builtin_cdr, m);
    add_builtin("current-scope", nil(), builtin_current_scope, m);
    add_builtin("table-get", list(2, arg("sym"), arg("table")), builtin_current_scope, m);
}






#endif
