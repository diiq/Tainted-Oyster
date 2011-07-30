#include "stdlib.h"
#include "oyster.h"

#include "oyster.c"
#include "machine.c"
#include "interpreter.c"
#include "parsing.c"
#include "scopes.c"
#include "table.c"
#include "memory.c"
#include "builtins.c"
#include "continuations.c"
#include "signals.c"

#include "printing.c"
#include "cons_tools.c"
#include "assemble.c"


#include "stdio.h"


int main(int argc, char *argv[])
{
    init_oyster();

    FILE *file;
    int print;
    if (argc == 1) {
        file = stdin;
        print = 1;
    } else {
        file = fopen(argv[1], "r");
        print = 0;
    }

    /// g'damn g_scanner won't return a token until EOF is reached.
    /// all my trickery is for naught.
    oyster *ret = evaluate_file(file, print);
    decref(ret);


    clean_up_oyster();

    return 0;

}
