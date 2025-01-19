//
// Created by Szymon on 1/19/2025.
//

#include <time.h>
#include <unistd.h>

#include "utilities.h"

#define PROCESS_NAME "PASSENGER FACTORY"
#define MIN_INTERVAL 3
#define MAX_INTERVAL 10

int main(int argc, char *argv[]) {
    srand(time(NULL));

    while (1) {
        int interval = get_random_number(MIN_INTERVAL, MAX_INTERVAL);
        log_message(PROCESS_NAME, "Interval %d sec\n", interval);
        if (interval >= 8) {
            log_error(PROCESS_NAME, errno, "Interval too big! %d\n", interval);
            break;
        }
        sleep(interval);
    }

    return 0;
}
