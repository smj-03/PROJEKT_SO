//
// Created by Szymon on 1/19/2025.
//
#include <utilities.h>

#define PROCESS_NAME "PASSENGER FACTORY"
#define MIN_INTERVAL 5
#define MAX_INTERVAL 10

int main(int argc, char *argv[]) {
    srand(time(NULL));

    int a = 3;
    while (a) {
        a--;
        int interval = get_random_number(MIN_INTERVAL, MAX_INTERVAL);
        // if (interval <= 5) interval = 0;

        const int forkVal = fork();
        switch (forkVal) {
            case -1:
                log_error(PROCESS_NAME, errno, "Fork Failure");
                exit(1);

            case 0:
                const int execVal = execl("./PASSENGER", "PASSENGER", NULL);
                if (execVal == -1) {
                    log_error(PROCESS_NAME, errno, "%s Execl Failure");
                    exit(1);
                }

            default:
                log_message(
                    PROCESS_NAME,
                    "Spawning PASSENGER: %d, Next in %d seconds\n",
                    forkVal,
                    interval);
        }

        sleep(interval);
    }

    return 0;
}
