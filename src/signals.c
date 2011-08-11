#include "oyster.h"
#include <error.h>

void no_signal_handler(oyster * signal)
{
    int i = 0;
    print_stack_trace(oyster_value(table_get(sym_id_from_string("continuation"),
                                             car(cdr(signal))->bindings,
                                             &i)));
    printf
        ("    trees shake in a strong wind\n    but the leaves have already fallen\n\n\n"
         "Error: Signal received but no signal handler is available.\n"
         "Message follows:\n\n\n    ");
    oyster_print(car(signal));
    printf("\n\n\n");
    error(314, 0, "Exited with unhandled signal.");
    //    m->paused = 1;
}

oyster *make_signal(oyster * message, machine * m)
{
    return list(2, message, make_continuation(m));
}


void toss_signal(oyster * signal, machine * m)
{
    while (!machine_paused(m) && 
           frame_flag(machine_active_frame(m)) != HANDLE_SIGNALS)
        machine_pop_stack(m);
    frame *now = machine_active_frame(m);
    if (machine_paused(m) && 
        frame_flag(now) != HANDLE_SIGNALS) {
        no_signal_handler(signal);
        return;
    }
    if (machine_paused(m)) machine_unpause(m); //hax

    oyster *eval = list(2, frame_instruction(now), 
                        list(2, make_symbol(CLEAR), signal));
    incref(eval);
    push_new_instruction(m, eval, EVALUATE);
    decref(eval);
}
