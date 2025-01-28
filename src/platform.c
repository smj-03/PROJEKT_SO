//
// Created by Szymon on 1/19/2025.
//
#include <utilities.h>
#include <config.h>

#define PROCESS_NAME "PLATFORM"

volatile int platform_open = 1;

void handle_sigusr1(int sig);

void handle_sigusr2(int);

void reap_passengers();

void *clear_passengers(void *);

void spawn_passenger();

int main(int argc, char *argv[]) {
    if(VERBOSE_LOGS) log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    // Ustawienie sygnału do zatrzymania tworzenia pasażerów.
    setup_signal_handler(SIGUSR2, handle_sigusr2);

    srand(time(NULL));

    // Wątek czyszczący pasażerów zombie.
    pthread_t clean_thread_id;
    if (pthread_create(&clean_thread_id, NULL, clear_passengers, NULL))
        throw_error(PROCESS_NAME, "Thread Creation");

    while (platform_open) {
        // Tworzenie pasażerów w losowych odstępach czasu oraz losowej ilości.
        const int interval = get_random_number(PASSENGER_MIN_INTERVAL, PASSENGER_MAX_INTERVAL);

        int concurrent_passengers = 1;
        const int solo_passenger = get_random_number(0,PASSENGER_SOLO_PROB);
        if (!solo_passenger) concurrent_passengers = get_random_number(2, PASSENGER_MAX_CONCURRENCY);

        if(VERBOSE_LOGS) log_message(PROCESS_NAME, "[SPAWN] %d PASSENGER(S)\n", concurrent_passengers);
        for (int i = 0; i < concurrent_passengers; i++) spawn_passenger(interval);

        sleep(interval);
    }

    const int sem_id = sem_alloc(SEM_PLATFORM_KEY, 1, IPC_GET);
    if (sem_id == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation");

    // Oczekiwanie na semafor o zakończeniu pracy (STATION MASTER 65).
    if (sem_wait(sem_id, 0, 0) == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Wait Error");

    if (pthread_join(clean_thread_id, NULL)) throw_error(PROCESS_NAME, "Thread Join");

    reap_passengers();
    return 0;
}

void handle_sigusr2(int sig) {
    // write(STDOUT_FILENO, "Received SIGUSR2, continuing...\n", 32);
    platform_open = 0;
}

void reap_passengers() {
    int status;
    while (waitpid(-1, &status, 0) > 0);
}

void *clear_passengers(void *args) {
    while (platform_open) reap_passengers();
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
