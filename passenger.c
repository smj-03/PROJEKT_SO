//
// Created by Szymon on 1/19/2025.
//

#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    sleep(1);
    printf("Hello Passenger!\n");
    fflush(stdout);
    return 0;
}
