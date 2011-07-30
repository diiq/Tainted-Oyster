#include "oyster.h"
#include "parsing.h"
#include <string.h>
#include <stdio.h>

void init_oyster()              // Where does this belong? Quo vadis, init?
{
    // This is the first call that should ever occur.
    static int a = 0;
    if (!a) {

        init_symbol_table();

        add_symbol(TYPE, "type");
        add_symbol(SYMBOL, "symbol");
        add_symbol(CONS, "cons");
        add_symbol(NIL, "()");
        add_symbol(LEAKED, "LEAKED");
        add_symbol(ATPEND, "unary-@");
        add_symbol(ELIPSIS, "...");
        add_symbol(ASTERIX, "unary-*");
        add_symbol(REALLY, "really");
        add_symbol(CLEAR, "clear");
        add_symbol(BUILT_IN_FUNCTION, "built-in-function");

        a = 1;
    }
}

void clean_up_oyster()          // And can clean_up come with you?
{
    // and this is the last call in any oyster program.
    free_symbol_table();
}

oyster *evaluate_file(FILE * inf, int print) // o god, it's a miscellaneous file
{
    token_stream *in = make_token_stream(inf);
    oyster *func = read(in);
    machine *m = make_machine();
    incref(m);
    while (func) {
        // oyster_print(func);printf("\n");

        push_new_instruction(m, func, EVALUATE);

        while (!m->paused) {
            step_machine(m);
        }
        m->paused = 0;

        decref(func);
        func = read(in);
    }
    oyster *ret = m->accumulator;
    incref(ret);

    fclose(inf);
    free_token_stream(in);
    decref(m);

    return ret;
}

oyster *evaluate_string(char *str)      // you disgust me, miscellany
{
    FILE *in = fmemopen(str, strlen(str), "r");
    return evaluate_file(in, 0);
}
