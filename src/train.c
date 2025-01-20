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

    int sem_id_td_p;
    int sem_id_td_c;
};

void init_train(struct train *, int, int);

void *open_doors_1(void *);

void *open_doors_2(void *);

void exit_(const char *);

int main(int argc, char *argv[]) {
    const int sem_id_td_p = sem_alloc(SEM_T_DOOR_P, SEM_T_DOOR_NUM, IPC_CREAT | 0666);
    if (sem_id_td_p == -1) exit_("Semaphore Allocation Error");

    const int sem_id_td_c = sem_alloc(SEM_T_DOOR_C, SEM_T_DOOR_NUM, IPC_CREAT | 0666);
    if (sem_id_td_c == -1) exit_("Semaphore Allocation Error");

    struct train *this = malloc(sizeof(struct train));
    init_train(this, sem_id_td_p, sem_id_td_c);

    if (this == NULL) exit_("Train Creation");

    log_message(
        PROCESS_NAME,
        "[NEW TRAIN]   ID: %-8d SEM_IDs: %d %d\n",
        this->id,
        this->sem_id_td_p,
        this->sem_id_td_c);

    // 1 - baggage 2 - bike
    pthread_t id_thread_door_1, id_thread_door_2;

    if (pthread_create(&id_thread_door_1, NULL, open_doors_1, this)) exit_("Thread 1 Creation");
    if (pthread_create(&id_thread_door_2, NULL, open_doors_2, this)) exit_("Thread 2 Creation");

    if (pthread_join(id_thread_door_1, NULL)) exit_("Thread 1 Join");
    if (pthread_join(id_thread_door_2, NULL)) exit_("Thread 2 Join");

    if (!pthread_detach(id_thread_door_1)) exit_("Thread 1 Detach");
    if (!pthread_detach(id_thread_door_2)) exit_("Thread 2 Detach");

    free(this);
}

void init_train(struct train *this, int sem_id_td_p, int sem_id_td_c) {
    this->id = getpid();
    this->passenger_count = 0;
    this->bike_count = 0;
    this->sem_id_td_p = sem_id_td_p,
    this->sem_id_td_c = sem_id_td_c;
}

void *open_doors_1(void *_this) {
    log_message(PROCESS_NAME, "Opening doors 1\n");
    struct train *this = _this;
    while (1) {
        const int post_res = sem_post(this->sem_id_td_p, 0);
        if (post_res == -1) exit_("Semaphore Init Post");

        this->passenger_count++;

        sem_wait(this->sem_id_td_c, 0, 0);
        log_message(PROCESS_NAME, "[BOARDING] Passenger has entered\n");
    }
}

void *open_doors_2(void *_this) {
    log_message(PROCESS_NAME, "Opening doors 2\n");
    struct train *this = _this;
    while (1) {
        const int post_res = sem_post(this->sem_id_td_p, 1);
        if (post_res == -1) exit_("Semaphore Init Post");

        this->passenger_count++;
        this->bike_count++;

        sem_wait(this->sem_id_td_c, 1, 0);
        log_message(PROCESS_NAME, "[BOARDING] Passenger with a bike has entered\n");
    }
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
