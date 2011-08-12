
// This file contains basic builtin functions like cons, car, cdr, quote, and so on.

#include "oyster.h"
#include "parsing.h"
#include "machine.h"

// ------------------ The Apparatus -------------------//

oyster *make_builtin(oyster * (*func) (machine * m))
{
    oyster *ret = make_oyster(BUILT_IN_FUNCTION);
    oyster_assign_value(ret, func);
    oyster_assign_gc(ret, 0);
    return ret;
}

oyster *arg(char *name)
{
    return make_symbol(sym_id_from_string(name));
}

oyster *unev(char *name)
{
    return list(2, make_symbol(CLEAR), arg(name));
}

oyster *quot(char *name)
{
    return list(2, arg("unary-'"), arg(name));
}

void add_builtin(char *name,
                 oyster * lambda_list,
                 oyster * (*func) (machine * m), machine * m)
{
    oyster *ofunc = list(2, lambda_list, make_builtin(func));
    set(sym_id_from_string(name), ofunc, m->base_frame);
}



// ------------------ The Functions -------------------//
// TODO OMGS: in put val id ay shun

oyster *builtin_cons(machine * m)
{
    ARG(car);
    ARG(cdr);
    return cons(car, cdr);
}

oyster *builtin_car(machine * m)
{
    ARG(cons);
    if (oyster_type(cons) != CONS) {
        oyster *signal = list(2, arg("car-of-wrong-type"), cons);
        toss_signal(make_signal(signal, m), m);
        return NULL;
    }
    return car(cons);
}

oyster *builtin_cdr(machine * m)
{
    ARG(cons);
    if (oyster_type(cons) != CONS) {
        oyster *signal = list(2, arg("cdr-of-wrong-type"), cons);
        toss_signal(make_signal(signal, m), m);
        return NULL;
    }
    return cdr(cons);
}

oyster *builtin_assign_cdr(machine *m){
    ARG(cons);
    ARG(value);
    if (oyster_type(cons) != CONS) {
        oyster *signal = list(2, arg("set-cdr-of-wrong-type"), cons);
        toss_signal(make_signal(signal, m), m);
        return NULL;
    }
    set_cdr(cons, value);
    return value;
}

oyster *builtin_assign(machine * m)
{
    ARG(symbol);
    ARG(value);
    if(oyster_bindings(symbol)){
        table_insert_or_replace(symbol_id(symbol), value, oyster_bindings(symbol));
    } else {
        oyster *signal = list(2, arg("There is no scope!"), cons);
        toss_signal(make_signal(signal, m), m);
        return NULL;
    }        
    return value;
}

oyster *builtin_leak(machine * m)
{
    // Gross.
    ARG(symbol);
    ARG(closure);
    ARG(value);
    frame *current = m->current_frame;

    int id = symbol_id(symbol);
    if (nilp(closure)) {
        int i = 0;
        table_entry *a =
            table_get_entry(id, frame_scope_below(current), &i);

        if (i) {
            table_put_entry(id, a, frame_scope(current));
        } else {
            table_put(id, NULL, frame_scope(current));
            table_put_entry(id, table_get_entry(id, frame_scope(current), &i), frame_scope_below(current));
        }

    } else if (nilp(value)){
        if (!oyster_bindings(closure)) {
            oyster_assign_bindings(closure, make_table());
            incref(oyster_bindings(closure));
        }
        leak(id, oyster_bindings(closure));

    } else {
        table_put(symbol_id(symbol), value, oyster_bindings(closure));
    }

    return closure;

}

oyster *builtin_leak_all(machine *m)
{
    ARG(obj);
    return oyster_copy(obj, NULL);
}

oyster *builtin_bindings(machine *m)
{
    ARG(obj);
    oyster *ret = make_oyster(sym_id_from_string("table"));
    oyster_assign_value(ret, oyster_bindings(obj));
    incref(oyster_bindings(obj));
    return ret;
}

oyster *builtin_assign_bindings(machine *m)
{
    ARG(obj);
    ARG(bindings);
    oyster_assign_bindings(obj, oyster_value(bindings));
    incref(oyster_bindings(obj));
    return obj;
}

oyster *builtin_quote(machine * m)
{
    ARG(x);
    if(!oyster_bindings(x) || table_empty(oyster_bindings(x)))
        return oyster_copy(x, frame_scope_below(machine_active_frame(m)));
    return oyster_copy(x, oyster_bindings(x));
}

oyster *builtin_atom_p(machine * m)
{
    ARG(x);
    if (oyster_type(x) == CONS)
        return nil();
    return x;
}

