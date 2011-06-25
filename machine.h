#ifndef MACHINEH
#define MACHINEH

typedef struct frame frame;
typedef struct machine machine;
typedef struct instruction instruction;

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
};

#endif

