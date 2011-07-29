#ifndef PARSING
#define PARSING

// This file handles parsing.

#include "oyster.h"
#include "parsing.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>
#include <glib.h>
#include <ctype.h>
#include <error.h>

// ------------------------ psychotic bastard ------------------------- //
// -------------------------- tokenizer ------------------------------- //
// All of this needs EOF validation somewhere.

int delimiter(char c){
    if (c == ' '  ||
        c == '('  ||
        c == ')'  ||
        c == ':'  ||
        c == '>'  ||
        c == '\n' ||
        c == EOF)
        return 1;
    return 0;
}

token *make_token(int type){
    token *ret = malloc(sizeof(token));
    ret->type = type;
    ret->string = NULL;
    ret->count = 0;
    return ret;
}

void print_token(token *t){
    if(t->type == SYMBOL_TOKEN){
        printf("%s", t->string);
    } else {
        char * conv[]={"SYMBOL_TOKEN",
                       "PREFIX_TOKEN",
                       "INFIX_TOKEN",
                       "DEFIX_TOKEN",
                       "OPEN_TOKEN",
                       "CLOSE_TOKEN",
                       "COLON_TOKEN",
                       "NEWLINE_TOKEN",
                       "NOTHING_TOKEN"};
        printf("%s", conv[t->type]);
    }
}

void free_token(token *t){
    if(t->string) free(t->string);
    free(t);
}

token *read_symbol(FILE *stream){
    char a[1000]; // for clarity, for now
    int c = fgetc(stream);

    if(!isalpha(c)){
        ungetc(c, stream);
        return NULL;
    } 
      
    a[0] = c;
    int i = 1;
    c = fgetc(stream);
    while(!delimiter(c)){
        a[i] = c;
        c = fgetc(stream);
        i++;
    }
    ungetc(c, stream);
    a[i] = '\0';
    
    token *ret = make_token(SYMBOL_TOKEN);
    ret->string = malloc((strlen(a)+1)*sizeof(char));
    strcpy(ret->string, a);
    return ret;
}


token *read_prefix(FILE *stream){
    int c = fgetc(stream);
    if(isalnum(c) || delimiter(c)){
        ungetc(c, stream);
        return NULL;
    } 
    int d = fgetc(stream);
    if(d == ' ' || d == ':'){
        ungetc(d, stream);
        ungetc(c, stream);
        return NULL;
    }
    ungetc(d, stream);
    token *ret = make_token(PREFIX_TOKEN);
    ret->string = malloc(sizeof(char)*8); 
    sprintf(ret->string, "unary-%c", c);
    return ret;
}


token *read_open(FILE *stream){
    int c = fgetc(stream);
    if(c != '('){
        ungetc(c, stream);
        return NULL;
    }
    return make_token(OPEN_TOKEN);
}

token *read_close(FILE *stream){
    int c = fgetc(stream);
    if(c != ')'){
        ungetc(c, stream);
        return NULL;
    }
    return make_token(CLOSE_TOKEN);
}

token *read_colon(FILE *stream){
    int c = fgetc(stream);
    if(c != ':'){
        ungetc(c, stream);
        return NULL;
    }
    return make_token(COLON_TOKEN);
}


token *read_infix(FILE *stream)
{
    int c = fgetc(stream);
    int d = fgetc(stream);
    
    if(c != '<' || d != '<'){
        ungetc(d, stream);
        ungetc(c, stream);
        return NULL;
    } 
    token *ret = make_token(INFIX_TOKEN);
    return ret;
}

token *read_defix(FILE *stream)
{
    int c = fgetc(stream);
    int d = fgetc(stream);
    
    if(c != '>' || d != '>'){
        ungetc(d, stream);
        ungetc(c, stream);
        return NULL;
    } 
    token *ret = make_token(DEFIX_TOKEN);
    return ret;
}

