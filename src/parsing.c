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

    return NULL;
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

oyster *read_line(token_stream *stream, int indent)
{
    // test here for prefix
    oyster * ret;

    token * cur = get_token(stream);
    if(cur->type == INFIX_TOKEN ||
       cur->type == CLOSE_TOKEN){
        unget_token(cur, stream);
        return NULL;
    }

    unget_token(cur, stream);

    ret = read_line_straight(stream, indent);
   
    cur = get_token(stream);
    if(cur->type == INFIX_TOKEN){
        oyster *med = read_line(stream, indent);
        free(cur);
        cur = get_token(stream);
        if (cur->type != DEFIX_TOKEN){
            printf("What the hey? I was expecting a >>!\n");
        }
        oyster *final = read_line(stream, indent);
        ret = list(3, med, ret, final);
    } else if (cur->type == OPEN_TOKEN){
        free(cur);
        oyster *inner = cons(read_line(stream, indent), nil());
        cur = get_token(stream);
        while(cur->type != CLOSE_TOKEN){
            unget_token(cur, stream);
            inner = cons(read_line(stream, indent), inner);
            cur = get_token(stream);
        }
        inner = reverse(inner);
        ret = inner;
    } else {
        unget_token(cur, stream);
    }
    return ret;
}


oyster *read_line_straight(token_stream *stream, int indent)
{
    // o sweet baby jesus
    // TODO: FREE CUR
    oyster *ret;
    token *cur = get_token(stream);
    switch (cur->type){

    case SYMBOL_TOKEN:
        {
            token *next;
            ret = nil();
            int i;
            for(next = cur, i = 0;
                next->type == SYMBOL_TOKEN;
                next = get_token(stream), i++){
                ret = cons(make_symbol(sym_id_from_string(next->string)), ret);
                free(next);
            }
            unget_token(next, stream);

            ret = reverse(ret);
            oyster *ret2 = read_line(stream, indent);

            if (i == 1 && (!ret2 || nilp(ret2))){
                ret = car(ret);
            } else if (ret2){
                 ret = append(ret, ret2);
            }
        }
        break;

    case NEWLINE_TOKEN:
        if (cur->count < indent){
            unget_token(cur, stream);
        } else if (cur->count != indent){
            printf("What the hey? There's an indent without a colon!\n");
        }
        ret = nil();
        break;

    case COLON_TOKEN:
        {
            token *next = get_token(stream);
            if (next->type == NEWLINE_TOKEN){
                if (next->count <= indent){
                    printf ("What the hey? That's a colon without an indent!\n");
                }
                int cindent = next->count;
                free(next);
                ret = nil();
                //for each read_line, check for newline; 
                // if found, check against current indent
                // stop when indent completed.
                while(1){

                    oyster *a = read_line(stream, cindent);

                    ret = cons(a, ret);
                    next = get_token(stream);
                    if (next->type != NEWLINE_TOKEN){
                        unget_token(next, stream);
                        continue;
                    } else {
                        if (next->count < indent){
                            unget_token(next, stream);
                            break;
                        } else if (next->count == indent){
                            break;
                        } else {
                            cindent = next->count;
                        }
                    }
                }
                ret = reverse(ret);
                    
            } else {
                unget_token(next, stream);
                oyster *a = read_line(stream, indent);
                ret = list(1, a);
            }
        }
        break;
        
        //    case NEWLINE_TOKEN:
    case INFIX_TOKEN:
    case DEFIX_TOKEN: 
    case OPEN_TOKEN:
    case CLOSE_TOKEN:
        unget_token(cur, stream);
        ret = NULL;
        break;

    case PREFIX_TOKEN:
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
