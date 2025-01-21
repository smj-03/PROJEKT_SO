//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "PASSENGER"

struct passenger {
    int id;
    _Bool has_bike;
};

struct params {
    int sem_id_td_p;
    int sem_id_td_c;
};

void init_params(struct params *);

void init_passenger(struct passenger *);

void board_train(const struct passenger *, struct params *);

void exit_(const char *);

int main() {
    struct params *params = malloc(sizeof(struct params));
    if (params == NULL) {
        log_error(PROCESS_NAME, errno, "Params Error");
        exit(1);
    }

    struct passenger *this = malloc(sizeof(struct passenger));
    if (this == NULL) {
        log_error(PROCESS_NAME, errno, "Passenger Error");
        exit(1);
    }

    init_params(params);
    init_passenger(this);
    board_train(this, params);

    // log_message(
    //     PROCESS_NAME,
    //     "[INIT]   ID: %-8d BIKE: %-3d SEM_IDs: %d %d\n",
    //     this->id,
    //     this->has_bike,
    //     params->sem_id_td_p,
    //     params->sem_id_td_c
    // );

    free(params);
    free(this);

    return 0;
}

void init_params(struct params *params) {
    const int sem_id_td_p = sem_alloc(SEM_T_DOOR_P_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_p == IPC_ERROR) exit_("Semaphore Allocation Error");

    const int sem_id_td_c = sem_alloc(SEM_T_DOOR_C_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_c == IPC_ERROR) exit_("Semaphore Allocation Error");

    params->sem_id_td_p = sem_id_td_p;
    params->sem_id_td_c = sem_id_td_c;
}

void init_passenger(struct passenger *this) {
    this->id = getpid();

    srand(getpid());
    if (get_random_number(0, PASSENGER_BIKE_PROB - 1))
        this->has_bike = 0;
    else
        this->has_bike = 1;
}

void board_train(const struct passenger *this, struct params *params) {
    sem_wait(params->sem_id_td_p, this->has_bike, 0);

    log_message(PROCESS_NAME,
                "[BOARDING]   ID: %-8d BIKE: %-3d\n",
                this->id,
                this->has_bike);

    sleep(3);
    sem_post(params->sem_id_td_c, this->has_bike);
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
