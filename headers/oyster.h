#ifndef OYSTERH
#define OYSTERH

#define UNTO_DUST 0

#define NEW(type) initialize_memory_object(sizeof(type),            \
                                            &type ## _ref,          \
                                            &type ## _unref)        

#include <glib.h>

typedef struct table table;
typedef struct table_entry table_entry;

typedef struct oyster oyster;
typedef struct cons_cell cons_cell;
typedef struct inner inner;

typedef struct frame frame;
typedef struct machine machine;



//--------------------------------- Memory -----------------------------------//
// This does clever stuff to make reference counting less painful; but
//it's pretty painful anyway.

struct memorable {
    void (*inc) (void *);
    void (*dec) (void *);
    int ref;
};

void incref(void *x);
void decref(void *x);
void * initialize_memory_object(size_t size,     
                                void * inc,
                                void * dec);


//--------------------------------- Tables -----------------------------------//
// Tables are used to keep track of symbol-bindings --- in individual
// objects, in scopes, and during parsing.
//
// Tables also track leaks, which allows the use of symbol bindings from 
// higher scopes.

enum {
    TABLE_ENTRY_NOT_FOUND,
    TABLE_ENTRY_FOUND,
    TABLE_ENTRY_LEAKED,
};

struct table_entry {
    void (*incref) (table_entry * x);
    void (*decref) (table_entry * x);
    int ref;
    oyster *it;
};

struct table {
    void (*incref) (table * x);
    void (*decref) (table * x);
    int ref;
    GHashTable *it;
    GHashTable *leaked;
};

table *make_table();
table_entry *table_get_entry(int key, table * tab, int *flag);
void table_put_entry(int key, table_entry *entry, table *tab);
oyster *table_get(int key, table * tab, int *err);
void table_put(int key, oyster * ent, table * tab);
void table_ref(table * x);
void table_unref(table * x);

int leaked_p(int sym, table *tab);
int table_empty(table * tab);
void table_print(table * x);

