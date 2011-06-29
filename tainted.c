#include "stdlib.h"
#include "oyster.h"
#include "oyster.c"
#include "machine.c"
#include "parsing.c"
#include "bindings.c"
#include "table.c"

#include "stdio.h"


int main(int argc, char *argv[])
{
    init_oyster();

    char *filename = argv[1];

    evaluate_scan(file_scanner(filename));

    return 0;

}
