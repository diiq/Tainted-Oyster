#include "oyster.h"


void no_signal_handler(oyster * signal)
{
    printf
        ("\nthe trees shake in a strong wind\nbut the leaves have already fallen\n\n"
         "Error: Signal received but no signal handler is available.\n"
         "Error message follows:\n");
    oyster_print(car(signal));
    printf("\n");
}

oyster *make_signal(oyster * message, machine * m)
{
    return list(2, message, make_continuation(m));
}


void toss_signal(oyster * signal, machine * m)
{
    while (!m->paused && !m->current_frame->signal_handler)
        machine_pop_stack(m);
    if (m->paused && !m->current_frame->signal_handler) {
        no_signal_handler(signal);
        return;
    }
    if (m->paused)
        m->paused = 0;          //hax
    oyster *eval = list(2, m->current_frame->signal_handler, signal);
    incref(eval);
    push_new_instruction(m, eval,EVALUATE);
    decref(eval);
}
