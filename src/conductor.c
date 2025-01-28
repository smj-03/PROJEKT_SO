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
} *params;

void init_params();

void kick_passengers();

int main(int argc, char *argv[]) {
    if(VERBOSE_LOGS) log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    params = malloc(sizeof(params));
    if (params == NULL) throw_error(PROCESS_NAME, "Params Init Error");

    init_params(params);

    sem_wait(params->sem_id_c, 0, 0);
    log_message(PROCESS_NAME, "[%d][INFO] The doors have closed!\n", getppid());

    kick_passengers(params);

    sem_post(params->sem_id_c, 1);

    free(params);
    shared_block_detach(params->stack_1);
    shared_block_detach(params->stack_2);
    pause();

    return 0;
}

void init_params() {
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

void kick_passengers() {
    const int bikes_to_exit = params->stack_2->top - TRAIN_B_LIMIT;
    if (params->stack_2->top > TRAIN_B_LIMIT)
        for (int i = 0; i < bikes_to_exit; i++) {
            const int id_to_exit = pop(params->stack_2);
            if (id_to_exit == -1) throw_error(PROCESS_NAME, "Pop Error");
            kill(id_to_exit, SIGUSR1);
        }

    const int passengers_to_exit = params->stack_1->top - TRAIN_P_LIMIT;
    if (passengers_to_exit > 0)
        for (int i = 0; i < passengers_to_exit; i++) {
            const int id_to_exit = pop(params->stack_1);
            if (id_to_exit == -1) throw_error(PROCESS_NAME, "Pop Error");
            kill(id_to_exit, SIGUSR1);
        }
}
