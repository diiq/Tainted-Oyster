#include "oyster.h"
#include <stdlib.h>

// So this is sort of a testing grounds for FFI; wondering what it takes to 
// add a new set of functions to Oyster from the outside. So far, I've had to 
// edit:
//    the parser --- not a common case, and will improve with the circular parser
//    the printer --- again, will improve with the circular printer
//    the initializing-function for the machine --- this is what will 
// have to be magicked away

oyster *make_number(double num)
{
    oyster *ret = make_oyster(sym_id_from_string("number"));
    double *anum = malloc(sizeof(double));
    *anum = num;
    ret->in->value = anum;
    ret->in->gc_type = 2;
    return ret;
}

double number_of(oyster *num){
    return *(double *)num->in->value;
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

oyster *builtin_greater(machine *m)
{
    ARG(a);
    ARG(b);
    if (number_of(a) > number_of(b)){
        return arg("t");
    }
    return nil();
}

oyster *builtin_less(machine *m)
{
    ARG(a);
    ARG(b);
    if (number_of(a) < number_of(b)){
        return arg("t");
    }
    return nil();
}

void add_builtin_numbers(machine *m)
{
    add_builtin("binary-+", list(2, arg("a"), arg("b")), builtin_plus, m);
    add_builtin("binary--", list(2, arg("a"), arg("b")), builtin_minus, m);
    add_builtin("binary-*", list(2, arg("a"), arg("b")), builtin_multiply, m);
    add_builtin("binary-/", list(2, arg("a"), arg("b")), builtin_divide, m);
    add_builtin("binary-greater", list(2, arg("a"), arg("b")), builtin_greater, m);
    add_builtin("binary-less", list(2, arg("a"), arg("b")), builtin_less, m);
}
