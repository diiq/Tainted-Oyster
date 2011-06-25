#ifndef OYSTERH
#define OYSTERH

#define UNTO_DUST 0
#define NEW(x) GC_MALLOC(sizeof(x))


#include "table.c"

typedef struct oyster oyster;
typedef struct cons_cell cons_cell;
typedef struct inner inner;

struct cons_cell {
    oyster *car;
    oyster *cdr;
};

struct inner {
    table *info;
    int   type;
    union {
        int symbol_id;
        cons_cell *cons;
    };
};

struct oyster {
    table *bindings;
    inner *in;
};
    
// type/symbols:
enum {
    TYPE,
    SYMBOL,
    CONS,
    NIL,
    LEAKED,
    ATPEND,
    ELIPSIS,
    ASTERIX
};

void init_oyster();
oyster *make_untyped_oyster();
oyster *make_oyster(int type);
oyster *make_symbol(int symbol_id);

#endif
