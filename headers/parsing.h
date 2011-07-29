#ifndef PARSINGH
#define PARSINGH

#include "stdio.h"

enum {
    SYMBOL_TOKEN,
    PREFIX_TOKEN,
    INFIX_TOKEN,
    DEFIX_TOKEN,
    OPEN_TOKEN,
    CLOSE_TOKEN,
    COLON_TOKEN,
    NEWLINE_TOKEN,
    NOTHING_TOKEN
};

typedef struct {
    char *string;
    int type;
    int count;
} token;

typedef struct token_chain_struct token_chain;

struct token_chain_struct {
    token_chain *next;
    token *it;
};

typedef struct {
    token_chain *pulled;
    FILE *char_stream;
} token_stream;

typedef struct partial_parse partial_parse;
typedef struct partial_parsee partial_parsee;

struct partial_parse{
    partial_parsee *it;
};

struct partial_parsee {
    int islist;
    token *token;
    partial_parsee *first;
    partial_parsee *rest;
};

oyster *next_oyster(GScanner * in);
int sym_id_from_string(char *sym);
char *string_from_sym_id(int sym);
void init_symbol_table();
void add_symbol(int id, char *sym);
void free_symbol_table();
GScanner *string_scanner(char *text);
GScanner *file_scanner(char *file);

token_stream *make_token_stream(FILE *stream);
token *get_token(token_stream *stream);
void unget_token(token *token, token_stream *stream);

oyster *parse_one(token_stream *stream);
oyster *parse_expression(token_stream *stream);

#endif
