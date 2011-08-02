#include "oyster.h"
#include "parsing.h"
#include "testing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tests_run;

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
    char *str = ("this ... '(is <<(antioch)>> ,unweildy a) feild:\n"
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
    run_test(next_token);
    run_test(token_stream);
    run_test(read_one);
} _tset;


