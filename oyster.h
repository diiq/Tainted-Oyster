#ifndef OYSTERH
#define OYSTERH

#define UNTO_DUST 0
#define NEW(x) GC_MALLOC(sizeof(x))


#include "table.c"

typedef struct oyster oyster;
typedef struct cons cons;
typedef struct inner inner;

struct cons {
    oyster *car;
    oyster *cdr;
};

struct inner {
    table *info;
    int   type;
    union {
        int symbol_id;
        cons *cons;
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
};

void init_oyster();
oyster *make_untyped_oyster();
oyster *make_oyster(int type);
oyster *make_symbol(int symbol_id);

#endif
