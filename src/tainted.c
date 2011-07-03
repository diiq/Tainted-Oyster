#include "stdlib.h"
#include "oyster.h"
#include "oyster.c"
#include "machine.c"
#include "parsing.c"
#include "bindings.c"
#include "table.c"
#include "memory.c"
#include "builtins.c"

#include "stdio.h"


int main(int argc, char *argv[])
{
    init_oyster();

    char *filename = argv[1];

    oyster *ret = evaluate_scan(file_scanner(filename));

    oyster_print(ret);

    decref(ret);

    clean_up_oyster();

    return 0;

}
