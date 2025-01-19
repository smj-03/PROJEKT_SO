#include "utilities.h"

#define PROCESS_NAME "MAIN"
#define PROCESS_NUMBER 2

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "MAIN PID: %d\n", getpid());
    char *processes[PROCESS_NUMBER] = {"PASSENGER_FACTORY", "TRAIN"};

    for (int i = 0; i < PROCESS_NUMBER; i++) {
        switch (fork()) {
            case -1:
                log_error(PROCESS_NAME, errno, "Fork Failure");
                exit(1);

            case 0:
                char path[20];
                get_process_path(path, processes[i]);
                const int execVal = execl(path, processes[i], NULL);
                if (execVal == -1) {
                    log_error(PROCESS_NAME, errno, "%s Execl Failure", processes[i]);
                    exit(1);
                }

            default:
                log_message(PROCESS_NAME, "Spawning %s process\n", processes[i]);
        }
    }

    for (int i = 0; i < PROCESS_NUMBER; i++) {
        wait((int *) NULL);
    }
    return 0;
}
