//
// Created by Szymon on 1/21/2025.
//

#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "STATION MASTER"

struct params {
    int sem_id_sm;
    int msg_id_sm;
    int *shared_memory;
};

void init_params(struct params *);

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "[INIT] STATION MASTER PID: %d\n", getpid());

    struct params *params = malloc(sizeof(struct params));
    if (params == NULL) throw_error(PROCESS_NAME, "Params Error");

    init_params(params);

    while (1);

    free(params);
    return 0;
}

void init_params(struct params *params) {
    const int sem_id_sm = sem_alloc(SEM_STATION_MASTER_KEY, 3, IPC_GET);
    if (sem_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");
    params->sem_id_sm = sem_id_sm;

    int *shared_memory = shared_block_attach(SHM_STATION_MASTER_KEY, (TRAIN_NUM + 2) * sizeof(int));
    if (shared_memory == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->shared_memory = shared_memory;

    const int msg_id_sm = message_queue_alloc(MSG_STATION_MASTER_KEY,IPC_GET);
    if (msg_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");
    params->msg_id_sm = msg_id_sm;
}
