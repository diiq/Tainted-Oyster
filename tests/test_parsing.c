#include "oyster.h"
#include "parsing.c"
#include "testing.h"
#include "stdio.h"

extern int tests_run;

_test(read_symbol)
{
    char *str = "hello-there!dahling never read this";
    FILE *stream = fmemopen(str, strlen(str), "r");
    token *ret  = read_symbol(stream);
    assert(strcmp("hello-there!dahling", ret->string) == 0);
    free_token(ret);
} _tset;

_test(read_prefix)
{
    char *str = "$hello-there!dahling never read this";
    FILE *stream = fmemopen(str, strlen(str), "r");
    token *ret  = read_prefix(stream);
    assert(strcmp("unary-$", ret->string) == 0);
    free_token(ret);
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
    free_token(a);
    free_token(b);
    free_token(c);
} _tset;

_test(read_newline)
{
    char *str = "\n    there)";
    FILE *stream = fmemopen(str, strlen(str), "r");
    token *a  = read_newline(stream);
    assert(a->type == NEWLINE_TOKEN);
    assert(a->count == 4);
    free_token(a);
} _tset;

_test(next_token)
{
    char *str = "this <<is>> a: (series) 'of\n    tokens ";
    FILE *stream = fmemopen(str, strlen(str), "r");
    
    int conv[] = {SYMBOL_TOKEN, 
                  INFIX_TOKEN,
                  SYMBOL_TOKEN, 
                  DEFIX_TOKEN,
                  SYMBOL_TOKEN,
                  COLON_TOKEN,
                  OPEN_TOKEN,
                  SYMBOL_TOKEN, 
                  CLOSE_TOKEN,
                  PREFIX_TOKEN,
                  SYMBOL_TOKEN, 
                  NEWLINE_TOKEN,
                  SYMBOL_TOKEN};
    token *a;
    int i;
    for(i = 0, a = next_token(stream); 
        a->count >= 0;
        a = next_token(stream), i++){
        assert(a->type == conv[i], 
               "I failed on %d cause %d isnt %d.", i, a->type, conv[i]);
        free_token(a);
    }
    free_token(a);
}_tset;

_test(token_stream){
    char *str = "this <<is>> a: (series) 'of\n    tokens ";
    FILE *stream = fmemopen(str, strlen(str), "r");
    
    token_stream *a = make_token_stream(stream);
    token *b = get_token(a);
    assert(b->type == SYMBOL_TOKEN);
    unget_token(b, a);
    b = get_token(a);
    assert(b->type == SYMBOL_TOKEN);
    free_token(b);
    b = get_token(a);
    assert(b->type == INFIX_TOKEN);
    free_token(b);
    free_token_stream(a);
}_tset;

_test(read_one){
    char *str = ("this '(is <<(antioch)>> ,unweildy a) feild:\n"
                 "    of dust and peat\n"
                 "    (single sauce)\n"
                 "    soop:\n"
                 "        (poop)\n"
                 "this is another expression here");
    FILE *stream = fmemopen(str, strlen(str), "r");    
    token_stream *a = make_token_stream(stream);

    oyster_print(parse_expression(a));printf("\n");
    free(get_token(a));
    oyster_print(parse_expression(a));
    free_token_stream(a);

}_tset;


_test(parsing)
{
    printf("\nTesting parsing:\n");
    run_test(read_symbol);
    run_test(read_prefix);
    run_test(read_open_close);
    run_test(read_newline);
    run_test(next_token);
    run_test(token_stream);
    run_test(read_one);
} _tset;


