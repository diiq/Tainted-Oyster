#include "testing.h"
#include "stdio.h"

#include "oyster.h"
#include "memory.c"

#include "test_parsing.c"
#include "test_table.c"
#include "test_oysters.c"
#include "test_scopes.c" 
#include "test_machine.c"
#include "test_builtins.c"
#include "test_continuation.c"
#include "test_signals.c"

#include "printing.c"
#include "cons_tools.c"
#include "assemble.c"

int tests_run = 0;


int main()
{
    init_oyster();
    run_test(table);
    run_test(bindings);
    run_test(oyster);
    run_test(parsing);
    run_test(machine);
    run_test(builtins);
    run_test(continuations);
    clean_up_oyster();
    printf("\nPASSED!\n");
    return 0;
}
