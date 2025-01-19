#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    char* processes[] = {"PASSENGER_FACTORY", "PASSENGER"};


    for (int i = 0; i < 2; i++) {
        switch (fork()) {
            case -1:
                perror("fork");
                exit(1);
            case 0:
                printf("I am the child\n");
                char path[20];
                path[0] = '\0';
                strcat(path, "./");
                strcat(path, processes[i]);
                const int execVal = execl(path, processes[i], NULL);
                if (execVal == -1) {
                    perror("execvp");
                    exit(1);
                }
            default:
                printf("I am the parent\n");
        }
    }

    for (int i = 0; i < 2; i++) {
        wait((int *) NULL);
    }
    return 0;
}
