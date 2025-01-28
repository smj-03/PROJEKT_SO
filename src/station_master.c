//
// Created by Szymon on 1/21/2025.
//

#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "STATION MASTER"

struct params {
    int sem_id_sm;
    int sem_id_p;
    int msg_id_sm;
    int *shared_memory_train;
    int *shared_memory_counter;
} *params;

struct thread_args {
    int platform_id;
};

volatile int platform_closed = 0;

void handle_train();

void *close_platform(void *);

void init_params();

void kill_all_trains(int []);

int main(int argc, char *argv[]) {
    const int platform_id = atoi(argv[1]);

    int train_ids[TRAIN_NUM];
    for (int i = 0; i < TRAIN_NUM; i++)
        train_ids[i] = atoi(argv[i + 2]);

    if (VERBOSE_LOGS) log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    params = malloc(sizeof(struct params));
    if (params == NULL) throw_error(PROCESS_NAME, "Params Error");

    init_params(params);

    pthread_t close_thread_id;

    struct thread_args *close_args = malloc(sizeof(struct thread_args));
    if (close_args == NULL) throw_error(PROCESS_NAME, "Thread Arguments Creation");
    close_args->platform_id = platform_id;

    if (pthread_create(&close_thread_id, NULL, close_platform, close_args))
        throw_error(PROCESS_NAME, "Thread 1 Creation");

    const int *passenger_counter = params->shared_memory_counter;

    while (!platform_closed || passenger_counter[0]) {
        log_message(PROCESS_NAME, "[INFO] Passengers on the platform: %d\n", params->shared_memory_counter[0]);
        handle_train(params);
        // if(!platform_closed) kill(platform_id, SIGUSR1);
    }
    log_message(PROCESS_NAME, "[INFO] Passengers on the platform: %d\n", params->shared_memory_counter[0]);

    kill_all_trains(train_ids);
    sem_post(params->sem_id_p, 0);

    free(params);
    return 0;
}

void handle_train() {
    struct message message;
    if (message_queue_receive(params->msg_id_sm, &message, MSG_TYPE_FULL, 0) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Receive Error");

    sem_wait(params->sem_id_sm, 0, 0);

    int *shared_memory = params->shared_memory_train;
    const int read = shared_memory[TRAIN_NUM];
    const int train_id = shared_memory[read];
    shared_memory[TRAIN_NUM] = (shared_memory[TRAIN_NUM] + 1) % TRAIN_NUM;

    if (kill(train_id, SIGCONT) == IPC_ERROR) throw_error(PROCESS_NAME, "SIGCONT Error");

    log_warning(PROCESS_NAME, "[ANNOUNCEMENT] Train %d has arrived!\n", train_id);

    sleep(TRAIN_DEPART_TIME);

    log_warning(PROCESS_NAME, "[ANNOUNCEMENT] Train %d is ready to depart!\n", train_id);

    if (kill(train_id, SIGUSR1) == IPC_ERROR) throw_error(PROCESS_NAME, "SIGUSR1 Error");

    sem_wait(params->sem_id_sm, 2, 0);

    log_warning(PROCESS_NAME, "[ANNOUNCEMENT] Train %d has departed!\n", train_id);

    sem_post(params->sem_id_sm, 0);

    message.mtype = MSG_TYPE_EMPTY;
    if (message_queue_send(params->msg_id_sm, &message) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Send Error");
}

void *close_platform(void *_args) {
    const struct thread_args *args = _args;

    sleep(PLATFORM_CLOSE_AFTER);
    log_warning(PROCESS_NAME, "[INFO] Closing Platform!\n");
    kill(args->platform_id, SIGUSR2);

    platform_closed = 1;

    return NULL;
}

void init_params() {
    const int sem_id_sm = sem_alloc(SEM_STATION_MASTER_KEY, 3, IPC_GET);
    if (sem_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");
    params->sem_id_sm = sem_id_sm;

    const int sem_id_p = sem_alloc(SEM_PLATFORM_KEY, 1, IPC_GET);
    if (sem_id_p == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation");
    params->sem_id_p = sem_id_p;

    int *shared_memory_train = shared_block_attach(SHM_STATION_MASTER_TRAIN_KEY, (TRAIN_NUM + 2) * sizeof(int));
    if (shared_memory_train == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->shared_memory_train = shared_memory_train;

    const int msg_id_sm = message_queue_alloc(MSG_STATION_MASTER_KEY,IPC_GET);
    if (msg_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");
    params->msg_id_sm = msg_id_sm;

    int *shared_memory_counter = shared_block_attach(SHM_STATION_MASTER_PASSENGER_KEY, sizeof(int));
    if (shared_memory_counter == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->shared_memory_counter = shared_memory_counter;
}

void kill_all_trains(int train_ids[]) {
    for (int i = 0; i < TRAIN_NUM; i++)
        if (kill(train_ids[i], SIGKILL) == IPC_ERROR)
            log_message(PROCESS_NAME, "Failed to kill train process %d\n", train_ids[i]);
}
