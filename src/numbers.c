#include "oyster.h"
#include <stdlib.h>

// So this is sort of a testing grounds for FFI; wondering what it takes to 
// add a new set of functions to Oyster from the outside. So far, I've had to 
// edit:
//    the parser --- not a common case, and will improve with the circular parser
//    the printer --- again, will improve with the circular printer
//    the initializing-function for the machine --- this is what will 
// have to be magicked away

oyster *make_number(int num)
{
    oyster *ret = make_oyster(sym_id_from_string("number"));
    ret->in->value = NEW(number);
    incref(ret->in->value);
    ((number *)ret->in->value)->num = num;
    return ret;
}

int number_of(oyster *o)
{
    return ((number *)oyster_value(o))->num;
}


void number_free(number *num)
{
    free(num);
}

oyster *builtin_plus(machine *m)
{
    ARG(a);
    ARG(b);
    return make_number(number_of(a) + number_of(b));
}

oyster *builtin_minus(machine *m)
{
    ARG(a);
    ARG(b);
    return make_number(number_of(a) - number_of(b));
}

oyster *builtin_multiply(machine *m)
{
    ARG(a);
    ARG(b);
    return make_number(number_of(a) * number_of(b));
}

oyster *builtin_divide(machine *m)
{
    ARG(a);
    ARG(b);
    return make_number(number_of(a) / number_of(b));
}

void add_builtin_numbers(machine *m)
{
    add_builtin("binary-+", list(2, arg("a"), arg("b")), builtin_plus, m);
    add_builtin("binary--", list(2, arg("a"), arg("b")), builtin_minus, m);
    add_builtin("binary-*", list(2, arg("a"), arg("b")), builtin_multiply, m);
    add_builtin("binary-/", list(2, arg("a"), arg("b")), builtin_divide, m);
}
