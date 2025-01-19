//
// Created by Szymon on 1/19/2025.
//

#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "utilities.h"

#define MIN_INTERVAL 3
#define MAX_INTERVAL 10

int main(int argc, char *argv[]) {
    srand(time(NULL));

    time_t now = time(NULL);

    while (1) {
        int interval = get_random_number(MIN_INTERVAL, MAX_INTERVAL);
        printf("Interval %d sec\n", interval);
        sleep(interval);
    }

    return 0;
}
