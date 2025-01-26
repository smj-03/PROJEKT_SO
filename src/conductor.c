//
// Created by Szymon on 1/21/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "CONDUCTOR"

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    const struct passenger_stack_1 *stack_1 = shared_block_attach(
        SHM_TRAIN_STACK_1_KEY, sizeof(struct passenger_stack_1));
    if (stack_1 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");

    const struct passenger_stack_2 *stack_2 = shared_block_attach(
        SHM_TRAIN_STACK_2_KEY, sizeof(struct passenger_stack_2));
    if (stack_2 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");

    const int sem_id_c = sem_alloc(SEM_CONDUCTOR_KEY, 2, IPC_GET);
    if (sem_id_c == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation");

    sem_wait(sem_id_c, 0, 0);
    log_message(PROCESS_NAME, "[%d][INFO] The doors have closed!\n", getppid());

    sem_post(sem_id_c, 1);

    pause();

    return 0;
}
