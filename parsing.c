#include "parsing.h"
#include "gc.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>

parsed_token *parse_symbol(FILE *stream){
    char *notcs = " ()\n";
    int i = 0;
    char c;
    char *ret = NEW(char);
    for(;;){
        c = fgetc(stream);
        int j;
        for(j=0; j<strlen(notcs); j++){
            if (notcs[j] == c)
                break;
        }
        if (j<strlen(notcs)) break;
        i++;
        GC_REALLOC(ret, i+1);
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

void parse_whitespace(FILE *stream){
    char c = fgetc(stream);
    while(c == ' ') c = fgetc(stream);
    ungetc(c, stream);
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
        fputc(c, stream);
        parsed_token *cheese = NEW(parsed_token);
        cheese->flag = NEWLINE;
        cheese->count = i;
        cheese->value = NULL;
        return cheese;
    } 
    ungetc(c, stream);
    return NULL;
} 

//next_token