#define table_loop(k, v, tab) {                                         \
    GHashTableIter iter;                                                \
    g_hash_table_iter_init(&iter, tab);                                 \
    int *TEMPVAR;                                                       \
    while(g_hash_table_iter_next(&iter, (void **)&TEMPVAR, (void **)&v)){ \
    k = GPOINTER_TO_INT(TEMPVAR);

#define table_end_loop }} do{}while(0)



//--------------------------------- Bindings --------------------------------//
// Bindings are just tables with symbol_ids as keys and oysters as values.

table_entry *look_up_entry(int sym, frame *cur);
oyster *look_up(int sym, frame *cur);
void set(int sym, oyster * val, frame * f);
table *table_copy(table *t);
table *reify_scope(table *t, frame *f);
oyster *look_up_symbol(oyster * sym, frame * f);



//--------------------------------- Oysters ---------------------------------//
// An oyster is a little shell that pearls hide inside when they are scared.
// The shell of these oysters keeps track of the bindings of this object ---
// a scope limited to just this object's evaluation.
// Inside the shell is the inner in, the pearl, the goodie bag, the
// bilbo bagginses. This allows the same hobbit to have a different binding 
// in different circumstances -- the clothes fit the occasion.

struct cons_cell {
    void (*incref) (cons_cell * x);
    void (*decref) (cons_cell * x);
    int ref;

    oyster *car;
    oyster *cdr;
};

struct inner {
    void (*incref) (inner * x);
    void (*decref) (inner * x);
    int ref;

    table *info;
    int type;
    union {
        int symbol_id;
        cons_cell *cons;
        oyster *(*built_in) (machine * m);
        void *value;
    };
};

struct oyster {
    void (*incref) (oyster * x);
    void (*decref) (oyster * x);
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
    JUST,
    CLEAR,
    BUILT_IN_FUNCTION,
    MAX_PREDEF_SYMBOL,
};

void init_oyster();
void clean_up_oyster();

oyster *nil();
oyster *make_untyped_oyster();
void inner_ref(inner * x);
void inner_unref(inner * x);
oyster *make_oyster(int type);
void oyster_ref(oyster * x);
void oyster_unref(oyster * x);
oyster *oyster_copy(oyster * x, table * bindings);
void oyster_add_to_bindings(int sym_id, oyster *val, oyster *x);

oyster *make_symbol(int symbol_id);

//----------------------- Cons tools -----------------------------//

oyster *make_cons(oyster * car, oyster * cdr);
void cons_cell_ref(cons_cell * x);
void cons_cell_unref(cons_cell * x);

int oyster_type(oyster * x);
int nilp(oyster * x);

oyster *cheap_car(oyster * cons);
oyster *cheap_cdr(oyster * cons);
oyster *cons(oyster * car, oyster * cdr);
oyster *car(oyster * cons);
oyster *cdr(oyster * cons);

oyster *cheap_list(int count, ...);
oyster *list(int count, ...);
oyster *cheap_append(oyster * a, oyster * b);
oyster *append(oyster * a, oyster * b);
oyster *reverse(oyster * xs);

void oyster_print(oyster * x);



//-------------------------------- The Machine ------------------------------//
// I want you to be totally honest with me about how the machine makes you feel.
//
// Each frame represents an instructions and a scope in which
// that instruction should be carried out. Instructions, like slaves,
// come in chains:
//
//  Current frame
//   vvvbelowvvv
//  Another frame 
//   vvvbelowvvv
//  Da base frame 
//
// Most instructions are simply oysters waiting to be evaluated as code.
// Some, though, are flagged as special instructions to the machine --- 
// adding values to the scope-to-be, handling special arguments to functions,
// and so on.

struct frame {
    void (*incref) (frame * x);
    void (*decref) (frame * x);
    int ref;

    frame *below;

    table *scope;
    table *scope_to_be;
    table *scope_below;

    oyster *instruction;
    int flag;
};

struct machine {
    void (*incref) (machine * x);
    void (*decref) (machine * x);
    int ref;

    frame *now;
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
    PREPARE_ARGUMENTS,
    PAUSE,
    HANDLE_SIGNALS
};

frame *make_frame(frame * below, 
                  table * scope, 
                  table * scope_to_be, 
                  table * scope_below, 
                  oyster *instruction,
                  int flag);
void frame_ref(frame * x);
void frame_unref(frame * x);
void frame_free(frame * x);

machine *make_machine();
frame *machine_pop_stack(machine * m);
void machine_ref(machine * x);
void machine_free(machine * x);
void machine_unref(machine * x);

void push_instruction_list(machine *m, 
                           oyster *ins,
                           table *scope,
                           table *scope_below);

void push_new_instruction(machine *m, oyster *instruction, int flag);

//-------------------------- The interpreter ----------------------------//


void step_machine(machine * m);
void evaluate_oyster(frame * instruct, machine * m);

int asterix_p(oyster * x);
int atpend_p(oyster * x);
int comma_p(oyster * x);
int elipsis_p(oyster * x);
void argument_chain_link(oyster * lambda_list,
                         oyster * arg_list, 
                         machine * m);


oyster *unevaluate_list(oyster * xs);
void push_bindings_to_scope(machine * m, oyster * o);

int car_is_sym(oyster * x, int sym);


void machine_print(machine * m);
void print_stack_trace(machine * m);
void frame_print(frame * f, int print_scope);


//------------------------------ Assemble -------------------------------//
// jesus what do these things have in common

oyster *evaluate_string(char *str);



//------------------------------ Builtins -------------------------------//

#define ARG(a) oyster *a = look_up(sym_id_from_string(#a), m->now)
#define sARG(a, an) oyster *a = look_up(sym_id_from_string(an), m->now)

oyster *make_builtin(oyster * (*func) (machine * m));
oyster *arg(char *name);
oyster *unev(char *name);
oyster *quot(char *name);
void add_builtins(machine * m);



//------------------------------- Numbers ------------------------------//

typedef struct {
    void (*inc) (void *);
    void (*dec) (void *);
    int ref;
    int num;
} number;

oyster *make_number(int num);



//------------------------------ Continuations --------------------------//

oyster *make_continuation(machine * m);



//--------------------------------- Signals -----------------------------//

oyster *make_signal(oyster * message, machine * m);
void toss_signal(oyster * signal, machine * m);

#endif
