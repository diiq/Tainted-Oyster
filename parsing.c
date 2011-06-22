#include "parsing.h"
#include "gc.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>


// Tokens and token streams

void push_token(parsed_token *t, token_stream *s){
    s->push_to++;
    s->s = GC_REALLOC(s->s, sizeof(parsed_token*)*(s->push_to+1));
    s->s[s->push_to] = t;
    if (s->pull_from == -1)
        s->pull_from = 0;
}

parsed_token *pull_token(token_stream *s){
    if (s->pull_from == -1 || s->pull_from > s->push_to)
        return NULL;
    s->pull_from++;
    return s->s[s->pull_from-1];
}

token_stream *new_token_stream(){
    token_stream *ret = NEW(token_stream);
    ret->push_to = -1;
    ret->pull_from = -1;
    ret->s = NULL;
    return ret;
}

void print_token(parsed_token *t){
    switch (t->flag) {
    case 0: printf("%s ", t->value); break;
    case 2: printf("( "); break;
    case 3: printf(") "); break;
    case 4: printf(" NEWLINE(%d) ", t->count); break;
    case 6: printf(">( "); break;
    }
}

void print_token_stream(token_stream *s){
    parsed_token *t = pull_token(s);
    for(; t; t = pull_token(s))
        print_token(t);
}

// Parsing tokens

int symbol_character(char c)
{
    char *notcs = " ()\n";
    int j;
    for(j=0; j<strlen(notcs); j++)
        if (notcs[j] == c)
            return 0;
    return 1;
}

parsed_token *parse_symbol(FILE *stream){
    int i = 0;
    char c;
    char *ret = NEW(char);
    for(;;){
        c = fgetc(stream);

        if(c == EOF || !symbol_character(c)) break;

        i++;
        ret = GC_REALLOC(ret, sizeof(char)*(i+1));
        ret[i-1] = c;
    }
    ungetc(c, stream);

    if(i == 0)
        return NULL;

    ret[i] = 0;
    parsed_token *cheese = NEW(parsed_token);
    cheese->flag = SYMBOL;
    cheese->count = i;
    cheese->value = ret;
    return cheese;
}

int parse_whitespace(FILE *stream){
    int ret = 0;
    char c = fgetc(stream);
    while(c == ' ') {
        c = fgetc(stream);
        ret = 1;
    }
    ungetc(c, stream);
    return ret;
}

parsed_token *parse_open(FILE *stream){
    char c = fgetc(stream);
    if(c == '('){
        parsed_token *cheese = NEW(parsed_token);
        cheese->flag = OPEN;
        cheese->count = 1;
        cheese->value = NULL;
        return cheese;
    } 
    ungetc(c, stream);
    return NULL;
}
 
parsed_token *parse_close(FILE *stream){
    char c = fgetc(stream);
    if(c == ')'){
        parsed_token *cheese = NEW(parsed_token);
        cheese->flag = CLOSE;
        cheese->count = 1;
        cheese->value = NULL;
        return cheese;
    } 
    ungetc(c, stream);
    return NULL;
} 

parsed_token *parse_newline(FILE *stream){
    char c = fgetc(stream);
    if(c == '\n'){
        int i = -1;
        do{
            i++;
            c = fgetc(stream);
        } while(c == ' ');
        ungetc(c, stream);
        parsed_token *cheese = NEW(parsed_token);
        cheese->flag = NEWLINE;
        cheese->count = i;
        cheese->value = NULL;
        return cheese;
    } 
    ungetc(c, stream);
    return NULL;
} 

parsed_token *parse_eof(FILE *stream){
    char c = fgetc(stream);
    if (c == EOF){
        parsed_token *cheese = NEW(parsed_token);
        cheese->flag = END;
        cheese->count = 0;
        cheese->value = NULL;        
        return cheese;
    } 
    ungetc(c, stream);
    return NULL;
}


