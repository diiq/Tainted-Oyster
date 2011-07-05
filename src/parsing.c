#ifndef PARSING
#define PARSING

// This file handles parsing.

#include "oyster.h"
#include "parsing.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>
#include <glib.h>


// -------------------------------------------------------------------- //

int current_max_symbol;

struct symbol_table {
    GHashTable *str;
    GHashTable *sym;
} *symbol_table;

void init_symbol_table(){
    if(!symbol_table){
        symbol_table = malloc(sizeof(struct symbol_table));
        symbol_table->sym = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, NULL);
        symbol_table->str = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
        current_max_symbol = MAX_PREDEF_SYMBOL+50;
    }
};

void free_symbol_table(){
    g_hash_table_destroy(symbol_table->sym);
    g_hash_table_destroy(symbol_table->str);
    free(symbol_table);
}

void add_symbol(int id, char *sym)
{
    char *val = malloc(sizeof(char)*(strlen(sym)+1)); 
    memcpy(val, sym, (strlen(sym)+1)*sizeof(char));
    int *key = NEW(int);
    *key = id;
    g_hash_table_insert(symbol_table->sym, key, val);
    g_hash_table_insert(symbol_table->str, val, key);
}

int string_equal(void *a, void *b){
    if(strcmp((char *)a, (char *)b) == 0)
        return 1;
    return 0;
}

int sym_id_from_string(char *sym){
    int *j = g_hash_table_lookup(symbol_table->str, sym);
    if (j) {
        return *j;
    }
    current_max_symbol++;
    add_symbol(current_max_symbol, sym);
    return current_max_symbol;
}

char* string_from_sym_id(int sym){
    char *ret = g_hash_table_lookup(symbol_table->sym, &sym);
    if (ret)
        return ret;
    return "wow-I-have-no-idea-what-this-symbol-is-where-did-you-find-it?";
}

GScanner *make_scanner(){
    GScanner *scan = g_scanner_new(NULL);
    scan->config->cset_identifier_first = ("abcdefghijklmnopqrstuvwxyz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "'-!@#$%^&*<>?,./=_+`~");
    scan->config->cset_identifier_nth = ("abcdefghijklmnopqrstuvwxyz"
                                         "1234567890"
                                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                         "'-!@#$%^&*<>?,./=_+`~");
    //    scan->config->char_2_token = FALSE;
    scan->config->scan_string_sq = FALSE;
    scan->config->scan_identifier_1char = TRUE;
    return scan;
}


GScanner *string_scanner(char *text)
{
    GScanner *scan = make_scanner();
    g_scanner_input_text(scan, text, strlen(text));
    return scan;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

GScanner *file_scanner(char *file)
{
    GScanner *scan = make_scanner();
    if (file)
        g_scanner_input_file(scan, open(file, 'r'));
    else
        g_scanner_input_file(scan, 0);
    return scan;
}


oyster *next_oyster(GScanner *in){
    g_scanner_get_next_token(in);
    //int line_number = g_scanner_cur_line(in);
    oyster *ret;
    if (in->token == G_TOKEN_EOF) 
        ret = NULL;
    else if (in->token == G_TOKEN_IDENTIFIER)
        ret = make_symbol(sym_id_from_string(in->value.v_string));
    else if (in->token == G_TOKEN_LEFT_PAREN) {
        ret = nil();
        oyster *cur = next_oyster(in);
        while(cur) {
            ret = cons(cur, ret);
            cur = next_oyster(in);
        } 
        oyster *rev = reverse(ret);
        decref(ret);
        ret = rev;
    }
    else if (in->token == G_TOKEN_RIGHT_PAREN)
        ret = NULL;
    //    table_set(sym_id_from_string("line-number"), 
    //          make_number(line_number), 
    //          ret->in->info);
    return ret;
}

#endif
