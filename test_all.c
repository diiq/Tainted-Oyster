#include "gc.h"
#include "testing.h"
#include "stdio.h"
#include "test_parsing.c"
#include "test_table.c"
#include "test_oysters.c"


int tests_run = 0;


int main(){
    GC_INIT();
    run_test(oyster);
    run_test(parsing);
    run_test(table);
    printf("\nPASSED!\n");
    return 0;
}
