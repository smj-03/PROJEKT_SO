//
// Created by Szymon on 1/19/2025.
//
#include "utilities.h"

#define PROCESS_NAME "PASSENGER"
#define BIKE_PROB 4 // 25%

struct passenger {
    int id;
    _Bool has_bike;
    time_t *ts_onboarding;
};

void init_passenger(struct passenger *this) {
    this->id = getpid();
    this->ts_onboarding = NULL;

    if (get_random_number(0, BIKE_PROB - 1))
        this->has_bike = 0;
    else
        this->has_bike = 1;
}

int main() {
    srand(time(NULL));

    sleep(1);
    struct passenger *this = malloc(sizeof(struct passenger));
    init_passenger(this);

    if (this == NULL) {
        log_error(PROCESS_NAME, errno, "Passenger Failure");
        exit(1);
    }

    log_message(
        PROCESS_NAME,
        "[NEW PASSENGER]   ID: %-8d BIKE: %d\n",
        this->id,
        this->has_bike);

    free(this);

    return 0;
}
