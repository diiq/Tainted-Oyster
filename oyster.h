#ifndef OYSTERH
#define OYSTERH

#define UNTO_DUST 0
#define NEW(x) malloc(sizeof(x))

#include <glib.h>

typedef struct table_unit_struct table_unit;

typedef struct oyster oyster;
typedef struct cons_cell cons_cell;
typedef struct inner inner;

typedef struct frame frame;
typedef struct machine machine;
typedef struct instruction instruction;

//--------------------------------- Tables -----------------------------------//
// Tables are used to keep track of symbol-bindings --- in individual
// objects, in scopes, and during parsing.
//
// Tables are gonna change in implementation, for the sake of speed,
// but this is the intervace to use. It's gon' be a challenge to cope
// with union and combine in an efficient way.

struct table_unit_struct {
    table_unit *next;
    int key;
    void *value;
};

typedef struct {
    //    table_unit *root;
    GHashTable *it;
} table;

table *make_table();
void *table_get(int key, table *tab, int *err);
void table_put(int key, void *entry, table *tab);
//int table_rev_lookup(void *value, int (equal)(void *a, void *b), table *tab, int *err);
//void table_remove(int key, table *tab);
int table_empty(table *tab);

#define table_loop(k, v, tab) {                                         \
    GHashTableIter iter;                                                \
    g_hash_table_iter_init(&iter, tab->it);                             \
    int *TEMPVAR;                                                       \
    while(g_hash_table_iter_next(&iter, (void **)&TEMPVAR, (void **)&v)){ \
    k = GPOINTER_TO_INT(TEMPVAR);



#define table_end_loop }} do{}while(0)

//--------------------------------- Bindings --------------------------------//
// Bindings are just tables with symbol_ids as keys and oysters as values.

oyster *leak();
int leaked_p(oyster *x);
table *binding_combine(table *a, table *b, 
                       table *newa, table *newb);

table *binding_union(table *a, table *b);

oyster *look_up(int sym, machine *m);
void set(int sym, oyster *val, machine *m, frame *f);
//--------------------------------- Oysters ---------------------------------//
// An oyster is a little shell that pearls hide inside when they are scared.
// The shell of these oysters keeps track of the bindings of this object ---
// a scope limited to just this object's evaluation.
// Inside the shell is the inner in, the pearl, the goodie bag, the
// bilbo bagginses. This allows the same hobbit to have a different binding 
// in different circumstances -- the clothes fit the occasion.

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
        void (*built_in)(machine *m); 
        void *value;
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
    ASTERIX,
    COMMA,
    CLEAR,
    BUILT_IN_FUNCTION,
    MAX_PREDEF_SYMBOL,
};

void init_oyster();
oyster *make_untyped_oyster();
oyster *make_oyster(int type);
oyster *oyster_copy(oyster *x);

oyster *make_symbol(int symbol_id);
oyster *make_cons(oyster *car, oyster *cdr);

int oyster_type(oyster *x);
int nilp(oyster *x);

oyster *cheap_car(oyster *cons);
oyster *cheap_cdr(oyster *cons);
oyster *cons(oyster *car, oyster *cdr);
oyster *car(oyster *cons);
oyster *cdr(oyster *cons);

oyster *cheap_list(int count, ...);
oyster *list(int count, ...);
oyster *cheap_append(oyster *a, oyster *b);
oyster *append(oyster *a, oyster *b);
oyster *reverse(oyster *xs);

void oyster_print(oyster *x);

//-------------------------------- The Machine ------------------------------//
// I want you to be totally honest with me about how the machine makes you feel.
// Frames come in stacks:
//
//  Current frame
//   vvvbelowvvv
//  Another frame
//   vvvbelowvvv
//  Da base frame
//
// Each frame represents a set of instructions and a scope in which
// those instructions should be carriet out. Instructions, like slaves,
// come in chains:
//
//  Current frame -> current instruction -> next instruction -> null
//   vvvbelowvvv
//  Another frame -> 'current' instruction -> null
//   vvvbelowvvv
//  Da base frame -> null
//
// Most instructions are simply oysters waiting to be evaluated as code.
// Some, though, are flagged as special instructions to the machine --- 
// adding values to the scope-to-be, handling special arguments to functions,
// and so on.

struct frame {
    frame *below;
    table *scope;
    table *scope_to_be;
    instruction *current_instruction;
};

struct instruction {
    instruction *next;
    oyster *instruction;
    int flag;
};

struct machine {
    frame *current_frame;
    frame *base_frame;
    oyster *accumulator;
    int paused;
};

enum instruction_flags {
    ASTERPEND_CONTINUE,
    ATPEND_CONTINUE,
    ARGUMENT,
    ELIPSIS_ARGUMENT,
    EVALUATE,
    CONTINUE,
    APPLY_FUNCTION,
    PREPARE_ARGUMENTS
};

frame *make_frame(table *scope, frame *below);
machine *make_machine();
instruction *make_instruction(oyster *ins, int flag, instruction *next);
oyster *instruction_object(instruction *i);

int asterix_p(oyster *x);
int atpend_p(oyster *x);
int comma_p(oyster *x);
int elipsis_p(oyster *x);
instruction *argument_chain_link(oyster *lambda_list, 
                                 oyster *arg_list, 
                                 instruction *chain);
oyster *unevaluate_list(oyster *xs);

instruction *machine_current_instruction(machine *m);
void machine_pop_stack(machine *m);
void run_built_in_function(oyster *o, machine *m);
void step_machine(machine *m);

void machine_print(machine *m);
void frame_print(frame *f);
void instruction_print(instruction *i);
#endif

