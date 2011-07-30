
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

void machine_print(machine * m)
{
    frame *f = m->current_frame;
    printf("Now: ");
    frame_print(m->now, 1);
    if (m->now->instruction && !table_empty(m->now->instruction->bindings)) {
        printf(" with the bindings: \n");
        table_print(m->now->instruction->bindings);
    }
    while (f) {
        printf("vvv vvv vvv\n");
        printf("frame: ");
        frame_print(f, 1);
        f = f->below;
    }
    if (m->accumulator) {
        printf("accum: ");
        oyster_print(m->accumulator);
    }
    printf
        ("\n--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---\n\n\n\n");
}

void print_stack_trace(machine * m)
{
    printf("Now: ");
    frame_print(m->now, 1);

    frame *f = m->current_frame;
    while (f) {
        frame_print(f, 1);
        f = f->below;
    }
    if (m->accumulator) {
        printf("accum: ");
        oyster_print(m->accumulator);
    }
    printf
        ("\n--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---\n\n\n\n");
}

void frame_print(frame * i, int print_scope)
{
    char *flags[] = { 
        "ASTERPEND_CONTINUE",
        "ATPEND_CONTINUE",
        "ARGUMENT",
        "ELIPSIS_ARGUMENT",
        "ELIPSIS_ASTERPEND_CONTINUE",
        "ELIPSIS_ATPEND_CONTINUE",
        "EVALUATE",
        "CONTINUE",
        "APPLY_FUNCTION",
        "PREPARE_ARGUMENTS",
        "PAUSE",
        "HANDLE_SIGNALS"
    };

    printf(" --> ");
    printf("%s, ", flags[i->flag]);
    if (i->instruction)
        oyster_print(i->instruction);
    if (print_scope) {
        printf("\n Scope:\n");
        table_print(i->scope);
        printf("Upcoming scope:\n");
        table_print(i->scope_to_be);
    }
    printf("\n");
}

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
        printf("()");
        break;
    case BUILT_IN_FUNCTION:
        printf("[builtin]");
        break;
    default:
        printf("?(%d)", o->in->type);
        break;
    }
    decref(o);
}
