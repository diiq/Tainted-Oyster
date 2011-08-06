#ifndef PARSING
#define PARSING

// This file handles parsing.

#include "oyster.h"
#include "parsing.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <ctype.h>
#include <error.h>
#include <wchar.h>
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

void read_word(FILE *stream, char *place){
    int c;
    int i;
    for(c = fgetc(stream), i=0; c != EOF && c != ' '; c = fgetc(stream), i++){
        place[i] = c;
    }
    ungetc(c, stream);
    place[i] = '\0';
}

void push_word(char *string, FILE *stream){
    int a = strlen(string);
    for(a--; a>=0; a--){
        ungetc(string[a], stream);
    }
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

    if(!isalpha(c) && c != '.'){
        ungetc(c, stream);
        return NULL;
    } 
      
    a[0] = c;
    int i = 1;
    c = fgetc(stream);
    while(!delimiter(c) && (c != '.' || a[i-1] == '.')){
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

token *read_number(FILE *stream){
    int c = fgetc(stream);
    ungetc(c, stream);
    
    if(!isdigit(c)){
        return NULL;
    } 

    double a;
    int e = fscanf(stream, "%lf", &a);
    if (!e) error(314, 0, "What. There is a digit but fscanf finds no number.");
    token *ret = make_token(NUMBER_TOKEN);
    ret->num = a;
    return ret;
}

token *read_info_access(FILE *stream){
    int c = fgetc(stream);
    if(c != '.'){
        ungetc(c, stream);
        return NULL;
    }
    int d = fgetc(stream);
    if(d == '.'){
        ungetc(d, stream);
        ungetc(c, stream);
        return NULL;
    }
    ungetc(d, stream);
    return make_token(INFO_ACCESS_TOKEN);
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
    ungetc(c, stream);
    char a[100];
    read_word(stream, a);
    int len = mblen(a, 100);
    push_word(&(a[len]), stream);
    a[len] = '\0';
    token *ret = make_token(PREFIX_TOKEN);
    ret->string = malloc(sizeof(char)*(8+len)); 
    sprintf(ret->string, "unary-%s", a);
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

int read_replace(FILE *stream){
    char *replace[] = {
        "::", "<<join>>",
        "<-", "<<set>>",
        "←", "<<set>>",
        "+", "<<binary-+>>",
        "-", "<<binary-->>",
        "*", "<<binary-*>>",
        "/", "<<binary-/>>",
        "==", "<<is>>",
        ">", "<<binary-greater>>",
        "<", "<<binary-less>>",
    };
    int replace_len = 10;
    char a[1000];
    read_word(stream, a);
    int i;
    for(i=0; i<replace_len*2; i += 2){
        if(strcmp(a, replace[i]) == 0){
            push_word(replace[i+1], stream);
            return 1;
        }
    }
    push_word(a, stream);
    return 0;
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

token *next_token(FILE *stream)
{
    token *ret = NULL;
    
    int i = 1;
    while(i){
        i = 0;
        i += read_backslash(stream);
        i += read_space(stream);
        i += read_comment(stream);
        i += read_replace(stream);
    }

    ret = read_newline(stream);
    if (ret) return ret;

    ret = read_info_access(stream);
    if (ret) return ret;

    ret = read_symbol(stream);
    if (ret) return ret;

    ret = read_number(stream);
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

void free_token_stream(token_stream *t){
    void free_tokens(token_chain *chain){
        if(chain){
            free_tokens(chain->next);
            free_token(chain->it);
            free(chain);
        }
    }
    free_tokens(t->pulled);
    free(t);
}


// ---------------------------- Parsing --------------------------------//

oyster *parse_symbol(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == SYMBOL_TOKEN){
        oyster *ret = make_symbol(sym_id_from_string(next->string));
        free_token(next);
        return ret;
    }
    unget_token(next, stream);
    return NULL;
}

oyster *parse_number(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == NUMBER_TOKEN){
        oyster *ret = make_number(next->num);
        free_token(next);
        return ret;
    }
    unget_token(next, stream);
    return NULL;
}

oyster *parse_parens(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == OPEN_TOKEN){
        oyster *ret = parse_expression(stream);
        free_token(next);
        next = get_token(stream);
        if(next->type != CLOSE_TOKEN)
            error(314, 0, "Parse error: expected ).");
        free_token(next);
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
        free_token(next);
        return ret;
    }
    unget_token(next, stream);
    return NULL;
}

int info_access(token_stream *stream){
    token *next = get_token(stream);
    if(next->type != INFO_ACCESS_TOKEN){
        unget_token(next, stream);
        return 0;
    }
    return 1;
}

oyster *parse_one(token_stream *stream)
{
    oyster *ret;
    if ((ret = parse_symbol(stream)));
    else if ((ret = parse_number(stream)));
    else if ((ret = parse_parens(stream)));
    else if ((ret = parse_prefix(stream)));
    else ret = NULL;

    if(ret && info_access(stream)){
        ret = list(3, 
                   make_symbol(sym_id_from_string("table-get")),
                   parse_one(stream),
                   list(2, make_symbol(sym_id_from_string("info-table")), ret));
    }

    return ret;
}

oyster *parse_infix(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == INFIX_TOKEN){
        oyster *ret = parse_one(stream);
        free_token(next);
        next = get_token(stream);
        if(next->type != DEFIX_TOKEN)
            error(314, 0, "Parse error: expected >>.");
        free_token(next);
        return ret;
    }
    unget_token(next, stream);
    return NULL;
}

#define handle_singleton(expression)                  \
    do{                                               \
        oyster * TEMP = cdr(expression);              \
        if(nilp(TEMP)){                               \
            oyster *temp = car(expression);           \
            incref(temp);                             \
            decref(expression);                       \
            expression = temp;                        \
        }                                             \
        decref(TEMP);                                 \
    }while(0);                                        \

// todo break into parse_colon and parse_colon_newline
oyster *parse_colon(token_stream *stream){
    token *next = get_token(stream);
    if(next->type == COLON_TOKEN){
        free_token(next);
        next = get_token(stream);
        if(next->type == NEWLINE_TOKEN){
            int indent = next->count;
            oyster *ret = nil();
            while(1){
                oyster *subret = parse_expression(stream);
                
                incref(subret);
                handle_singleton(subret);

                ret = cons(subret, ret);
                decref(subret);
                free(next);
                next = get_token(stream);
                if(next->type != NEWLINE_TOKEN){
                    unget_token(next, stream);
                    break;
                } else if (next->count > indent) {
                    error(314, 0, "Unexpected indent.");
                } else if (next->count < indent) {
                    unget_token(next, stream);
                    break;
                } 
            }
            return ret;
        } else {
            unget_token(next, stream);
            oyster *subret = parse_expression(stream);

            incref(subret);
            handle_singleton(subret);
            oyster *ret = list(1, subret);
            decref(subret);
            return ret;
        }
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
            oyster *pre = reverse(ret);
            
            incref(pre);
            handle_singleton(pre);

            oyster *post = parse_expression(stream);
            incref(post);
            handle_singleton(post);
 
            ret = list(3, post, pre, subret);
            decref(pre);
            decref(post);
            continue;
        }


        subret = parse_colon(stream);
        if(subret){
            ret = append(subret, ret);
            continue;
        }

        break;
    }
    return reverse(ret);
}

oyster *read(token_stream *x){
    oyster *ret = parse_expression(x);

    incref(ret);
    handle_singleton(ret);

    token *t = get_token(x);
    if(t->type != NEWLINE_TOKEN){
        error(314, 0, "Unexpected token.");
    }
    if (nilp(ret) && t->count < 0){
        decref(ret);
        ret = NULL;
    }
    free_token(t);
    return ret;
}

// -------------------------------------------------------------------- //
// The symbol table

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

#endif
