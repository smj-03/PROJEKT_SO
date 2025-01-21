//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "TRAIN"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct train {
    int id;
    int passenger_count;
    int bike_count;

    // int sem_id_td_p;
    // int sem_id_td_c;
    // pthread_mutex_t* mutex;
};

struct thread_args {
    int door_number;
    int sem_id_td_p;
    int sem_id_td_c;
    pthread_mutex_t *mutex;
    struct train *this;
};

void *open_doors(void *);

void exit_(const char *);

int main(int argc, char *argv[]) {
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    const int sem_id_td_p = sem_alloc(SEM_T_DOOR_P_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_p == IPC_ERROR) exit_("Semaphore Allocation Error");

    const int sem_id_td_c = sem_alloc(SEM_T_DOOR_C_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_c == IPC_ERROR) exit_("Semaphore Allocation Error");

    struct train *this = malloc(sizeof(struct train));
    init_train(this, sem_id_td_p, sem_id_td_c);

    if (this == NULL) exit_("Train Creation");
    this->id = getpid();
    this->passenger_count = 0;
    this->bike_count = 0;

    struct thread_args *args_1 = malloc(sizeof(struct thread_args));
    if (args_1 == NULL) exit_("Thread Arguments Creation");
    args_1->door_number = 0;
    args_1->sem_id_td_p = sem_id_td_p;
    args_1->sem_id_td_c = sem_id_td_c;
    args_1->mutex = &mutex;
    args_1->this = this;

    struct thread_args *args_2 = malloc(sizeof(struct thread_args));
    if (args_2 == NULL) exit_("Thread Arguments Creation");
    args_2->door_number = 1;
    args_2->sem_id_td_p = sem_id_td_p;
    args_2->sem_id_td_c = sem_id_td_c;
    args_2->mutex = &mutex;
    args_2->this = this;


    // log_message(
    //     PROCESS_NAME,
    //     "[INIT]   ID: %-8d SEM_IDs: %d %d\n",
    //     this->id,
    //     this->sem_id_td_p,
    //     this->sem_id_td_c);

    // 1 - baggage 2 - bike
    pthread_t id_thread_door_1, id_thread_door_2;

    if (pthread_create(&id_thread_door_1, NULL, open_doors, args_1)) exit_("Thread 1 Creation");
    if (pthread_create(&id_thread_door_2, NULL, open_doors, args_2)) exit_("Thread 2 Creation");

    if (pthread_join(id_thread_door_1, NULL)) exit_("Thread 1 Join");
    if (pthread_join(id_thread_door_2, NULL)) exit_("Thread 2 Join");

    if (!pthread_detach(id_thread_door_1)) exit_("Thread 1 Detach");
    if (!pthread_detach(id_thread_door_2)) exit_("Thread 2 Detach");

    free(this);
    free(args_1);
    free(args_2);
    shared_block_destroy(SHM_TRAIN_DOOR_1_KEY);
    shared_block_destroy(SHM_TRAIN_DOOR_2_KEY);
}

void *open_doors(void *_args) {
    struct thread_args *args = _args;
    struct train *this = args->this;
    log_message(PROCESS_NAME, "[THREAD] Doors %d\n", args->door_number + 1);

    while (1) {
        const int post_res = sem_post(args->sem_id_td_p, args->door_number);
        if (post_res == IPC_ERROR) exit_("Semaphore Init Post");

        sem_wait(args->sem_id_td_c, args->door_number, 0);

        pthread_mutex_lock(args->mutex);
        this->passenger_count++;
        if(args->door_number == 1) this->bike_count++;
        log_message(PROCESS_NAME,
            "[BOARDING] Passenger has entered. P: %d, B: %d\n",
            this->passenger_count,
            this->bike_count);
        pthread_mutex_unlock(args->mutex);
    }
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