token_stream *tokens(FILE *stream){

    token_stream *ret = new_token_stream();
    parsed_token *v = NULL; 
    do{
        //        if(v)printf("%d\n", v->flag);

        v = parse_eof(stream);
        if(v) break;

        v = parse_newline(stream);
        if(!v) v = parse_open(stream);
        
        if(!v) v = parse_close(stream);
        if(!v) v = parse_symbol(stream);                      
        if(!v) {
            printf("Not found? %c\n", fgetc(stream));
            return ret;
        }

        push_token(v, ret);

        if(!parse_whitespace(stream)){
            parsed_token *newv = parse_open(stream);
            if(newv){
                if(v->flag==SYMBOL || v->flag == CLOSE){
                    if(newv->flag == OPEN){
                        newv->flag = FUNCTIONAL_OPEN;
                    }
                }
            
                v = newv;
                push_token(v, ret);
            }
        }

    } while(v);
    
    return ret;
}

// Manipulating streams

/* token_stream *strip_newlines(token_stream *in){ */
/*     token_stream *ret = new_token_stream(); */
/*     parsed_token *window[3]; */
/*     int i; */
/*     for(i=0;i<3;i++) */
/*         window[i] = pull_token(in); */

/*     int current_indent = 0; */

/*     parsed_token *open = NEW(parsed_token); */
/*     open->flag = OPEN; */

/*     parsed_token *close = NEW(parsed_token); */
/*     close->flag = CLOSE; */

/*     for(;window[2];){ */
/*         printf("0: ");print_token(window[0]);         */
/*         printf("1: ");print_token(window[1]);         */
/*         printf("2: ");print_token(window[2]); */
/*         printf("\n"); */
/*         if (window[0]->flag == NEWLINE){ */
            
/*             if(window[1]->flag == NEWLINE){ */
/*                 window[0] = window[1]; */
/*                 window[1] = window[2]; */
/*                 window[2] = pull_token(in); */
/*                 continue; */
/*             }  */
            
/*             if (window[0]->count <= current_indent){ */
/*                 int indent = (current_indent-window[0]->count)/4; */
/*                 for(;indent;indent--) */
/*                     push_token(close, ret); */
/*                 current_indent = window[0]->count; */
/*             } */
            
/*             if (window[2]->flag == NEWLINE){ */
/*                 if (window[2]->count > current_indent){ */
/*                     push_token(open, ret); */
/*                 } */
                
/*                 push_token(window[1], ret); */
               
/*                 window[0] = window[2]; */
/*                 window[1] = pull_token(in); */
/*                 window[2] = pull_token(in); */
/*             } else { */
/*                 push_token(open, ret); */
/*                 push_token(window[1], ret); */
/*                 window[0] = window[2]; */
/*                 window[1] = pull_token(in); */
/*                 window[2] = pull_token(in); */
/*             } */
           
/*         } else if(window[1]->flag == NEWLINE){ */
/*             push_token(window[0], ret); */
/*             if (window[1]->count <= current_indent){ */
/*                 int dedent = (current_indent-window[1]->count)/4; */
/*                 push_token(close, ret); */
/*                 for(;dedent;dedent--) */
/*                     push_token(close, ret); */
/*             } */
/*             current_indent = window[1]->count; */
/*             window[0] = window[1]; */
/*             window[1] = window[2]; */
/*             window[2] = pull_token(in); */
/*         } else { */
/*             push_token(window[0], ret); */
/*             window[0] = window[1]; */
/*             window[1] = window[2]; */
/*             window[2] = pull_token(in); */
/*         } */
/*     } */
    
/*     return ret; */
/* } */


token_stream *strip_newlines(token_stream *in){
    token_stream *ret = new_token_stream();
    parsed_token *b;
    for(b = pull_token(in); b; b = pull_token(in))
        if (b->flag != NEWLINE)
            push_token(b, ret);
    return ret;
}
