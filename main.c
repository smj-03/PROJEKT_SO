#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#include "utilities.h"

#define PROCESS_NUMBER 1

int main(void) {
    char* processes[PROCESS_NUMBER] = {"PASSENGER_FACTORY"};


    for (int i = 0; i < PROCESS_NUMBER; i++) {
        switch (fork()) {
            case -1:
                perror("fork");
                exit(1);
            case 0:
                printf("I am the child\n");
                char path[20];
                get_process_path(path, processes[i]);
                const int execVal = execl(path, processes[i], NULL);
                if (execVal == -1) {
                    perror("execvp");
                    exit(1);
                }
            default:
                printf("I am the parent\n");
        }
    }

    for (int i = 0; i < PROCESS_NUMBER; i++) {
        wait((int *) NULL);
    }
    return 0;
}
