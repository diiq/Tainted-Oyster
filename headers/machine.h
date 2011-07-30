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


// Stuff everybody wants:

// Stuff some people want:
machine *machine_copy(machine * m);
void set_accumulator(machine * m, oyster * value);



void push_new_instruction(machine * m, oyster * instruction, int flag);
void push_instruction_list(machine * m,
                           oyster * ins,
                           table * scope, table * scope_below);

//m->current_frame
//m->base_frame
//m->copy_frame


#endif
