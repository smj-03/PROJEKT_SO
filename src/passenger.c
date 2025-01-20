//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "PASSENGER"
#define BIKE_PROB 4 // 25%

struct passenger {
    int id;
    _Bool has_bike;
    time_t *ts_onboarding;

    int sem_id_td_p;
    int sem_id_td_c;
};

void init_passenger(struct passenger *, int, int);

void board_train(const struct passenger *);

void exit_(const char *);

int main() {
    srand(time(NULL));

    const int sem_id_td_p = sem_alloc(SEM_T_DOOR_P, SEM_T_DOOR_NUM, IPC_CREAT | 0666);
    if (sem_id_td_p == -1) exit_("Semaphore Allocation Error");

    const int sem_id_td_c = sem_alloc(SEM_T_DOOR_C, SEM_T_DOOR_NUM, IPC_CREAT | 0666);
    if (sem_id_td_c == -1) exit_("Semaphore Allocation Error");

    struct passenger *this = malloc(sizeof(struct passenger));
    init_passenger(this, sem_id_td_p, sem_id_td_c);

    if (this == NULL) {
        log_error(PROCESS_NAME, errno, "Passenger Failure");
        exit(1);
    }

    log_message(
        PROCESS_NAME,
        "[NEW PASSENGER]   ID: %-8d BIKE: %-3d SEM_IDs: %d %d\n",
        this->id,
        this->has_bike,
        this->sem_id_td_p,
        this->sem_id_td_c
    );

    board_train(this);

    free(this);

    return 0;
}

void init_passenger(struct passenger *this, int sem_id_td_p, int sem_id_td_c) {
    this->id = getpid();
    this->ts_onboarding = NULL;
    this->sem_id_td_p = sem_id_td_p;
    this->sem_id_td_c = sem_id_td_c;

    if (get_random_number(0, BIKE_PROB - 1))
        this->has_bike = 0;
    else
        this->has_bike = 1;
}

void board_train(const struct passenger *this) {
    sem_wait(this->sem_id_td_p, this->has_bike, 0);

    log_message(PROCESS_NAME,
        "[BOARDING]   ID: %-8d BIKE: %-3d\n",
        this->id,
        this->has_bike);

    sleep(3);
    sem_post(this->sem_id_td_c, this->has_bike);
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
