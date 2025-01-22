#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "MAIN"

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "MAIN PID: %d\n", getpid());
    char *processes[MAIN_PROCESS_NUM] = {"PLATFORM", "TRAIN", "STATION_MASTER"};

    const int sem_id_td= sem_alloc(SEM_TRAIN_DOOR_KEY, SEM_TRAIN_DOOR_NUM, IPC_CREATE);
    if (sem_id_td == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");

    for (int i = 0; i < SEM_TRAIN_DOOR_NUM; i++) {
        const int init_res_p = sem_init(sem_id_td, i, 1);
        if (init_res_p == -1) throw_error(PROCESS_NAME, "Semaphore Control Error");
    }

    const int shm_id_td_1 = shared_block_alloc(SHM_TRAIN_DOOR_1_KEY, (TRAIN_P_LIMIT + 2) * sizeof(int), IPC_CREATE);
    if (shm_id_td_1 == IPC_ERROR) throw_error(PROCESS_NAME,"Shared Memory Allocation Error");

    const int shm_id_td_2 = shared_block_alloc(SHM_TRAIN_DOOR_2_KEY, (TRAIN_B_LIMIT + 2) * sizeof(int), IPC_CREATE);
    if (shm_id_td_2 == IPC_ERROR) throw_error(PROCESS_NAME,"Shared Memory Allocation Error");

    const int msg_id_td_1 = message_queue_alloc(MSG_TRAIN_DOOR_1_KEY,IPC_CREATE);
    if (msg_id_td_1 == IPC_ERROR) throw_error(PROCESS_NAME,"Message Queue Allocation Error");

    const int msg_id_td_2 = message_queue_alloc(MSG_TRAIN_DOOR_2_KEY,IPC_CREATE);
    if (msg_id_td_2 == IPC_ERROR) throw_error(PROCESS_NAME,"Message Queue Allocation Error");

    struct message empty_message;
    empty_message.mtype = MSG_TYPE_EMPTY;
    for (int i = 0; i < TRAIN_P_LIMIT; i++)
        if (message_queue_send(msg_id_td_1, &empty_message) == IPC_ERROR) throw_error(PROCESS_NAME,"Queue Initialization Error");
    for (int i = 0; i < TRAIN_B_LIMIT; i++)
        if (message_queue_send(msg_id_td_2, &empty_message) == IPC_ERROR) throw_error(PROCESS_NAME,"Queue Initialization Error");

    for (int i = 0; i < MAIN_PROCESS_NUM; i++) {
        switch (fork()) {
            case IPC_ERROR:
                throw_error(PROCESS_NAME,"Fork Error");

            case 0:
                char path[20];
                get_process_path(path, processes[i]);
                const int exec_val = execl(path, processes[i], NULL);
                if (exec_val == IPC_ERROR)
                    throw_error(PROCESS_NAME, "%s Execl Error", processes[i]);


            default:
                if(VERBOSE_LOGS) log_message(PROCESS_NAME, "[SPAWN] %s process\n", processes[i]);
        }
    }

    for (int i = 0; i < MAIN_PROCESS_NUM; i++) {
        wait((int *) NULL);
    }

    sem_destroy(sem_id_td, SEM_TRAIN_DOOR_NUM);
    shared_block_destroy(SHM_TRAIN_DOOR_1_KEY);
    shared_block_destroy(SHM_TRAIN_DOOR_2_KEY);

    return 0;
}
