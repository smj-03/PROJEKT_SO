//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "PASSENGER"

struct passenger {
    int id;
    _Bool has_bike;
} *this;

struct params {
    int sem_id_pc;
    int sem_id_sm;
    int sem_id_td;
    int msg_id_td_1;
    int msg_id_td_2;
    int *shared_memory_td_1;
    int *shared_memory_td_2;
    int *shared_memory_counter;
} *params;

void handle_sigusr1(int);

void handle_sigterm(int);

void init_params();

void init_passenger();

void board_train();

void increase_counter();

void decrease_counter();

int main() {
    setup_signal_handler(SIGUSR1, handle_sigusr1);
    setup_signal_handler(SIGTERM, handle_sigterm);

    params = malloc(sizeof(struct params));
    if (params == NULL) throw_error(PROCESS_NAME, "Params Error");

    this = malloc(sizeof(struct passenger));
    if (this == NULL) throw_error(PROCESS_NAME, "Passenger Error");

    init_params();

    init_passenger();

    increase_counter();

    log_message(PROCESS_NAME, "[ARRIVAL] ID: %d BIKE: %d\n", this->id, this->has_bike);

    board_train();

    while (1) {
        pause();
        log_message(PROCESS_NAME, "[EXIT] ID: %d\n", getpid());
        board_train(this, params);
    }

    // TODO: SIGTERM EXIT + CLEANUP

    shared_block_detach(params->shared_memory_td_1);
    shared_block_detach(params->shared_memory_td_2);
    free(params);
    free(this);

    return 0;
}

void handle_sigusr1(int sig) {
    // write(STDOUT_FILENO, "Received SIGUSR1, continuing...\n", 32);
}

void handle_sigterm(int) {
    decrease_counter();
    shared_block_detach(params->shared_memory_td_1);
    shared_block_detach(params->shared_memory_td_2);
    shared_block_detach(params->shared_memory_counter);
    free(params);
    free(this);
    _exit(0);
}

void init_params() {
    const int sem_id_sm = sem_alloc(SEM_STATION_MASTER_KEY, 1, IPC_GET);
    if (sem_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");
    params->sem_id_sm = sem_id_sm;

    const int sem_id_td = sem_alloc(SEM_TRAIN_DOOR_KEY, SEM_TRAIN_DOOR_NUM * 2, IPC_GET);
    if (sem_id_td == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");
    params->sem_id_td = sem_id_td;

    int *shared_memory_1 = shared_block_attach(SHM_TRAIN_DOOR_1_KEY, (TRAIN_P_LIMIT + 2) * sizeof(int));
    if (shared_memory_1 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->shared_memory_td_1 = shared_memory_1;

    int *shared_memory_2 = shared_block_attach(SHM_TRAIN_DOOR_2_KEY, (TRAIN_B_LIMIT + 2) * sizeof(int));
    if (shared_memory_2 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->shared_memory_td_2 = shared_memory_2;

    const int msg_id_td_1 = message_queue_alloc(MSG_TRAIN_DOOR_1_KEY,IPC_GET);
    if (msg_id_td_1 == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");
    params->msg_id_td_1 = msg_id_td_1;

    const int msg_id_td_2 = message_queue_alloc(MSG_TRAIN_DOOR_2_KEY,IPC_GET);
    if (msg_id_td_2 == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");
    params->msg_id_td_2 = msg_id_td_2;

    int *shared_memory_counter = shared_block_attach(SHM_STATION_MASTER_PASSENGER_KEY, sizeof(int));
    if (shared_memory_counter == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->shared_memory_counter = shared_memory_counter;

    const int sem_id_pc = sem_alloc(SEM_PASSENGER_KEY, 1, IPC_GET);
    if (sem_id_pc == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation");
    params->sem_id_pc = sem_id_pc;
}

void init_passenger() {
    this->id = getpid();

    srand(getpid());
    if (get_random_number(0, PASSENGER_BIKE_PROB - 1))
        this->has_bike = 0;
    else
        this->has_bike = 1;
}

void board_train() {
    sem_wait_no_op(params->sem_id_sm, 0, 0);

    struct message message;
    const int msg_id = this->has_bike ? params->msg_id_td_2 : params->msg_id_td_1;
    if (message_queue_receive(msg_id, &message, MSG_TYPE_EMPTY, 0) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Receive Error");

    log_message(PROCESS_NAME, "Message received %d\n", this->id);

    int *shared_memory = this->has_bike ? params->shared_memory_td_2 : params->shared_memory_td_1;
    const int limit = this->has_bike ? TRAIN_B_LIMIT : TRAIN_P_LIMIT;
    const int save = shared_memory[limit + 1];

    // TODO: CRITICAL SECTION ADD SEMAPHORE
    sem_wait(params->sem_id_td, this->has_bike, 0);
    shared_memory[save] = this->id;
    shared_memory[limit + 1] = (save + 1) % limit;
    sem_post(params->sem_id_td, this->has_bike);

    message.mtype = MSG_TYPE_FULL;
    if (message_queue_send(msg_id, &message) == IPC_ERROR) throw_error(PROCESS_NAME, "Message Send Error");
}

void increase_counter() {
    sem_wait(params->sem_id_pc, 0, 0);
    params->shared_memory_counter[0]++;
    sem_post(params->sem_id_pc, 0);
}

void decrease_counter() {
    sem_wait(params->sem_id_pc, 0, 0);
    params->shared_memory_counter[0]--;
    sem_post(params->sem_id_pc, 0);
}
