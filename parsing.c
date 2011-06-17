#include "parsing.h"
#include "gc.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>

char *one_string(char c){
    char *str = NEW(2, char);
    str[1] = 0;
    str[0] = c;
    return str;
}

parsed fail(char *stream, int index){
    parsed cheese = {0, "", 0, index};
    return cheese;
}

parsed anyer(char *stream, int index, parser p){
    char *str = one_string(stream[index]);
    parsed cheese = {1, str, 1, index+1};
    return cheese;
}

parsed characterer(char *stream, int index, parser p){
    if(stream[index] == p.c){
        parsed cheese = {1, one_string(p.c), 1, index+1};
        return cheese;
    } else {
        return fail(stream, index);
    }
}

parsed one_ofer(char *stream, int index, parser p){  
    int len = strlen(p.cs);
    int i=0;
    parsed o;
    do {
        parser x = make_character(p.cs[i]);
        o = x.parse(stream, index, x); 
        i++;
    } while(i<len && o.flag == 0);

    if (o.flag) {
        return o;
    }
    return fail(stream, index);
}

parsed manyer(char *stream, int index, parser p){
    parsed o;
    void **ret = GC_MALLOC(sizeof(void *)*1);;
    int i=0;
    int newindex = index;
    do{
        o = p.pser->parse(stream, newindex, *p.pser);
        if(o.flag){
            i++;
            ret = GC_REALLOC(ret, sizeof(void *)*i);
            ret[i-1] = o.obj;
            newindex = o.index;
        }
    } while (o.flag);
    if (i > 0){
        parsed cheese = {1, ret, i, newindex};
        return cheese;
    } 
    return fail(stream, index);
}


parser make_any(){
    parser x;
    x.parse = anyer;
    return x;
}

parser make_character(char c){
    parser x;
    x.parse = characterer;
    x.c = c;
    return x;
}

parser make_one_of(char *cs){    
    char *str = NEW(strlen(cs)+1, char);
    int i;
    for(i=0; i<strlen(cs)+1; i++){
        str[i] = cs[i];
    }
    parser x;
    x.parse = one_ofer;
    x.cs = str;
    return x;
}

parser make_many(parser p){
    parser *x = NEW(1, parser);
    *x = p;
    parser ret;
    ret.pser = x;
    ret.parse = manyer;
    return ret;
}