int read_backslash(FILE *stream)
{
    int c = fgetc(stream);
    if(c != '\\'){
        ungetc(c, stream);
        return 0;
    }
    int d = fgetc(stream);
    if(d != '\n'){
        ungetc(d, stream);
        ungetc(c, stream);
        return 0;
    }
    return 1;
}

int read_space(FILE *stream){
    int c = fgetc(stream);
    if(c != ' ') {
        ungetc(c, stream);
        return 0;
    }
    for(c = fgetc(stream); c == ' '; c = fgetc(stream));
    ungetc(c, stream);
    return 1;
}

int read_comment(FILE *stream){
    int c = fgetc(stream);
    if(c != '#'){
        ungetc(c, stream);
        return 0;
    }
    while(c != '\n')
        c = fgetc(stream);
    return 1;
}

token *read_newline(FILE *stream){
    int c = fgetc(stream);
    if(c != '\n'){
        ungetc(c, stream);
        return NULL;
    }
    token *ret =  make_token(NEWLINE_TOKEN);
    ret->count = 0;
    for(c = fgetc(stream); c == ' '; c = fgetc(stream)){
        ret->count++;
    }

    int i = 1;
    while(i){
        i = 0;
        i += read_backslash(stream);
        i += read_space(stream);
        i += read_comment(stream);
    }

    token *ret2 = read_newline(stream);
    if (ret2){
        free(ret);
        return ret2;
    } else {
        return ret;
    }
}

token *next_token(FILE *stream){
    token *ret = NULL;
    
    int i = 1;
    while(i){
        i = 0;
        i += read_backslash(stream);
        i += read_space(stream);
        i += read_comment(stream);
    }

    ret = read_newline(stream);
    if (ret) return ret;

    ret = read_symbol(stream);
    if (ret) return ret;

    ret = read_open(stream);
    if (ret) return ret;

    ret = read_close(stream);
    if (ret) return ret;

    ret = read_colon(stream);
    if (ret) return ret;

    ret = read_newline(stream);
    if (ret) return ret;

    ret = read_infix(stream);
    if (ret) return ret;

    ret = read_defix(stream);
    if (ret) return ret;

    ret = read_prefix(stream);
    if (ret) return ret;

    ret = make_token(NEWLINE_TOKEN);
    ret->count = -1;
    return ret;
}

//------------------------ token stream -------------------------------//

token_stream *make_token_stream(FILE *stream)
{
    token_stream *ret = calloc(1, sizeof(token_stream));
    ret->char_stream = stream;
    token * next = get_token(ret);
    while(next->type == NEWLINE_TOKEN){ // kuldge!
        free(next);
        next = get_token(ret);
    }
    unget_token(next, ret);
    return ret;
}

token *get_token(token_stream *stream)
{
    if(stream->pulled){
        token *ret = stream->pulled->it;
        token_chain *temp = stream->pulled;
        stream->pulled = stream->pulled->next;
        free(temp);
        return ret;
    }
    return next_token(stream->char_stream);
}

void unget_token(token *token, token_stream *stream)
{
    token_chain *t = malloc(sizeof(token_chain));
    t->next = stream->pulled;
    t->it = token;
    stream->pulled = t;
}



// ---------------------------- Parsing --------------------------------//

oyster *parse_symbol(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == SYMBOL_TOKEN){
        oyster *ret = make_symbol(sym_id_from_string(next->string));
        free(next);
        return ret;
    }
    unget_token(next, stream);
    return NULL;
}

oyster *parse_parens(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == OPEN_TOKEN){
        oyster *ret = parse_expression(stream);
        free(next);
        next = get_token(stream);
        if(next->type != CLOSE_TOKEN)
            error(314, 0, "Parse error: expected ).");
        return ret;
    }
    unget_token(next, stream);
    return NULL;
}

oyster *parse_prefix(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == PREFIX_TOKEN){
        oyster *func = make_symbol(sym_id_from_string(next->string));
        oyster *ret = list(2, func, parse_one(stream));
        free(next);
        return ret;
    }
    unget_token(next, stream);
    return NULL;
}

