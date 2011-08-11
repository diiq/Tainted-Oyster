// OK, fileio should sneak in under first and rest, which means that a file needs 

#include "oyster.h"
#include <stdlib.h>
#include <stdio.h>

oyster *make_file(FILE *in)
{
    oyster *ret = make_oyster(sym_id_from_string("c-infile"));
    oyster_assign_value(ret, in); // this is NO GOOD, the file may be freed afore closed!
    oyster_assign_gc(ret, 2);
    return ret;
}

FILE *file_of(oyster *fil){
    return (FILE *)oyster_value(fil);
}

oyster *builtin_open(machine *m){
    ARG(name);
    FILE *in = fopen(string_of(name), "r");
    if (in == NULL)
        toss_signal(make_signal(make_string("File open failure!"), m), m);
    return make_file(in);
}

oyster *builtin_get_char(machine *m){
    ARG(file);
    FILE *a = file_of(file);
    char c = fgetc(a);
    return make_character(c);
}

oyster *builtin_eofp(machine *m){
    ARG(c);
    char d = (char)oyster_value(c);
    if(d == EOF){
        return arg("t");
    } 
    return nil();
};

void add_builtin_files(machine *m)
{
    add_builtin("file-open", list(1, arg("name")), builtin_open, m);
    add_builtin("file-get-char", list(1, arg("file")), builtin_get_char, m);
    add_builtin("eof?", list(1, arg("c")), builtin_eofp, m);
    //add_builtin("file-close", list(1, arg("name")), builtin_open, m);
}

