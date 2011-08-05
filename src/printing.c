#include "oyster.h"

void table_print(table * x)
{
    if (!x) {
        printf("NONE\n");
        return;
    }
    int key;
    table_entry *value;
    table_loop(key, value, x->it) {
        printf("    %s : ", string_from_sym_id(key));
        oyster_print(value->it);
        printf("\n");
    } table_end_loop;
    printf("\n");
}

//


//



void list_print(oyster * o)
{
    incref(o);
    oyster *t;
    for(; !nilp(o); t = cheap_cdr(o), decref(o), o = t, incref(o)){
        oyster *c = cheap_car(o);
        incref(c);
        if (c->in->type == CONS){
            printf("(");
            oyster_print(c);
            printf(")");
        } else {
            oyster_print(c);
        }
        if (o->in->cons->cdr->in->type == CONS) //unacceptable!
            printf(" ");

        decref(c);
    }
    decref(o);
}

void oyster_print(oyster * o)
{
    incref(o);
    int type = o->in->type;
    switch (type) {
    case CONS:
        list_print(o);
        break;
    case SYMBOL:
        printf("%s", string_from_sym_id(o->in->symbol_id));
        break;
    case NIL:
        printf("nil");
        break;
    case BUILT_IN_FUNCTION:
        printf("[builtin]");
        break;
    default:

        // Eventually this will, instead, test the info table for a print
        // function, or the table of the type-symbol, or sommat. Dunno yet. 
        if(type == sym_id_from_string("table")){
            table_print(o->in->value);

        } else if(type == sym_id_from_string("number")){
            printf("%d", number_of(o));

        } else {
            printf("?(%d)", o->in->type);
        }
    }
    decref(o);
}
