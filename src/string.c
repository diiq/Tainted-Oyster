#include "oyster.h"
#include <stdlib.h>
#include <string.h>

oyster *make_character(char c)
{
    oyster *ret = make_oyster(sym_id_from_string("character"));
    oyster_assign_value(ret, (void *)c);
    oyster_assign_gc(ret, 0);
    return ret;
}


oyster *make_string(char *str)
{
    oyster *ret = make_oyster(sym_id_from_string("c-string"));
    oyster_assign_value(ret, malloc(sizeof(char)*(strlen(str)+1)));
    strcpy(oyster_value(ret), str);
    oyster_assign_gc(ret, 2);
    return ret;
}

char *string_of(oyster * str)
{
    return (char *)oyster_value(str);
}

oyster * builtin_string_first(machine *m){
    ARG(str);
    return make_character(*string_of(str));
}

oyster *builtin_string_rest(machine *m){
    // SUPER DUPER UBER INNEFFICIENT! WOWZA!
    ARG(str);
    char *stra = string_of(str);
    if (*(stra+1) == '\0')
        return nil();
    return make_string(stra+1);
}

void add_builtin_strings(machine *m)
{
    add_builtin("string-first", list(1, arg("str")), builtin_string_first, m);
    add_builtin("string-rest", list(1, arg("str")), builtin_string_rest, m);
}
