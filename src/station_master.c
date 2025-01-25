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

void handle_train(struct params *);

void init_params(struct params *);

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    struct params *params = malloc(sizeof(struct params));
    if (params == NULL) throw_error(PROCESS_NAME, "Params Error");

    init_params(params);

    while (1) handle_train(params);

    free(params);
    return 0;
}

void handle_train(struct params *params) {
    struct message message;
    if (message_queue_receive(params->msg_id_sm, &message, MSG_TYPE_FULL, 0) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Receive Error");

    sem_wait(params->sem_id_sm, 0, 0);

    int *shared_memory = params->shared_memory;
    const int read = shared_memory[TRAIN_NUM];
    const int train_id = shared_memory[read];
    shared_memory[TRAIN_NUM] = (shared_memory[TRAIN_NUM] + 1) % TRAIN_NUM;

    if (kill(train_id, SIGCONT) == IPC_ERROR) throw_error(PROCESS_NAME, "SIGCONT Error");

    log_message(PROCESS_NAME, "[ANNOUNCEMENT] Train %d has arrived!\n", train_id);

    sleep(TRAIN_DEPART_TIME);

    log_message(PROCESS_NAME, "[ANNOUNCEMENT] Train %d is ready to depart!\n", train_id);

    if (kill(train_id, SIGUSR1) == IPC_ERROR) throw_error(PROCESS_NAME, "SIGUSR1 Error");

    sem_wait(params->sem_id_sm, 2, 0);

    log_message(PROCESS_NAME, "[ANNOUNCEMENT] Train %d has departed!\n", train_id);

    sem_post(params->sem_id_sm, 0);

    message.mtype = MSG_TYPE_EMPTY;
    if (message_queue_send(params->msg_id_sm, &message) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Send Error");
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
