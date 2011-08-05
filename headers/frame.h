#ifndef FRAMEH
#define FRAMEH

struct frame {
    void (*decref) (frame * x);
    int ref;

    frame *below;

    table *scope;
    table *scope_to_be;
    table *scope_below;

    oyster *instruction;
    int flag;
};


#endif
