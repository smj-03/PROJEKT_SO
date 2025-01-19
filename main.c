#include <unistd.h>
#include <sys/wait.h>

#include "utilities.h"

#define PROCESS_NAME "MAIN"
#define PROCESS_NUMBER 1

int main(int argc, char *argv[]) {
    char *processes[PROCESS_NUMBER] = {"PASSENGER_FACTORY"};

    for (int i = 0; i < PROCESS_NUMBER; i++) {
        switch (fork()) {
            case -1:
                perror("fork");
                exit(1);
            case 0:
                log_message(PROCESS_NAME, "I'm the child\n");
                char path[20];
                get_process_path(path, processes[i]);
                const int execVal = execl(path, processes[i], NULL);
                if (execVal == -1) {
                    log_error(PROCESS_NAME, errno, "Execl Failure");
                    exit(1);
                }
            default:
                log_message(PROCESS_NAME, "I'm the parent\n");
        }
    }

    for (int i = 0; i < PROCESS_NUMBER; i++) {
        wait((int *) NULL);
    }
    return 0;
}
