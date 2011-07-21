#include "oyster.h"
#include "parsing.h"
#include "string.h"

void init_oyster()              // Where does this belong? Quo vadis, init?
{
    static int a = 0;
    if (!a) {

        init_symbol_table();

        add_symbol(TYPE, "type");
        add_symbol(SYMBOL, "symbol");
        add_symbol(CONS, "cons");
        add_symbol(NIL, "()");
        add_symbol(LEAKED, "LEAKED");
        add_symbol(ATPEND, "@");
        add_symbol(ELIPSIS, "...");
        add_symbol(ASTERIX, "*");
        add_symbol(JUST, "just");
        add_symbol(CLEAR, "clear");
        add_symbol(BUILT_IN_FUNCTION, "built-in-function");

        a = 1;
    }
}

void clean_up_oyster()          // And can clean_up come with you?
{
    free_symbol_table();
}




oyster *evaluate_scan(GScanner * in, int print) // o god, it's a miscellaneous file
{
    oyster *func = next_oyster(in);
    machine *m = make_machine();
    incref(m);

    while (func) {
        push_new_instruction(m, func, EVALUATE);

        while (!m->paused) {
            step_machine(m);
        }
        m->paused = 0;

        func = next_oyster(in);
    }

    g_scanner_destroy(in);
    oyster *ret = m->accumulator;

    incref(ret);
    decref(m);
    return ret;
}


oyster *evaluate_string(char *str) // you disgust me
{
    return evaluate_scan(string_scanner(str), 0);
}
