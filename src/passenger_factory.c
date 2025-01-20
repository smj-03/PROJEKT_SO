//
// Created by Szymon on 1/19/2025.
//
#include <utilities.h>
#include <config.h>

#define PROCESS_NAME "PASSENGER FACTORY"

void spawn_passenger();

void handle_sigchld(int sig);

int main(int argc, char *argv[]) {
    srand(time(NULL));

    int a = 3;
    while (a) {
        a--;
        int interval = get_random_number(PASSENGER_MIN_INTERVAL, PASSENGER_MAX_INTERVAL);
        int concurrent_passengers = get_random_number(1, PASSENGER_MAX_CONCURRENCY);

        log_message(PROCESS_NAME, "[SPAWN] %d processe(s)\n", concurrent_passengers);
        for (int i = 0; i < concurrent_passengers; i++) spawn_passenger(interval);

        sleep(interval);
    }

    return 0;
}

void spawn_passenger(int interval) {
    const int fork_val = fork();
    switch (fork_val) {
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
                "[SPAWN] PASSENGER: %d, Next in %d seconds\n",
                fork_val,
                interval);
    }
}
