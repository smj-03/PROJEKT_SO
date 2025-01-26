#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "MAIN"

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "MAIN PID: %d\n", getpid());

    // TRAIN IPC INIT
    const int sem_id_ta = sem_alloc(SEM_TRAIN_ARRIVAL_KEY, 1, IPC_CREATE);
    if (sem_id_ta == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");
    if (sem_init(sem_id_ta, 0, 0) == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Control Error");

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

    if (shared_block_alloc(SHM_STATION_MASTER_TRAIN_KEY, (TRAIN_NUM+ 2) * sizeof(int), IPC_CREATE) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Shared Memory Allocation Error");

    if (shared_block_alloc(SHM_STATION_MASTER_PLATFORM_KEY, sizeof(int), IPC_CREATE) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Shared Memory Allocation Error");

    const int msg_id_sm = message_queue_alloc(MSG_STATION_MASTER_KEY,IPC_CREATE);
    if (msg_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");

    // PLATFORM IPC INIT
    const int sem_id_p = sem_alloc(SEM_PLATFORM_KEY, 1, IPC_CREATE);
    if (sem_id_p == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation");
    if (sem_init(sem_id_p, 0, 0) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Semaphore Control Error");

    struct message train_message;
    train_message.mtype = MSG_TYPE_EMPTY;
    for (int i = 0; i < TRAIN_P_LIMIT; i++)
        if (message_queue_send(msg_id_sm, &train_message) == IPC_ERROR) throw_error(
            PROCESS_NAME, "Queue Initialization Error");

    const int platform_id = fork();
    switch (platform_id) {
        case IPC_ERROR:
            throw_error(PROCESS_NAME,  " Platform Fork Error");

        case 0:
            if (execl("./PLATFORM", "PLATFORM", NULL) == IPC_ERROR)
                throw_error(PROCESS_NAME, "Platform Execl Error");

        default:
            if (VERBOSE_LOGS) log_message(PROCESS_NAME, "[SPAWN] PLATFORM %d\n", platform_id);
    }

    char platform_id_str[10];
    snprintf(platform_id_str, sizeof(platform_id_str), "%d", platform_id);
    const int station_master_id = fork();
    switch (station_master_id) {
        case IPC_ERROR:
            throw_error(PROCESS_NAME, "Station Master Fork Error");

        case 0:
            if (execl("./STATION_MASTER", "STATION_MASTER", platform_id_str, NULL) == IPC_ERROR)
                throw_error(PROCESS_NAME, "Station Master Execl Error");

        default:
            if (VERBOSE_LOGS) log_message(PROCESS_NAME, "[SPAWN] STATION MASTER %d\n", station_master_id);
    }

    for(int i = 0; i< TRAIN_NUM; i++) {
        const int train_id = fork();
        switch (train_id) {
            case IPC_ERROR:
                throw_error(PROCESS_NAME, "Fork Error");

            case 0:
                if(execl("./TRAIN", "TRAIN", NULL) == IPC_ERROR)
                    throw_error(PROCESS_NAME, "Execl Error");

            default:
                if(VERBOSE_LOGS) log_message(PROCESS_NAME, "[SPAWN] TRAIN process %d\n", train_id);
        }
    }

    for (int i = 0; i < TRAIN_NUM + 2; i++) {
        wait((int *) NULL);
    }

    sem_destroy(sem_id_td, SEM_TRAIN_DOOR_NUM);
    shared_block_destroy(SHM_TRAIN_DOOR_1_KEY);
    shared_block_destroy(SHM_TRAIN_DOOR_2_KEY);

    return 0;
}
