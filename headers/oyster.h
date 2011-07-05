#ifndef OYSTERH
#define OYSTERH

#define UNTO_DUST 0
#define NEW(x) malloc(sizeof(x))

#include <glib.h>

typedef struct table table;

typedef struct oyster oyster;
typedef struct cons_cell cons_cell;
typedef struct inner inner;

typedef struct frame frame;
typedef struct machine machine;
typedef struct instruction instruction;



//--------------------------------- Memory -----------------------------------//
// This does clever stuff to make reference counting less painful; but
//it's pretty painful anyway.

struct memorable{
    void (*inc)(void *);
    void (*dec)(void *);
};

void incref(void *x);
void decref(void *x);



//--------------------------------- Tables -----------------------------------//
// Tables are used to keep track of symbol-bindings --- in individual
// objects, in scopes, and during parsing.
//
// Tables are gonna change in implementation, for the sake of speed,
// but this is the interface to use. It's gon' be a challenge to cope
// with union and combine in an efficient way.

struct table{
    void (*incref)(table *x);
    void (*decref)(table *x);
    int ref;
    //    table_unit *root;
    GHashTable *it;
};

table *make_table();
void *table_get(int key, table *tab, int *err);
void table_put(int key, oyster *entry, table *tab);
void table_ref(table *x);
void table_unref(table *x);
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
table *binding_copy(table *x);

oyster *look_up(int sym, machine *m);
void set(int sym, oyster *val, machine *m, frame *f);
oyster *look_up_symbol(oyster *sym, machine *m);



//--------------------------------- Oysters ---------------------------------//
// An oyster is a little shell that pearls hide inside when they are scared.
// The shell of these oysters keeps track of the bindings of this object ---
// a scope limited to just this object's evaluation.
// Inside the shell is the inner in, the pearl, the goodie bag, the
// bilbo bagginses. This allows the same hobbit to have a different binding 
// in different circumstances -- the clothes fit the occasion.

struct cons_cell {
    void (*incref)(cons_cell *x);
    void (*decref)(cons_cell *x);
    int ref;

    oyster *car;
    oyster *cdr;
};

struct inner {
    void (*incref)(inner *x);
    void (*decref)(inner *x);
    int ref;

    table *info;
    int   type;
    union {
        int symbol_id;
        cons_cell *cons;
        oyster *(*built_in)(machine *m); 
        void *value;
    };
};

struct oyster {
    void (*incref)(oyster *x);
    void (*decref)(oyster *x);
    int ref;

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
void clean_up_oyster();

oyster *nil();
oyster *make_untyped_oyster();
void inner_ref(inner *x);
void inner_unref(inner *x);
oyster *make_oyster(int type);
void oyster_ref(oyster *x);
void oyster_unref(oyster *x);
oyster *oyster_copy(oyster *x, table *bindings);

oyster *make_symbol(int symbol_id);
oyster *make_cons(oyster *car, oyster *cdr);
void cons_ref(cons_cell *x);
void cons_unref(cons_cell *x);

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
    void (*incref)(frame *x);
    void (*decref)(frame *x);
    int ref;

    frame *below;
    table *scope;
    table *scope_to_be;
    instruction *current_instruction;
    oyster *signal_handler;
};

struct instruction {
    void (*incref)(instruction *x);
    void (*decref)(instruction *x);
    int ref;

    instruction *next;
    oyster *instruction;
    int flag;
};

struct machine {
    void (*incref)(machine *x);
    void (*decref)(machine *x);
    int ref;

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
void frame_ref(frame *x);
void frame_unref(frame *x);
void frame_free(frame *x);

machine *make_machine();
void machine_ref(machine *x);
void machine_free(machine *x);
void machine_unref(machine *x);
void push_current_frame(machine *m, table *scope);

instruction *make_instruction(oyster *ins, int flag, instruction *next);
void instruction_ref(instruction *x);
void instruction_unref(instruction *x);

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
oyster *run_built_in_function(oyster *o, machine *m);
void step_machine(machine *m);

void machine_print(machine *m);
void frame_print(frame *f);
void instruction_print(instruction *i);



//------------------------------ Builtins -------------------------------//

#define ARG(a) oyster *a = look_up(sym_id_from_string(#a), m)
#define sARG(a, an) oyster *a = look_up(sym_id_from_string(an), m)

oyster *make_builtin(oyster *(*func)(machine *m));
oyster *arg(char *name);
oyster *unev(char *name);
oyster *quot(char *name);
void add_builtins(machine *m);



//------------------------------- Numbers ------------------------------//

typedef struct {
    void (*inc)(void *);
    void (*dec)(void *);
    int ref;
    int num;
} number;

oyster *make_number(int num);

#endif

//------------------------------ Continuations --------------------------//

oyster *make_continuation(machine *m);

//--------------------------------- Signals -----------------------------//


