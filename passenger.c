//
// Created by Szymon on 1/19/2025.
//
#include "utilities.h"

#define PROCESS_NAME "PASSENGER"
#define BIKE_PROB 4 // 25%

#define TRAIN_SEMAPHORES 2

struct passenger {
    int id;
    _Bool has_bike;
    time_t *ts_onboarding;

    int sem_id;
};

void init_passenger(struct passenger *, int);

void exit_(const char *);

int main() {
    srand(time(NULL));

    const key_t train_key = ftok(".", "TRAIN");
    if (train_key == -1) exit_("Key Creation");

    const int train_sem_id = sem_alloc(train_key, TRAIN_SEMAPHORES, IPC_CREAT | 0666);
    if (train_sem_id == -1) exit_("Semaphore Allocation Error");

    struct passenger *this = malloc(sizeof(struct passenger));
    init_passenger(this, train_sem_id);

    if (this == NULL) {
        log_error(PROCESS_NAME, errno, "Passenger Failure");
        exit(1);
    }

    log_message(
        PROCESS_NAME,
        "[NEW PASSENGER]   ID: %-8d BIKE: %-3d SEM_ID: %d\n",
        this->id,
        this->has_bike,
        this->sem_id
    );

    free(this);

    return 0;
}

void init_passenger(struct passenger *this, int sem_id) {
    this->id = getpid();
    this->ts_onboarding = NULL;
    this->sem_id = sem_id;

    if (get_random_number(0, BIKE_PROB - 1))
        this->has_bike = 0;
    else
        this->has_bike = 1;
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
