//
// Created by Szymon on 1/19/2025.
//
#include <utilities.h>
#include <config.h>

#define PROCESS_NAME "PLATFORM"

volatile int platform_open = 1;

void handle_sigusr2(int);

void *clean_passengers(void *);

void spawn_passenger();

int main(int argc, char *argv[]) {
    setup_signal_handler(SIGUSR2, handle_sigusr2);

    srand(time(NULL));

    while (platform_open) {
        const int interval = get_random_number(PASSENGER_MIN_INTERVAL, PASSENGER_MAX_INTERVAL);

        int concurrent_passengers = 1;
        int solo_passenger = get_random_number(0,PASSENGER_SOLO_PROB);
        if (!solo_passenger) concurrent_passengers = get_random_number(2, PASSENGER_MAX_CONCURRENCY);

        log_message(PROCESS_NAME, "[SPAWN] %d PASSENGER(S)\n", concurrent_passengers);
        for (int i = 0; i < concurrent_passengers; i++) spawn_passenger(interval);

        sleep(interval);
    }

    log_message(PROCESS_NAME, "Platform closed!\n");
    const int sem_id = sem_alloc(SEM_PLATFORM_KEY, 1, IPC_GET);
    if(sem_id == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation");
    if (sem_wait(sem_id, 0, 0) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Semaphore Wait Error");
    log_message(PROCESS_NAME, "Signal received!\n");

    pthread_t clean_thread_id;
    if (pthread_create(&clean_thread_id, NULL, clean_passengers, NULL))
        throw_error(PROCESS_NAME, "Thread Creation");

    if (pthread_join(clean_thread_id, NULL)) throw_error(PROCESS_NAME, "Thread Join");

    sem_destroy(sem_id, 1);
    return 0;
}

void handle_sigusr2(int sig) {
    write(STDOUT_FILENO, "Received SIGUSR2, continuing...\n", 32);
    platform_open = 0;
}

void *clean_passengers(void *args) {
    int status;
    while (waitpid(-1, &status, 0) > 0)
        if (VERBOSE_LOGS)
            log_message(PROCESS_NAME, "Reaped child process with status: %d\n", status);
    log_message(PROCESS_NAME, "Finished reaping passengers!\n");
    return NULL;
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
            if (VERBOSE_LOGS)
                log_message(
                    PROCESS_NAME,
                    "[SPAWN] PASSENGER: %d, Next in %d seconds\n",
                    fork_val,
                    interval);
    }
}
