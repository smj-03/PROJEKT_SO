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

    int a = 3;
    while (a) {
        a--;
        int interval = get_random_number(MIN_INTERVAL, MAX_INTERVAL);
        log_message(PROCESS_NAME, "Interval %d sec\n", interval);
        switch (fork()) {
            case -1:
                log_error(PROCESS_NAME, errno, "Fork Failure");
                exit(1);

            case 0:
                log_message(PROCESS_NAME, "Spawning PASSENGER process\n");
                const int execVal = execl("./PASSENGER", "PASSENGER", NULL);
                if (execVal == -1) {
                    log_error(PROCESS_NAME, errno, "%s Execl Failure");
                    exit(1);
                }
        }
        sleep(interval);
    }

    return 0;
}
