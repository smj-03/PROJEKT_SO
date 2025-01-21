//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "TRAIN"

struct params {
    int sem_id_td_p;
    int sem_id_td_c;
    int shm_id_td_1;
    int shm_id_td_2;
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
    //     "[INIT]   ID: %-8d SEM_IDs: %d %d\n",
    //     this->id,
    //     this->sem_id_td_p,
    //     this->sem_id_td_c);

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
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    params->mutex = &mutex;

    const int sem_id_td_p = sem_alloc(SEM_T_DOOR_P_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_p == IPC_ERROR) exit_("Semaphore Allocation Error");
    params->sem_id_td_p = sem_id_td_p;

    const int sem_id_td_c = sem_alloc(SEM_T_DOOR_C_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_c == IPC_ERROR) exit_("Semaphore Allocation Error");
    params->sem_id_td_c = sem_id_td_c;

    const int shm_id_td_1 = shared_block_alloc(SHM_TRAIN_DOOR_1_KEY, TRAIN_P_LIMIT * sizeof(int), IPC_CREATE);
    if (shm_id_td_1 == IPC_ERROR) exit_("Shared Memory Allocation Error");
    params->shm_id_td_1 = shm_id_td_1;

    const int shm_id_td_2 = shared_block_alloc(SHM_TRAIN_DOOR_2_KEY, TRAIN_B_LIMIT * sizeof(int), IPC_CREATE);
    if (shm_id_td_2 == IPC_ERROR) exit_("Shared Memory Allocation Error");
    params->shm_id_td_2 = shm_id_td_2;
}

void init_train(struct train *this) {
    this->id = getpid();
    this->passenger_count = 0;
    this->bike_count = 0;
}

void *open_doors(void *_args) {
    struct thread_args *args = _args;
    struct params *params = args->params;
    struct train *this = args->this;
    log_message(PROCESS_NAME, "[THREAD] Doors %d\n", args->door_number + 1);

    while (1) {
        const int post_res = sem_post(params->sem_id_td_p, args->door_number);
        if (post_res == IPC_ERROR) exit_("Semaphore Init Post");

        sem_wait(params->sem_id_td_c, args->door_number, 0);

        pthread_mutex_lock(params->mutex);
        this->passenger_count++;
        if (args->door_number == 1) this->bike_count++;
        log_message(PROCESS_NAME,
                    "[BOARDING] Passenger has entered. P: %d, B: %d\n",
                    this->passenger_count,
                    this->bike_count);
        pthread_mutex_unlock(params->mutex);
    }
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
