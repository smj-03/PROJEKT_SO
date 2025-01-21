//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "TRAIN"

struct params {
    int sem_id_td_p;
    int sem_id_td_c;
    int msg_id_td_1;
    int msg_id_td_2;
    int *shared_memory_1;
    int *shared_memory_2;
    pthread_mutex_t *mutex;
};

struct train {
    int id;
    int passenger_count;
    int bike_count;
};

struct thread_args {
    int door_number;
    struct train *this;
    struct params *params;
};

void init_params(struct params *);

void init_train(struct train *);

void *open_doors(void *);

void exit_(const char *);

int main(int argc, char *argv[]) {
    struct params *params = malloc(sizeof(struct params));
    if (params == NULL) exit_("Params Error");

    struct train *this = malloc(sizeof(struct train));
    if (this == NULL) exit_("Train Error");

    init_params(params);

    init_train(this);

    // 1 - baggage 2 - bike
    pthread_t id_thread_door_1, id_thread_door_2;

    struct thread_args *args_1 = malloc(sizeof(struct thread_args));
    if (args_1 == NULL) exit_("Thread Arguments Creation");
    args_1->door_number = 0;
    args_1->this = this;
    args_1->params = params;

    struct thread_args *args_2 = malloc(sizeof(struct thread_args));
    if (args_2 == NULL) exit_("Thread Arguments Creation");
    args_2->door_number = 1;
    args_2->this = this;
    args_2->params = params;

    // log_message(
    //     PROCESS_NAME,
    //     "[INIT]   ID: %-8d SEM_IDs: %d %d MSG_IDs: %d %d\n",
    //     this->id,
    //     params->sem_id_td_p,
    //     params->sem_id_td_c,
    //     params->msg_id_td_1,
    //     params->msg_id_td_2
    // );


    if (pthread_create(&id_thread_door_1, NULL, open_doors, args_1)) exit_("Thread 1 Creation");
    if (pthread_create(&id_thread_door_2, NULL, open_doors, args_2)) exit_("Thread 2 Creation");

    if (pthread_join(id_thread_door_1, NULL)) exit_("Thread 1 Join");
    if (pthread_join(id_thread_door_2, NULL)) exit_("Thread 2 Join");

    if (!pthread_detach(id_thread_door_1)) exit_("Thread 1 Detach");
    if (!pthread_detach(id_thread_door_2)) exit_("Thread 2 Detach");

    free(this);
    free(params);
    free(args_1);
    free(args_2);
    shared_block_destroy(SHM_TRAIN_DOOR_1_KEY);
    shared_block_destroy(SHM_TRAIN_DOOR_2_KEY);
}

void init_params(struct params *params) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    params->mutex = &mutex;

    const int sem_id_td_p = sem_alloc(SEM_T_DOOR_P_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_p == IPC_ERROR) exit_("Semaphore Allocation Error");
    params->sem_id_td_p = sem_id_td_p;

    const int sem_id_td_c = sem_alloc(SEM_T_DOOR_C_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_c == IPC_ERROR) exit_("Semaphore Allocation Error");
    params->sem_id_td_c = sem_id_td_c;

    int *shared_memory_1 = shared_block_attach(SHM_TRAIN_DOOR_1_KEY, (TRAIN_P_LIMIT + 2) * sizeof(int));
    if (shared_memory_1 == NULL) exit_("Shared Memory Attach Error");
    params->shared_memory_1 = shared_memory_1;

    int *shared_memory_2 = shared_block_attach(SHM_TRAIN_DOOR_2_KEY, (TRAIN_B_LIMIT + 2) * sizeof(int));
    if (shared_memory_2 == NULL) exit_("Shared Memory Attach Error");
    params->shared_memory_2 = shared_memory_2;

    const int msg_id_td_1 = message_queue_alloc(MSG_TRAIN_DOOR_1_KEY,IPC_GET);
    if (msg_id_td_1 == IPC_ERROR) exit_("Message Queue Allocation Error");
    params->msg_id_td_1 = msg_id_td_1;

    const int msg_id_td_2 = message_queue_alloc(MSG_TRAIN_DOOR_2_KEY,IPC_GET);
    if (msg_id_td_2 == IPC_ERROR) exit_("Message Queue Allocation Error");
    params->msg_id_td_2 = msg_id_td_2;
}

void init_train(struct train *this) {
    this->id = getpid();
    this->passenger_count = 0;
    this->bike_count = 0;
}

void *open_doors(void *_args) {
    const struct thread_args *args = _args;
    const struct params *params = args->params;
    struct train *this = args->this;
    log_message(PROCESS_NAME, "[THREAD] Doors %d\n", args->door_number + 1);

    while (1) {
        sem_post(params->sem_id_td_p, args->door_number);
        sem_wait(params->sem_id_td_c, args->door_number, 0);

        struct message message;
        const int msg_id = args->door_number ? params->msg_id_td_2 : params->msg_id_td_1;
        if (message_queue_receive(msg_id, &message, MSG_TYPE_FULL) == IPC_ERROR) exit_("Message Receive Error");

        int *shared_memory = args->door_number ? params->shared_memory_2 : params->shared_memory_1;
        const int limit = args->door_number ? TRAIN_B_LIMIT : TRAIN_P_LIMIT;

        const int read = shared_memory[limit];
        const int passenger_id = shared_memory[read];

        shared_memory[limit] = (shared_memory[limit] + 1) % limit;

        log_message(PROCESS_NAME, "[DOOR] Welcome Passenger %d! Bike: %d\n", passenger_id, msg_id);
        // TO DO: ADD ID TO STACK

        pthread_mutex_lock(params->mutex);
        this->passenger_count++;
        if (args->door_number == 1) this->bike_count++;
        log_message(PROCESS_NAME,
                    "[BOARDING] Passenger has entered. P: %d, B: %d\n",
                    this->passenger_count,
                    this->bike_count);
        pthread_mutex_unlock(params->mutex);

        message.mtype = MSG_TYPE_EMPTY;
        if(message_queue_send(msg_id, &message) == IPC_ERROR) exit_("Message Send Error");
    }
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
