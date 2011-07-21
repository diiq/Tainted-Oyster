#include "oyster.h"


oyster *make_number(int num)
{
    oyster *ret = make_oyster(sym_id_from_string("number"));
    ret->in->value = NEW(struct number);
    ret->in->value->num = num;
    ret->in->value->inc = &number_ref;
    ret->in->value->inc = &number_unref;
    ret->in->value->ref = 1;
    return ret;
}
