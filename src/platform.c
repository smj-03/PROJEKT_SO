//
// Created by Szymon on 1/19/2025.
//
#include <utilities.h>
#include <config.h>

#define PROCESS_NAME "PLATFORM"

void spawn_passenger();

int main(int argc, char *argv[]) {
    srand(time(NULL));

    int a = 5;
    while (a) {
        // a--;
        const int interval = get_random_number(PASSENGER_MIN_INTERVAL, PASSENGER_MAX_INTERVAL);

        int concurrent_passengers = 1;
        int solo_passenger = get_random_number(0,PASSENGER_SOLO_PROB);
        if (!solo_passenger) concurrent_passengers = get_random_number(2, PASSENGER_MAX_CONCURRENCY);

        log_message(PROCESS_NAME, "[SPAWN] %d process(es)\n", concurrent_passengers);
        for (int i = 0; i < concurrent_passengers; i++) spawn_passenger(interval);

        sleep(interval);
    }

    return 0;
}

void spawn_passenger(int interval) {
    const int fork_val = fork();
    switch (fork_val) {
        case IPC_ERROR:
            throw_error(PROCESS_NAME, "Fork Error");

        case 0:
            const int exec_val = execl("./PASSENGER", "PASSENGER", NULL);
            if (exec_val == IPC_ERROR) throw_error(PROCESS_NAME, "%s Execl Error");

        default:
            log_message(
                PROCESS_NAME,
                "[SPAWN] PASSENGER: %d, Next in %d seconds\n",
                fork_val,
                interval);
    }
}
