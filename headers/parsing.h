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
    BACKSLASH_TOKEN
};

typedef struct {
    char *string;
    int type;
    int count;
} token;

oyster *next_oyster(GScanner * in);
int sym_id_from_string(char *sym);
char *string_from_sym_id(int sym);
void init_symbol_table();
void add_symbol(int id, char *sym);
void free_symbol_table();
GScanner *string_scanner(char *text);
GScanner *file_scanner(char *file);

#endif
