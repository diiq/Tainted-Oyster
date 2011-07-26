#include "oyster.h"
#include "parsing.c"
#include "testing.h"
#include "stdio.h"

extern int tests_run;

_test(next_oyster)
{
    char *str = "and (... never (@ and never) to be) my love";

    GScanner *a = string_scanner(str);

    oyster *ret = next_oyster(a);
    assert(ret->in->type == SYMBOL, "wrong type");
    assert(ret->in->symbol_id == sym_id_from_string("and"),
           "wrong string %d %d", sym_id_from_string("and"),
           ret->in->symbol_id);

    decref(ret);
    ret = next_oyster(a);

    assert(ret->in->type == CONS, "wrong type");

    decref(ret);
    g_scanner_destroy(a);

} _tset;

_test(read_symbol)
{
    char *str = "hello-there!dahling never read this";
    FILE *stream = fmemopen(str, strlen(str), "r");
    token *ret  = read_symbol(stream);
    assert(strcmp("hello-there!dahling", ret->string) == 0);
    free(ret->string);
    free(ret);
} _tset;

_test(read_prefix)
{
    char *str = "$hello-there!dahling never read this";
    FILE *stream = fmemopen(str, strlen(str), "r");
    token *ret  = read_prefix(stream);
    assert(strcmp("unary-$", ret->string) == 0);
    free(ret->string);
    free(ret);
} _tset;


_test(read_open_close)
{
    char *str = "(hello-there)";
    FILE *stream = fmemopen(str, strlen(str), "r");
    token *a  = read_open(stream);
    token *b = read_symbol(stream);
    token *c  = read_close(stream);
    assert(a->type == OPEN_TOKEN);
    assert(c->type == CLOSE_TOKEN);
    free(b->string);
    free(b);
} _tset;

_test(read_newline)
{
    char *str = "\n    there)";
    FILE *stream = fmemopen(str, strlen(str), "r");
    token *a  = read_newline(stream);
    assert(a->type == NEWLINE_TOKEN);
    assert(a->count == 4);
} _tset;


_test(parsing)
{
    printf("\nTesting parsing:\n");
    run_test(read_symbol);
    run_test(read_prefix);
    run_test(read_open_close);
    run_test(next_oyster);
    run_test(read_newline);
} _tset;
