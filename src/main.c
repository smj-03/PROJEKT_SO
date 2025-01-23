#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "MAIN"

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "MAIN PID: %d\n", getpid());
    char *processes[MAIN_PROCESS_NUM] = {"PLATFORM", "STATION_MASTER"};

    // TRAIN IPC INIT
    const int sem_id_td = sem_alloc(SEM_TRAIN_DOOR_KEY, SEM_TRAIN_DOOR_NUM, IPC_CREATE);
    if (sem_id_td == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");

    for (int i = 0; i < SEM_TRAIN_DOOR_NUM; i++)
        if (sem_init(sem_id_td, i, 1) == IPC_ERROR)
            throw_error(PROCESS_NAME, "Semaphore Control Error");

    const int shm_id_ts_1 = shared_block_alloc(SHM_TRAIN_STACK_1_KEY, sizeof(struct passenger_stack_1), IPC_CREATE);
    if (shm_id_ts_1 == IPC_ERROR) throw_error(PROCESS_NAME, "Shared Memory Allocation Error");

    const int shm_id_ts_2 = shared_block_alloc(SHM_TRAIN_STACK_2_KEY, sizeof(struct passenger_stack_2), IPC_CREATE);
    if (shm_id_ts_2 == IPC_ERROR) throw_error(PROCESS_NAME, "Shared Memory Allocation Error");

    if (shared_block_alloc(SHM_TRAIN_DOOR_1_KEY, (TRAIN_P_LIMIT + 2) * sizeof(int), IPC_CREATE) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Shared Memory Allocation Error");

    if (shared_block_alloc(SHM_TRAIN_DOOR_2_KEY, (TRAIN_B_LIMIT + 2) * sizeof(int), IPC_CREATE) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Shared Memory Allocation Error");

    const int msg_id_td_1 = message_queue_alloc(MSG_TRAIN_DOOR_1_KEY,IPC_CREATE);
    if (msg_id_td_1 == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");

    const int msg_id_td_2 = message_queue_alloc(MSG_TRAIN_DOOR_2_KEY,IPC_CREATE);
    if (msg_id_td_2 == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");

    struct message passenger_message;
    passenger_message.mtype = MSG_TYPE_EMPTY;
    for (int i = 0; i < TRAIN_P_LIMIT; i++)
        if (message_queue_send(msg_id_td_1, &passenger_message) == IPC_ERROR) throw_error(
            PROCESS_NAME, "Queue Initialization Error");
    for (int i = 0; i < TRAIN_B_LIMIT; i++)
        if (message_queue_send(msg_id_td_2, &passenger_message) == IPC_ERROR) throw_error(
            PROCESS_NAME, "Queue Initialization Error");

    // STATION MASTER IPC INIT
    const int sem_id_sm = sem_alloc(SEM_STATION_MASTER_KEY, 3, IPC_CREATE);
    if (sem_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");

    for (int i = 0; i < 3; i++)
        if (sem_init(sem_id_sm, i, 0) == IPC_ERROR)
            throw_error(PROCESS_NAME, "Semaphore Control Error");
    sem_post(sem_id_sm, 0);

    if (shared_block_alloc(SHM_STATION_MASTER_KEY, (TRAIN_NUM+ 2) * sizeof(int), IPC_CREATE) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Shared Memory Allocation Error");

    const int msg_id_sm = message_queue_alloc(MSG_STATION_MASTER_KEY,IPC_CREATE);
    if (msg_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");

    struct message train_message;
    train_message.mtype = MSG_TYPE_EMPTY;
    for (int i = 0; i < TRAIN_P_LIMIT; i++)
        if (message_queue_send(msg_id_sm, &train_message) == IPC_ERROR) throw_error(
            PROCESS_NAME, "Queue Initialization Error");
    

    for (int i = 0; i < MAIN_PROCESS_NUM; i++) {
        switch (fork()) {
            case IPC_ERROR:
                throw_error(PROCESS_NAME, "Fork Error");

            case 0:
                char path[20];
                get_process_path(path, processes[i]);
                const int exec_val = execl(path, processes[i], NULL);
                if (exec_val == IPC_ERROR)
                    throw_error(PROCESS_NAME, "%s Execl Error", processes[i]);

            default:
                if (VERBOSE_LOGS) log_message(PROCESS_NAME, "[SPAWN] %s process\n", processes[i]);
        }
    }

    for(int i = 0; i< TRAIN_NUM; i++) {
        const int fork_val = fork();
        switch (fork_val) {
            case IPC_ERROR:
                throw_error(PROCESS_NAME, "Fork Error");

            case 0:
                if(execl("./TRAIN", "TRAIN", NULL) == IPC_ERROR)
                    throw_error(PROCESS_NAME, "Execl Error");

            default:
                if(VERBOSE_LOGS) log_message(PROCESS_NAME, "[SPAWN] TRAIN process %d\n", fork_val);
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
