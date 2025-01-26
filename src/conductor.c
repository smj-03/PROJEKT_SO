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
    if (params == NULL) throw_error(PROCESS_NAME, "Params Init Error");

    init_params(params);

    sem_wait(params->sem_id_c, 0, 0);
    log_message(PROCESS_NAME, "[%d][INFO] The doors have closed!\n", getppid());

    int exit_ids[TRAIN_MAX_CAPACITY];
    int exit_count = 0;

    const int bikes_to_exit = params->stack_2->top - TRAIN_B_LIMIT;
    if (params->stack_2->top > TRAIN_B_LIMIT)
        for (int i = 0; i < bikes_to_exit; i++) {
            const int id_to_exit = pop(params->stack_2);
            if (id_to_exit == -1) throw_error(PROCESS_NAME, "Pop Error");
            exit_ids[exit_count++] = id_to_exit;
        }

    const int passengers_to_exit = params->stack_1->top - exit_count - TRAIN_P_LIMIT;
    if (passengers_to_exit > 0)
        for (int i = 0; i < passengers_to_exit;) {
            const int id_to_exit = pop(params->stack_1);
            if (id_to_exit == -1) throw_error(PROCESS_NAME, "Pop Error");
            if (!has(exit_ids, id_to_exit, exit_count)) {
                exit_ids[exit_count++] = id_to_exit;
                i++;
            }
        }

    for (int i = 0; i < params->stack_1->top;) {
        int id = params->stack_1->data[i];
        if (!has(exit_ids, id, exit_count)) {
            params->stack_1->data[i] = id;
            i++;
        }
    }
    params->stack_1->top = TRAIN_P_LIMIT;

    for (int i = 0; i < exit_count; i++)
        kill(exit_ids[i], SIGUSR1);


    // else if (bikes_to_exit > 0) params->stack_1->top -= bikes_to_exit;

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
