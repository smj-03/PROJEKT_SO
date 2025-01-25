//
// Created by Szymon on 1/21/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "CONDUCTOR"

int main(int argc, char *argv[]) {

    log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    struct passenger_stack_1 *stack_1 = shared_block_attach(SHM_TRAIN_STACK_1_KEY, sizeof(struct passenger_stack_1));
    if (stack_1 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");

    struct passenger_stack_2 *stack_2 = shared_block_attach(SHM_TRAIN_STACK_2_KEY, sizeof(struct passenger_stack_2));
    if (stack_2 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");

    pause();

    // while(1) {
    //     if(VERBOSE_LOGS) {
    //         sleep(5);
    //         log_message(PROCESS_NAME, "[STACK 1] Top: %d [0]: %d\n", stack_1->top, stack_1->data[0]);
    //         log_message(PROCESS_NAME, "[STACK 2] Top: %d [0]: %d\n", stack_2->top, stack_2->data[0]);
    //     }
    // }

    return 0;
}
