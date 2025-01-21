//
// Created by Szymon on 1/21/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "CONDUCTOR"

int main(int argc, char *argv[]) {

    log_message(PROCESS_NAME, "[INIT] CONDUCTOR PID: %d\n", getpid());

    while(1);

    return 0;
}