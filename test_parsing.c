#include "gc.h"
#include "parsing.c"
#include "testing.h"
#include "stdio.h"

extern int tests_run;
 
_test(open){
    char *str = "(betty (I love you";
    FILE *stream = fmemopen(str, strlen(str), "r");
    parsed_token *a = parse_open(stream);
    assert(a->flag == PARSED_OPEN);
    parsed_token *b = parse_open(stream);
    assert(b == NULL);
    fclose(stream);
}_tset;

_test(close){
    char *str = ") more than stars love swings (and";
    FILE *stream = fmemopen(str, strlen(str), "r");
    parsed_token *a = parse_close(stream);
    assert(a && a->flag == PARSED_CLOSE);
    parsed_token *b = parse_close(stream);
    assert(b == NULL);
    fclose(stream);
}_tset;

_test(newline){
    char *str = "\n    less (I guess) than three)";
    FILE *stream = fmemopen(str, strlen(str), "r");
    parsed_token *a = parse_newline(stream);
    assert(a->flag == PARSED_NEWLINE);
    assert(a->count == 4);
    parsed_token *b = parse_newline(stream);
    assert(b == NULL);
    fclose(stream);
}_tset;

_test(whitespace){
    char *str = "    and though I";
    FILE *stream = fmemopen(str, strlen(str), "r");
    parse_whitespace(stream);
    assert(fgetc(stream) == 'a');
    fclose(stream);
}_tset;

_test(symbol){
    char *str = "may-fail-to breathe(breathe)";
    FILE *stream = fmemopen(str, strlen(str), "r");
    parsed_token *a = parse_symbol(stream);
    assert(a->flag == PARSED_SYMBOL, "bad type");
    assert(a->count == 11, "bad length %d", a->count);
    assert(a->value[2] == 'y', "bad char, %c", a->value[2]);
    parsed_token *b = parse_newline(stream);
    assert(b == NULL);
    fclose(stream);
}_tset;
    
_test(all_tokens){ 
    char *str = "I-will-never-fail (your breath)(to)\n    betty o-my-love)";
    FILE *stream = fmemopen(str, strlen(str), "r"); 
    token_stream *a = tokens(stream);
    parsed_token *b = pull_token(a);
    assert(b->flag == PARSED_SYMBOL); 
    b = pull_token(a);
    assert(b->flag == PARSED_OPEN);
    b = pull_token(a);
    assert(b->flag == PARSED_SYMBOL);
    b = pull_token(a);
    assert(b->flag == PARSED_SYMBOL);
    b = pull_token(a);
    assert(b->flag == PARSED_CLOSE);
    b = pull_token(a);
    assert(b->flag == PARSED_FUNCTIONAL_OPEN);
    b = pull_token(a);
    assert(b->flag == PARSED_SYMBOL);
    b = pull_token(a);
    assert(b->flag == PARSED_CLOSE);
    b = pull_token(a);
    assert(b->flag == PARSED_NEWLINE);
    fclose(stream);
}_tset; // & know this: I will never parse you

_test(strip_newlines){
    char *str = "(na-na na-na\n    batman!()\n    (is sad))\n    ok?";
    FILE *stream = fmemopen(str, strlen(str), "r");
    token_stream *a = strip_newlines(tokens(stream));
    assert(pull_token(a)->flag == PARSED_OPEN);
}_tset;

_test(parsing){
    printf("\nTesting parsing:\n");
    run_test(open);
    run_test(close);
    run_test(newline);
    run_test(whitespace);
    run_test(symbol);
    run_test(all_tokens);
    run_test(strip_newlines);
}_tset;

