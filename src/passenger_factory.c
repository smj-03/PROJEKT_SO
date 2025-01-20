//
// Created by Szymon on 1/19/2025.
//
#include <utilities.h>

#define PROCESS_NAME "PASSENGER FACTORY"
#define MIN_INTERVAL 0
#define MAX_INTERVAL 15

void handle_sigchld(int);

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        log_error(PROCESS_NAME, errno, "Sigaction Error");
        exit(1);
    }

    while (1) {
        int interval = get_random_number(MIN_INTERVAL, MAX_INTERVAL);
        if (interval <= 5) interval = 0;

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
                    "[SPAWN] PASSENGER: %d, Next in %d seconds\n",
                    forkVal,
                    interval);
                // int status;
                // waitpid(forkVal, &status, 0);
        }

        sleep(interval);
    }

    return 0;
}

void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}