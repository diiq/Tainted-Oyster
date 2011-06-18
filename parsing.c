#include "parsing.h"
#include "gc.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>

parsed_value *append_parsed_values(parsed_value *a, int asize, parsed_value *b, int bsize){
    int len = asize + bsize;
    if(len == 0){
        return NULL;
    }
    parsed_value *lis = NEW(len, parsed_value);
    int i = 0;
    for (i=0; i<asize; i++){
        lis[i] = a[i];
    } 
    int j;
    for (j=0; j<bsize; i++, j++){
        lis[i] = b[j];
    } 
    return lis;
}


char *parseds_to_string(parsed_value *ps, int count){
    char *ret = NEW(count, char);
    int i;
    for(i=0; i<count; i++){
        if (ps[i].type != CHAR)
            return (char *)-1;
        ret[i] = *(char*)ps[i].x;
    }

    return ret;
}

parsed fail(char *stream, int index){
    parsed cheese = {0, NULL, 0, index};
    return cheese;
}

parser any(){
    parsed anyer(char *stream, int index, parser p){
        parsed_value *f = NEW(1, parsed_value);
        f[0].type = CHAR;
        f[0].x = NEW(1, char);
        *(char *)(f[0].x) = stream[index];
        parsed cheese = {1, f, 1, index+1};
        return cheese;
    }
    parser x;
    x.parse = anyer;
    return x;
}

parser character(char c){
    parsed characterer(char *stream, int index, parser p){
        if(stream[index] == p.c){
            parsed_value *f = NEW(1, parsed_value);
            f[0].type = CHAR;
            f[0].x = NEW(1, char);
            *(char *)(f[0].x) = p.c;
            parsed cheese = {1, f, 1, index+1};
            return cheese;
        } else {
            return fail(stream, index);
        }
    }
    parser x;
    x.parse = characterer;
    x.c = c;
    return x;
}


parser one_of(char *cs){    
    parsed one_ofer(char *stream, int index, parser p){  
        int len = strlen(p.cs);
        int i=0;
        parsed o;
        do {
            parser x = character(p.cs[i]);
            o = x.parse(stream, index, x); 
            i++;
        } while(i<len && o.flag == 0);
        
        if (o.flag) {
            return o;
        }
        return fail(stream, index);
    }
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

parser many(parser p){
    parsed manyer(char *stream, int index, parser p){
        parsed o;
        parsed_value *ret = NULL;
        int i=0, j=0;
        int newindex = index;
        do{
            o = p.pser->parse(stream, newindex, *p.pser);
            if(o.flag){
                ret = append_parsed_values(ret, i, o.objs, o.count);
                newindex = o.index;
                i+= o.count;
                j++;
            }
        } while (o.flag);
        if (j > 0){
            parsed cheese = {1, ret, i, newindex};
            return cheese;
        } 
        return fail(stream, index);
    }
    parser *x = NEW(1, parser);
    *x = p;
    parser ret;
    ret.pser = x;
    ret.parse = manyer;
    return ret;
}

parser parse_not(parser p){
    parsed noter(char *stream, int index, parser p){
        parsed o = p.pser->parse(stream, index, *p.pser);
        if(o.flag == 0){
            parsed cheese = {1, NULL, 0, index};
            return cheese;    
        }
        return fail(stream, index);
    }
    parser *x = NEW(1, parser);
    *x = p;
    parser ret;
    ret.pser = x;
    ret.parse = noter;
    return ret;
}

parser sequence(parser *ps, int count){
    parsed sequencer(char *stream, int index, parser p)
    {
        parsed o;
        parsed_value *ret = NULL;
        int i=0, j=0;
        int newindex = index;
        do{
            o = p.pser[j].parse(stream, newindex, p.pser[j]);
            if(o.flag){
                j++;
                ret = append_parsed_values(ret, i, o.objs, o.count);
                newindex = o.index;
                i+= o.count;
            }
        } while (o.flag && j < p.count);
        if (j == p.count){
            parsed cheese = {1, ret, i, newindex};
            return cheese;
        } 
        return fail(stream, index);
    }

    parser *x = NEW(count, parser);
    int i;
    for(i=0; i<count; i++){
        x[i] = ps[i];
    }
    parser ret;
    ret.pser = x;
    ret.count = count;
    ret.parse = sequencer;
    return ret;
}




parser first_of(parser *ps, int count){
    parsed first_ofer(char *stream, int index, parser p)
    {
        parsed o;
        int i;
        for(i=0; i<p.count; i++){
            o = p.pser[i].parse(stream, index, p.pser[i]);
            if(o.flag){
                return o;
            }
        }
        return fail(stream, index);
    };
    parser *x = NEW(count, parser);
    int i;
    for(i=0; i<count; i++){
        x[i] = ps[i];
    }
    parser ret;
    ret.pser = x;
    ret.count = count;
    ret.parse = first_ofer;
    return ret;
}
 

parser symbol(){
    parsed symboler(char *stream, int index, parser x)
    {
        parser seq[2] = {parse_not(one_of(" )(\n")), any()};
        parser sym = many(sequence(seq, 2));
        parsed a = sym.parse(stream, index, sym);
        if (a.flag){
            parsed_value *hole = NEW(1, parsed_value);
            hole->type = SYMBOL;
            hole->x = parseds_to_string(a.objs, a.count);
            parsed cheese = {1, hole, 1, a.index};
            return cheese;
        }
        return a;
    }
    parser x;
    x.parse = symboler;
    return x;
}

//parser tokens(){

parser newline() {
    parsed newliner(char *stream, int index, parser x){
        parsed swiss = character('\n').parse(stream, index, character('\n'));
        if (!swiss.flag){
            return fail(stream, index);
        }
        parser parsl = many(character(' '));
        parsed parsley = parsl.parse(stream, swiss.index, parsl); 
        parsed cheese = {1, parsley.objs, parsley.count, parsley.index};
        return cheese;
    }
    parser x;
    x.parse = newliner;
    return x;
}



/*     parser whitespace = many(character(' ')); */
/*     parser open = character('('); */
/*     parser close = character(')'); */
/*     many(first_of( */
/* many(firstof(newline, symbol, whitespace, open, close)) */
