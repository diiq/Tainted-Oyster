#include "testing.h"
#include "stdio.h"
#include "test_parsing.c"
#include "test_table.c"
#include "test_oysters.c"
#include "test_bindings.c"
#include "test_machine.c"


int tests_run = 0;


int main(){
    run_test(oyster);
    run_test(parsing);
    run_test(table);
    run_test(bindings);
    run_test(machine);
    printf("\nPASSED!\n");
    free_symbol_table();
    return 0;
}