oyster *parse_one(token_stream *stream)
{
    oyster *ret;
    ret = parse_symbol(stream);
    if (ret) return ret;
    
    ret = parse_parens(stream);
    if (ret) return ret;

    ret = parse_prefix(stream);
    if (ret) return ret;

    return NULL;
}

oyster *parse_infix(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == INFIX_TOKEN){
        oyster *ret = parse_one(stream);
        free(next);
        next = get_token(stream);
        if(next->type != DEFIX_TOKEN)
            error(314, 0, "Parse error: expected >>.");
        return ret;
    }
    unget_token(next, stream);
    return NULL;
}

oyster *parse_expression(token_stream *stream)
{
    oyster *ret = nil();
    oyster *subret = NULL;
    while(1){
        subret = parse_one(stream);
        if(subret){
            ret = cons(subret, ret);
            continue;
        }

        subret = parse_infix(stream);
        if(subret){
            ret = list(3, parse_expression(stream), ret, subret);
            continue;
        }

        break;
    }
    return reverse(ret);
}

// -------------------------------------------------------------------- //

int current_max_symbol;

struct symbol_table {
    GHashTable *str;
    GHashTable *sym;
} *symbol_table;

void init_symbol_table()
{
    if (!symbol_table) {
        symbol_table = malloc(sizeof(struct symbol_table));
        symbol_table->sym =
            g_hash_table_new_full(g_int_hash, g_int_equal, NULL, NULL);
        symbol_table->str =
            g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
        current_max_symbol = MAX_PREDEF_SYMBOL + 50;
    }
};

void free_symbol_table()
{
    g_hash_table_destroy(symbol_table->sym);
    g_hash_table_destroy(symbol_table->str);
    free(symbol_table);
}

void add_symbol(int id, char *sym)
{
    char *val = malloc(sizeof(char) * (strlen(sym) + 1));
    memcpy(val, sym, (strlen(sym) + 1) * sizeof(char));
    int *key = malloc(sizeof(int));
    *key = id;
    g_hash_table_insert(symbol_table->sym, key, val);
    g_hash_table_insert(symbol_table->str, val, key);
}

int sym_id_from_string(char *sym)
{
    int *j = g_hash_table_lookup(symbol_table->str, sym);
    if (j) {
        return *j;
    }
    current_max_symbol++;
    add_symbol(current_max_symbol, sym);
    return current_max_symbol;
}

char *string_from_sym_id(int sym)
{
    char *ret = g_hash_table_lookup(symbol_table->sym, &sym);
    if (ret)
        return ret;
    return "wow-I-have-no-idea-what-this-symbol-is-where-did-you-find-it?";
}

GScanner *make_scanner()
{
    GScanner *scan = g_scanner_new(NULL);
    scan->config->cset_identifier_first = ("abcdefghijklmnopqrstuvwxyz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "'-!@$%^&*<>?,./=_+`~");
    scan->config->cset_identifier_nth = ("abcdefghijklmnopqrstuvwxyz"
                                         "1234567890"
                                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                         "'-!@$%^&*<>?,./=_+`~");
    //    scan->config->char_2_token = FALSE;
    scan->config->scan_string_sq = FALSE;
    scan->config->scan_identifier_1char = TRUE;
    scan->config->skip_comment_single = TRUE;
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


oyster *next_oyster(GScanner * in)
{
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
        while (cur) {
            ret = cons(cur, ret);
            cur = next_oyster(in);
        }
        oyster *rev = reverse(ret);
        decref(ret);
        ret = rev;
    } else if (in->token == G_TOKEN_RIGHT_PAREN)
        ret = NULL;
    //    table_set(sym_id_from_string("line-number"), 
    //          make_number(line_number), 
    //          ret->in->info);
    return ret;
}

#endif
