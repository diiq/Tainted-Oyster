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
    ret->count = 0;
    return ret;
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
    ungetc(c, stream);

    token *ret2 = read_newline(stream);
    if (ret2){
        free(ret);
        return ret2;
    } else {
        return ret;
    }
}

void *read_backslash(FILE *stream)
{
    int c = fgetc(stream);
    if(c != '\\'){
        ungetc(c, stream);
        return NULL;
    }
    int d = fgetc(stream);
    if(d != '\n'){
        ungetc(d, stream);
        ungetc(c, stream);
        return NULL;
    }
    return NULL;
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

void read_space(FILE *stream){
    int c;
    for(c = fgetc(stream); c == ' '; c = fgetc(stream));
    ungetc(c, stream);
}

token *next_token(FILE *stream){
    token *ret;

    read_backslash(stream);
    read_space(stream);

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

// ------------------------------ PB parser --------------------------- //
oyster *parse_symbols(token_stream *stream, int indent)
{
    token *next;
    oyster *ret = nil();
    while(1){ // change to for loop or summat
        next = get_token(stream);
        if(next->type != SYMBOL_TOKEN){
            unget_token(next, stream);
            break;
        }
        ret = cons(make_symbol(sym_id_from_string(next->string)), ret);
        free(next);
    }
    ret = reverse(ret);
    return ret;
}

oyster *read_one(token_stream *stream, int indent){
    // eventually, turn this into a dijkstra-style operator-precedence thing. Maybe.
    //repeatedly:
    oyster *ret = parse_symbols(stream, indent);
    token *next = get_token(stream);
    switch(next->type){
    case NEWLINE_TOKEN:
    case CLOSE_TOKEN:
    case DEFIX_TOKEN:
        unget_token(next, stream);
        //wrap up
        break;
    
    case PREFIX_TOKEN:
        {
            oyster *full;
            token *nnext = get_token(stream);
            int flag = 1;
            switch (nnext->type){
            case PREFIX_TOKEN:
                flag = 0;
            case OPEN_TOKEN:
                {
                    unget_token(nnext, stream);
                    oyster *rest = read_one(stream, indent);
                    incref(rest);
                    oyster *first = car(rest);
                    oyster *last = cdr(rest);
                    decref(rest);
                    full = cons(list(2, 
                                     make_symbol(sym_id_from_string(next->string)),
                                     first),
                                last);
                    if (flag){
                        full = cons(full, read_one(stream, indent));
                    }
                }
                break;
            case SYMBOL_TOKEN:
                full = cons(list(2, 
                                 make_symbol(sym_id_from_string(next->string)),
                                 make_symbol(sym_id_from_string(nnext->string))),
                            read_one(stream, indent));

                free(nnext->string);
                free(nnext);
                break;
            default:
                error(314, 0, "Unexpected token following a prefix.");
            }

            free(next->string);
            free(next);
            ret = append(ret, full);
        }
        break;

    case OPEN_TOKEN:
        {
            free(next);
            oyster *first = ret;
            oyster *inner = read_one(stream, indent);
            token *close = get_token(stream);
            if (close->type != CLOSE_TOKEN){
                error(314, 0, "I was expecting a \")\".");
            }
            free(close);
            oyster *after = read_one(stream, indent);
            ret = append(first, cons(inner, after));
        }
        break;

    case INFIX_TOKEN:
        {
            free(next);
            oyster *first = ret;
            oyster *inner = read_one(stream, indent);
            token *close = get_token(stream);
            if (close->type != DEFIX_TOKEN){
                error(314, 0, "I was expecting a \">>\".");
            }
            free(close);
            oyster *after = read_one(stream, indent);
            ret = list(3, inner, first, after);
        }
        break;

    case COLON_TOKEN:
        { 
            free(next);
            next = get_token(stream);
            if (next->type == NEWLINE_TOKEN){
                oyster *rest = nil();
                int cindent = next->count;
                while(1){
                    oyster *this = read_one(stream, cindent);
                    incref(this);
                    oyster *that;
                    if(oyster_length(this) == 1){
                        that = car(this);
                    } else {
                        that = oyster_copy(this, this->bindings);
                    } 
                    decref(this);
                    rest = cons(that, rest);
                    free(next);
                    next = get_token(stream);
                    if (next->type != NEWLINE_TOKEN){
                        error(314, 0, "I was expecting a new line.");
                    } else if (next->count == indent){
                        break;
                    } else if (next->count < indent){
                        unget_token(next, stream);
                        break;
                    } else if (next->count > cindent){
                        error(314, 0, "Unexpected indent.");
                    } else {
                        cindent = next->count;
                    }
                }
                rest = reverse(rest);
                ret = append(ret, rest);
            } else {
                unget_token(next, stream);

                oyster *this = read_one(stream, indent);
                incref(this);
                oyster *rest;
                if(oyster_length(this) == 1){
                    rest = car(this);
                } else {
                    rest = oyster_copy(this, this->bindings);
                } 
                decref(this);
                    
                ret = append(ret, list(1, rest));
            }
        }
        break;

    }

    return ret;
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