oyster *builtin_is(machine * m)
{
    ARG(a);
    ARG(b);
    if (oyster_type(a) == oyster_type(b) && oyster_value(a) == oyster_value(b))
        return make_symbol(sym_id_from_string("t"));
    return nil();
}


oyster *builtin_oif(machine * m)
{
    ARG(test);
    ARG(then);
    sARG(el, "else");
    if (!nilp(test)) {
        push_new_instruction(m, then, EVALUATE);
    } else {
        push_new_instruction(m, el, EVALUATE);
    }
    return NULL;
}

oyster *builtin_print(machine * m)
{
    // This gets a little silly...
    ARG(xs);
    incref(xs);
    oyster *ret = car(xs);
    do {
        oyster_print(car(xs));
        printf(" ");
        oyster *nxs = cdr(xs);
        incref(nxs);
        decref(xs);
        xs = nxs;
    } while (!nilp(xs));
    decref(xs);
    printf("\n");
    return ret;
}

oyster *builtin_signal(machine * m)
{
    ARG(message);
    toss_signal(make_signal(message, m), m);
    return NULL;
}

oyster *builtin_with_signal_handler(machine * m)
{
    ARG(handler);
    ARG(code);
    push_new_instruction(m, handler, HANDLE_SIGNALS);
    push_new_instruction(m, code, EVALUATE);
    incref(handler);
    return handler;
}

oyster *builtin_current_scope(machine * m)
{
    oyster *ret = make_oyster(sym_id_from_string("table"));
    oyster_assign_value(ret, frame_scope(m->current_frame));
    incref(oyster_value(ret));
    return ret;
}

oyster *builtin_info_table(machine * m)
{
    ARG(obj);
    oyster *ret = make_oyster(sym_id_from_string("table"));
    oyster_assign_value(ret, oyster_info(obj));
    incref(oyster_value(ret));
    return ret;
}

oyster *builtin_table_get(machine * m)
{
    ARG(symbol);
    sARG(tab, "table");
    int i = 0;
    oyster *ret = table_get(symbol_id(symbol), oyster_value(tab), &i);
    if (!i)
        return nil();
    return ret;
}

oyster *builtin_table_assign(machine * m)
{
    ARG(symbol);
    ARG(value);
    sARG(tab, "table");
    table_put(symbol_id(symbol), value, oyster_value(tab));
    return value;
}

oyster *builtin_new(machine *m)
{
    ARG(obj);
    oyster *ret = make_oyster(oyster_type(obj));
    oyster_assign_value(ret, oyster_value(obj));
    return ret;
}

void add_builtins(machine * m)
{
    add_builtin("cons", list(2, arg("car"), arg("cdr")), builtin_cons, m);
    add_builtin("car", list(1, arg("cons")), builtin_car, m);
    add_builtin("cdr", list(1, arg("cons")), builtin_cdr, m);

    add_builtin("assign-cdr", list(2, arg("cons"), arg("value")), builtin_assign_cdr, m);

    add_builtin("assign", list(2, quot("symbol"), arg("value")), builtin_assign,
                m);
    add_builtin("leak", list(3, unev("symbol"), arg("closure"), arg("value")),
                builtin_leak, m);
    add_builtin("leak-all", list(1, arg("obj")),
                builtin_leak_all, m);
    add_builtin("bindings", list(1, arg("obj")),
                builtin_bindings, m);
    add_builtin("assign-bindings", list(2, arg("obj"), arg("bindings")),
                builtin_assign_bindings, m);


    add_builtin("unary-'", list(1, unev("x")), builtin_quote, m);

    add_builtin("atom", list(1, arg("x")), builtin_atom_p, m);
    add_builtin("is", list(2, arg("a"), arg("b")), builtin_is, m);
    add_builtin("oif", list(3, arg("test"), quot("then"), quot("else")),
                builtin_oif, m);

    add_builtin("print", list(2, arg("..."), arg("xs")), builtin_print, m);

    add_builtin("signal", list(1, arg("message")), builtin_signal, m);
    add_builtin("with-signal-handler",
                list(2, quot("handler"),
                     quot("code")), builtin_with_signal_handler, m);

    add_builtin("current-scope", nil(), builtin_current_scope, m);

    add_builtin("info-table", list(1, arg("obj")), builtin_info_table, m);

    add_builtin("table-get", list(2, quot("symbol"), arg("table")),
                builtin_table_get, m);

    add_builtin("table-assign", list(3, quot("symbol"), arg("table"), arg("value")),
                builtin_table_assign, m);

    add_builtin("new", list(1, arg("obj")), builtin_new, m);

}

