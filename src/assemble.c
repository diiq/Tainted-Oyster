#include "oyster.h"
#include "parsing.h"
#include <string.h>
#include <stdio.h>
#include <locale.h>

void init_oyster()              // Where does this belong? Quo vadis, init?
{
    setlocale(LC_CTYPE, "en_US.UTF-8"); // OH NO FAILS ON WINDOWS
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

oyster *evaluate_one(machine *m, oyster *func){
    incref(func);
    push_new_instruction(m, func, EVALUATE);
    
    while (!machine_paused(m)) {
        step_machine(m);
    }

    machine_unpause(m);
    decref(func);    
    return machine_accumulator(m);
}

oyster *evaluate_file(FILE * inf, int print) // o god, it's a miscellaneous file
{
    token_stream *in = make_token_stream(inf);
    oyster *func;
    oyster *ret;
    machine *m = make_machine();
    incref(m);
    while (func = read(in)) {
        ret = evaluate_one(m, func);
    }

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
