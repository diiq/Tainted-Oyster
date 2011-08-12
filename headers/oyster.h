#ifndef OYSTERH
#define OYSTERH

#define UNTO_DUST 0

#include <glib.h>
#include <stdio.h>

typedef struct table table;
typedef struct table_entry table_entry;

typedef struct oyster oyster;
typedef struct cons_cell cons_cell;


typedef struct frame frame;
typedef struct machine machine;
typedef struct stack_trace stack_trace;

//------------------------------- Assemble -----------------------------------//
// These function bring all the rest together into a few simple calls.
void init_oyster();
void clean_up_oyster();
oyster *evaluate_scan(GScanner * in, int print);
oyster *evaluate_file(FILE * in, int print);
oyster *evaluate_string(char *str);



//------------------------------- Interpreter --------------------------------//
// These functions control individual steps of the oyster machine. 
void step_machine(machine * m);



//-------------------------------- The Machine ------------------------------//

machine *make_machine();
frame *machine_pop_stack(machine * m);
void push_new_instruction(machine * m, oyster * instruction, int flag);
frame *machine_pop_stack(machine * m);
oyster *machine_accumulator(machine *m);
int machine_paused(machine *m);
void machine_unpause(machine *m);
frame *machine_active_frame(machine *m);
void machine_free(machine *m);



//-------------------------------- Frames ------------------------------------//

enum instruction_flags {
    // This flag is set when a function call has been found.
    // The instruction will be a list of arguments; the accumulator
    // will be the function being applied.
    PREPARE_ARGUMENTS, 

    // This flag is set when an @ argument has been evaluated.
    // The instruction will be (arg-list lambda-list); the
    // accumulator will be the @ argument.
    ASTERPEND_CONTINUE,

    ATPEND_CONTINUE,
    ARGUMENT,
    ELIPSIS_ARGUMENT,
    ELIPSIS_ASTERPEND_CONTINUE,
    ELIPSIS_ATPEND_CONTINUE,
    EVALUATE,
    CONTINUE,
    APPLY_FUNCTION,
    PAUSE,
    HANDLE_SIGNALS
};

frame *make_frame(frame * below,
                  table * scope,
                  table * scope_to_be,
                  table * scope_below, oyster * instruction, int flag);

frame *frame_below(frame *f);
table *frame_scope(frame *f);
table *frame_scope_below(frame *f);
oyster *frame_instruction(frame *f);
int frame_flag(frame *f);
void frame_free(frame *f);

//------------------------ lists and how to use them -------------------------//

oyster *make_cons(oyster * car, oyster * cdr);
oyster *cheap_car(oyster * cons);
oyster *cheap_cdr(oyster * cons);
void set_car(oyster *cons, oyster *value);
void set_cdr(oyster *cons, oyster *value);
oyster *cons(oyster * car, oyster * cdr);
oyster *car(oyster * cons);
oyster *cdr(oyster * cons);

oyster *nil();
int nilp(oyster * x);
oyster *list(int count, ...);
oyster *append(oyster * a, oyster * b);
oyster *reverse(oyster * xs);
int oyster_length(oyster *xs);
oyster *ensure_list(oyster *xs);

void cons_cell_free(cons_cell *c);

//--------------------------------- Oysters ----------------------------------// 
// An oyster is a little shell that pearls hide inside when they are scared.
// The shell of these oysters keeps track of the bindings of this object ---
// a scope limited to just this object's evaluation.
// Inside the shell is the inner in, the pearl, the goodie bag, the
// bilbo bagginses. This allows the same hobbit to have a different binding 
// in different circumstances -- the clothes fit the occasion.

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
    REALLY,
    CLEAR,
    BUILT_IN_FUNCTION,
    MAX_PREDEF_SYMBOL,
};

oyster *make_untyped_oyster();
oyster *make_oyster(int type);
table *oyster_info(oyster *o);
void oyster_assign_info(oyster *o, table *t);

oyster * (*oyster_built_in(oyster *o)) (machine * m);
table *oyster_bindings(oyster * x);
void oyster_assign_bindings(oyster * x, table *value);
int oyster_type(oyster * x);
void oyster_assign_type(oyster *o, int type);
void *oyster_value(oyster * x);
void oyster_assign_value(oyster * x, void *value);
void oyster_assign_gc(oyster *o, int type);
oyster *make_symbol(int symbol_id);
int symbol_id(oyster *sym);
oyster *oyster_copy(oyster * x, table * new_bindings);
void oyster_add_to_bindings(int sym_id, oyster * val, oyster * x);
void oyster_free(oyster *o);


//--------------------------------- Scopes -----------------------------------//
// Looking up, leaking, setting, and packaging scopes, so that the right 
// symbol is bound to the right variable at the right time.
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


table *make_table();
table *table_copy(table * t);
void table_put_entry(int key, table_entry * entry, table * tab);
table_entry *table_get_entry(int key, table * tab, int *flag);
void table_put(int key, oyster * entry, table * tab);
oyster *table_get(int key, table * tab, int *flag);
int table_empty(table * tab);
void table_insert_or_replace(int sym_id, oyster *value, table *table);

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

#define ARG(a) oyster *a = look_up(sym_id_from_string(#a), machine_active_frame(m))
#define sARG(a, an) oyster *a = look_up(sym_id_from_string(an), machine_active_frame(m))

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

#define NEW(type) initialize_memory_object(sizeof(type),    \
                                           &type ## _free)

void *initialize_memory_object(size_t size, void *dec);
void incref(void *x);
void decref(void *x);



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

oyster *make_number(double num);
double number_of(oyster *o);
void add_builtin_numbers(machine *m);

//------------------------------- Strings -------------------------------//

oyster *make_character(char c);
oyster *make_string(char *str);
char *string_of(oyster * str);
void add_builtin_strings(machine *m);

//-------------------------------- Files -------------------------------//

oyster *make_file(FILE *in);
FILE *file_of(oyster *file);
void add_builtin_files(machine *m);

#endif
