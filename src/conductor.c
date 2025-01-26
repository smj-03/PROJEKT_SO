//
// Created by Szymon on 1/21/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "CONDUCTOR"

struct params {
    int sem_id_c;
    struct passenger_stack *stack_1;
    struct passenger_stack *stack_2;
};

void init_params(struct params *);

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    struct params *params = malloc(sizeof(params));
    if(params == NULL) throw_error(PROCESS_NAME, "Params Init Error");

    init_params(params);

    sem_wait(params->sem_id_c, 0, 0);
    log_message(PROCESS_NAME, "[%d][INFO] The doors have closed!\n", getppid());



    sem_post(params->sem_id_c, 1);

    free(params);
    pause();

    return 0;
}

void init_params(struct params *params) {
    const int sem_id_c = sem_alloc(SEM_CONDUCTOR_KEY, 2, IPC_GET);
    if (sem_id_c == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation");
    params->sem_id_c = sem_id_c;

    struct passenger_stack *stack_1 = shared_block_attach(
    SHM_TRAIN_STACK_1_KEY, sizeof(struct passenger_stack));
    if (stack_1 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->stack_1 = stack_1;

    struct passenger_stack *stack_2 = shared_block_attach(
        SHM_TRAIN_STACK_2_KEY, sizeof(struct passenger_stack));
    if (stack_2 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->stack_2 = stack_2;
}
