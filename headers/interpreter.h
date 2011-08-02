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

void elipsis_argument(oyster *arg_list, oyster * lambda,
                      oyster *so_far, machine * m);
