#include <stdlib.h>
#include <stdio.h>

#include "oyster.h"


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
