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


//------------------------------- Assemble -----------------------------------//
// These function bring all the rest together into a few simple calls.
void init_oyster();
void clean_up_oyster();
oyster *evaluate_scan(GScanner * in, int print);
oyster *evaluate_string(char *str);



//------------------------------- Interpreter --------------------------------//
// These functions control individual steps of the oyster machine. 
void step_machine(machine * m);
void evaluate_oyster(frame * instruct, machine * m);

void push_argument(oyster * argument,
                   oyster * name, int flag, oyster * continu, machine * m);
void push_normal_argument(oyster * arg,
                          oyster * lambda_list,
                          oyster * arg_list, machine * m);
void argument_chain_link(oyster * lambda_list,
                         oyster * arg_list, machine * m);
int car_is_sym(oyster * x, int sym);
oyster *unevaluate_list(oyster * xs);
void push_bindings_to_scope(machine * m, oyster * o);



//-------------------------------- The Machine ------------------------------//
// I want you to be totally honest with me about how the machine makes you feel.
//
// Each frame represents an instructions and a scope in which
// that instruction should be carried out. Instructions, like slaves,
// come in chains:
//
//  Current frame
//   vvv     vvv
//  Another frame 
//   vvv     vvv
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

machine *make_machine();
frame *machine_pop_stack(machine * m);
void set_accumulator(machine * m, oyster * value);
frame *make_frame(frame * below,
                  table * scope,
                  table * scope_to_be,
                  table * scope_below, oyster * instruction, int flag);
void push_new_instruction(machine * m, oyster * instruction, int flag);
void push_instruction_list(machine * m,
                           oyster * ins,
                           table * scope, table * scope_below);



//------------------------ lists and how to use them -------------------------//
oyster *make_cons(oyster * car, oyster * cdr);
oyster *cheap_car(oyster * cons);
oyster *cheap_cdr(oyster * cons);
oyster *cons(oyster * car, oyster * cdr);
oyster *car(oyster * cons);
oyster *cdr(oyster * cons);

oyster *nil();
int nilp(oyster * x);
oyster *list(int count, ...);
oyster *append(oyster * a, oyster * b);
oyster *reverse(oyster * xs);



//--------------------------------- Oysters ----------------------------------// 
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

oyster *make_untyped_oyster();
oyster *make_oyster(int type);
int oyster_type(oyster * x);
oyster *make_symbol(int symbol_id);
oyster *oyster_copy(oyster * x, table * new_bindings);
void oyster_add_to_bindings(int sym_id, oyster * val, oyster * x);



//--------------------------------- Scopes -----------------------------------//
// Looking up, leaking, setting, and packaging scopes, so that the right 
// symbol is bound to the right variable at the right time.
table_entry *look_up_entry(int sym, frame * cur);
oyster *look_up(int sym, frame * cur);
void set(int sym, oyster * val, frame * f);
table *reify_scope(table * t, frame * f);



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

table_entry *make_table_entry(oyster * it);
table *make_table();
table *table_copy(table * t);
void table_put_entry(int key, table_entry * entry, table * tab);
table_entry *table_get_entry(int key, table * tab, int *flag);
void table_put(int key, oyster * entry, table * tab);
oyster *table_get(int key, table * tab, int *flag);
int table_empty(table * tab);

void leak(int sym, table * tab);
int leaked_p(int sym, table * tab);

#define table_loop(k, v, tab) {                                         \
    GHashTableIter iter;                                                \
    g_hash_table_iter_init(&iter, tab);                                 \
    int *TEMPVAR;                                                       \
    while(g_hash_table_iter_next(&iter, (void **)&TEMPVAR, (void **)&v)){ \
    k = GPOINTER_TO_INT(TEMPVAR);

#define table_end_loop }} do{}while(0)



//------------------------------ built in functions --------------------------//
// These are functions that push the core functions in the beginning.

#define ARG(a) oyster *a = look_up(sym_id_from_string(#a), m->now)
#define sARG(a, an) oyster *a = look_up(sym_id_from_string(an), m->now)

oyster *make_builtin(oyster * (*func) (machine * m));
oyster *arg(char *name);
oyster *unev(char *name);
oyster *quot(char *name);
void add_builtin(char *name,
                 oyster * lambda_list,
                 oyster * (*func) (machine * m), machine * m);
void add_builtins(machine * m);



//--------------------------- Continuations ----------------------------------//
// Man, nobody likes continuations. Who asked for these suckers?
machine *machine_copy(machine * m);
oyster *call_continuation(machine * m);
oyster *make_continuation(machine * m);



//--------------------------------- Memory -----------------------------------//
// Functions that handle memory management. These are gonna hafta change, as 
// the reference counting is a necessarily temporary arrangement.
struct memorable {
    void (*inc) (void *);
    void (*dec) (void *);
    int ref;
};

void *initialize_memory_object(size_t size, void *inc, void *dec);
void incref(void *x);
void decref(void *x);
void table_entry_ref(table_entry * x);
void table_entry_unref(table_entry * x);
void table_ref(table * x);
void table_unref(table * x);
void oyster_ref(oyster * x);
void oyster_unref(oyster * x);
void inner_ref(inner * x);
void inner_unref(inner * x);
void cons_cell_ref(cons_cell * x);
void cons_cell_unref(cons_cell * x);
void machine_ref(machine * x);
void machine_unref(machine * x);
void frame_ref(frame * x);
void frame_unref(frame * x);
/* void number_ref(number * n); */
/* void number_unref(number * n); */



//--------------------------------- Parsing ----------------------------------//
// ooh my well doncha know this is gon' change after a little while so don'
// you get too comfy there y'hear?

void init_symbol_table();
void free_symbol_table();
void add_symbol(int id, char *sym);
int sym_id_from_string(char *sym);
char *string_from_sym_id(int sym);
GScanner *string_scanner(char *text);
GScanner *file_scanner(char *file);
oyster *next_oyster(GScanner * in);



//-------------------------------- Printing ----------------------------------//
// Though processes are what we build, it is the evidence that gives our 
// structures meaning.
void table_print(table * x);
void machine_print(machine * m);
void print_stack_trace(machine * m);
void frame_print(frame * i, int print_scope);
void list_print(oyster * o);
void oyster_print(oyster * o);



//-------------------------------- Signals -----------------------------------//
// Repeat after me: A signal is not an error.
// A signal is a message, packaged with a continuation, that goes upstream.
void no_signal_handler(oyster * signal);
oyster *make_signal(oyster * message, machine * m);
void toss_signal(oyster * signal, machine * m);



//------------------------------- Numbers ------------------------------//

typedef struct {
    void (*inc) (void *);
    void (*dec) (void *);
    int ref;
    int num;
} number;

oyster *make_number(int num);

#endif
