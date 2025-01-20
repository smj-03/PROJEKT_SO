#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "MAIN"
#define PROCESS_NUMBER 2

void exit_(const char *);

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "MAIN PID: %d\n", getpid());
    char *processes[PROCESS_NUMBER] = {"PASSENGER_FACTORY", "TRAIN"};

    const int train_sem_id = sem_alloc(SEM_TRAIN_OPEN_KEY, SEM_TRAIN_OPEN_NUM, IPC_CREAT | IPC_EXCL | 0666);
    if (train_sem_id == -1) exit_("Semaphore Allocation Error");

    for(int i = 0; i < SEM_TRAIN_OPEN_NUM; i++) {
        const int init_res= sem_init(train_sem_id, i, 0);
        if (init_res == -1) exit_("Semaphore Control Error");
    }

    for (int i = 0; i < PROCESS_NUMBER; i++) {
        switch (fork()) {
            case -1:
                exit_("Fork Failure");

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

    sem_destroy(train_sem_id, SEM_TRAIN_OPEN_NUM);

    return 0;
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
