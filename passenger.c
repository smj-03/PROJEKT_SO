//
// Created by Szymon on 1/19/2025.
//

#include <unistd.h>

#include "utilities.h"

#define PROCESS_NAME "PASSENGER"

int main() {
    sleep(1);
    log_message(PROCESS_NAME, "Hello, I'm a passenger %d!\n", getpid());
    return 0;
}
