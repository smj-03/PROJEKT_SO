#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "MAIN"

void exit_(const char *);

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "MAIN PID: %d\n", getpid());
    char *processes[MAIN_PROCESS_NUM] = {"PASSENGER_FACTORY", "TRAIN", "STATION_MASTER"};

    const int sem_id_td_p = sem_alloc(SEM_T_DOOR_P_KEY, SEM_T_DOOR_NUM, IPC_CREATE);
    if (sem_id_td_p == IPC_ERROR) exit_("Semaphore Allocation Error");

    const int sem_id_td_c = sem_alloc(SEM_T_DOOR_C_KEY, SEM_T_DOOR_NUM, IPC_CREATE);
    if (sem_id_td_c == IPC_ERROR) exit_("Semaphore Allocation Error");

    for (int i = 0; i < SEM_T_DOOR_NUM; i++) {
        const int init_res_p = sem_init(sem_id_td_p, i, 0);
        if (init_res_p == -1) exit_("Semaphore Control Error");

        const int init_res_c = sem_init(sem_id_td_c, i, 0);
        if (init_res_c == -1) exit_("Semaphore Control Error");
    }

    const int shm_id_td_1 = shared_block_alloc(SHM_TRAIN_DOOR_1_KEY, (TRAIN_P_LIMIT + 2) * sizeof(int), IPC_CREATE);
    if (shm_id_td_1 == IPC_ERROR) exit_("Shared Memory Allocation Error");

    const int shm_id_td_2 = shared_block_alloc(SHM_TRAIN_DOOR_2_KEY, (TRAIN_B_LIMIT + 2) * sizeof(int), IPC_CREATE);
    if (shm_id_td_2 == IPC_ERROR) exit_("Shared Memory Allocation Error");

    const int msg_id_td_1 = message_queue_alloc(MSG_TRAIN_DOOR_1_KEY,IPC_CREATE);
    if (msg_id_td_1 == IPC_ERROR) exit_("Message Queue Allocation Error");

    const int msg_id_td_2 = message_queue_alloc(MSG_TRAIN_DOOR_2_KEY,IPC_CREATE);
    if (msg_id_td_2 == IPC_ERROR) exit_("Message Queue Allocation Error");

    struct message empty_message;
    empty_message.mtype = MSG_TYPE_EMPTY;
    for (int i = 0; i < TRAIN_P_LIMIT; i++)
        if (message_queue_send(msg_id_td_1, &empty_message) == IPC_ERROR) exit_("Queue Initialization Error");
    for (int i = 0; i < TRAIN_B_LIMIT; i++)
        if (message_queue_send(msg_id_td_2, &empty_message) == IPC_ERROR) exit_("Queue Initialization Error");

    for (int i = 0; i < MAIN_PROCESS_NUM; i++) {
        switch (fork()) {
            case IPC_ERROR:
                exit_("Fork Failure");

            case 0:
                char path[20];
                get_process_path(path, processes[i]);
                const int exec_val = execl(path, processes[i], NULL);
                if (exec_val == IPC_ERROR) {
                    log_error(PROCESS_NAME, errno, "%s Execl Failure", processes[i]);
                    exit(1);
                }

            default:
                log_message(PROCESS_NAME, "[SPAWN] %s process\n", processes[i]);
        }
    }

    for (int i = 0; i < MAIN_PROCESS_NUM; i++) {
        wait((int *) NULL);
    }

    sem_destroy(sem_id_td_p, SEM_T_DOOR_NUM);
    sem_destroy(sem_id_td_c, SEM_T_DOOR_NUM);
    shared_block_destroy(SHM_TRAIN_DOOR_1_KEY);
    shared_block_destroy(SHM_TRAIN_DOOR_2_KEY);

    return 0;
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
