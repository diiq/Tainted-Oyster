// Testing framework. 

#define assert(test, ...) do { if (!(test)) {        \
            printf("FAILED %s:\n    ", testname);       \
            printf("\n"__VA_ARGS__); printf("\n\n"); return 1;}} while (0)

#define run_test(test) do { int fail = test_##test(); tests_run++;  \
        if (fail) return fail; } while (0)

#define _test(name) int test_##name() {     \
    char *testname = #name;                    
    
#define _tset if(testname)return 0;return 0;}

#define _TEST

extern int tests_run;
