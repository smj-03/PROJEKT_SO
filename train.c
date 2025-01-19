//
// Created by Szymon on 1/19/2025.
//
#include "utilities.h"

#define PROCESS_NAME "TRAIN"

#define TRAIN_SEMAPHORES 2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct train {
    int id;
    int passenger_count;
    int bike_count;

    int sem_id;
};

void init_train(struct train *, int);

void *open_doors_1(void *);

void *open_doors_2(void *);

void exit_(const char *);

int main(int argc, char *argv[]) {
    const key_t train_key = ftok(".", "TRAIN");
    if (train_key == -1) exit_("Key Creation");

    const int train_sem_id = semget(train_key, TRAIN_SEMAPHORES, IPC_CREAT | 0666);
    if (train_sem_id == -1) exit_("Semaphore Allocation Error");

    struct train *this = malloc(sizeof(struct train));
    init_train(this, train_sem_id);

    if (this == NULL) exit("Train Creation");

    log_message(
        PROCESS_NAME,
        "[NEW TRAIN]   ID: %-8d SEM_ID: %d\n",
        this->id,
        this->sem_id);

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

void init_train(struct train *this, int sem_id) {
    this->id = getpid();
    this->passenger_count = 0;
    this->bike_count = 0;
    this->sem_id = sem_id;
}

void *open_doors_1(void *this) {
    log_message(PROCESS_NAME, "Opening doors 1\n");
    while (1);
}

void *open_doors_2(void *this) {
    log_message(PROCESS_NAME, "Opening doors 2\n");
    while (1);
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
