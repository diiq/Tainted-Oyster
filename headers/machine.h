#ifndef MACHINEH
#define MACHINEH
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
    ELIPSIS_ASTERPEND_CONTINUE,
    ELIPSIS_ATPEND_CONTINUE,
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
#endif
