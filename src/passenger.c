//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "PASSENGER"

struct passenger {
    int id;
    _Bool has_bike;
};

struct params {
    int sem_id_td_p;
    int sem_id_td_c;
    int msg_id_td_1;
    int msg_id_td_2;
    int *shared_memory_1;
    int *shared_memory_2;
};

void init_params(struct params *);

void init_passenger(struct passenger *);

void board_train(const struct passenger *, struct params *);

void exit_(const char *);

int main() {
    struct params *params = malloc(sizeof(struct params));
    if (params == NULL) {
        log_error(PROCESS_NAME, errno, "Params Error");
        exit(1);
    }

    struct passenger *this = malloc(sizeof(struct passenger));
    if (this == NULL) {
        log_error(PROCESS_NAME, errno, "Passenger Error");
        exit(1);
    }

    init_params(params);
    init_passenger(this);
    board_train(this, params);

    // log_message(
    //     PROCESS_NAME,
    //     "[INIT]   ID: %-8d BIKE: %-3d SEM_IDs: %d %d MSG_IDs: %d %d\n",
    //     this->id,
    //     this->has_bike,
    //     params->sem_id_td_p,
    //     params->sem_id_td_c,
    //     params->msg_id_td_1,
    //     params->msg_id_td_2
    // );

    shared_block_detach(params->shared_memory_1);
    shared_block_detach(params->shared_memory_2);
    free(params);
    free(this);

    return 0;
}

void init_params(struct params *params) {
    const int sem_id_td_p = sem_alloc(SEM_T_DOOR_P_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_p == IPC_ERROR) exit_("Semaphore Allocation Error");
    params->sem_id_td_p = sem_id_td_p;

    const int sem_id_td_c = sem_alloc(SEM_T_DOOR_C_KEY, SEM_T_DOOR_NUM, IPC_GET);
    if (sem_id_td_c == IPC_ERROR) exit_("Semaphore Allocation Error");
    params->sem_id_td_c = sem_id_td_c;

    int *shared_memory_1 = shared_block_attach(SHM_TRAIN_DOOR_1_KEY, (TRAIN_P_LIMIT + 2) * sizeof(int));
    if (shared_memory_1 == NULL) exit_("Shared Memory Attach Error");
    params->shared_memory_1 = shared_memory_1;

    int *shared_memory_2 = shared_block_attach(SHM_TRAIN_DOOR_2_KEY, (TRAIN_B_LIMIT + 2) * sizeof(int));
    if (shared_memory_2 == NULL) exit_("Shared Memory Attach Error");
    params->shared_memory_2 = shared_memory_2;

    const int msg_id_td_1 = message_queue_alloc(MSG_TRAIN_DOOR_1_KEY,IPC_GET);
    if (msg_id_td_1 == IPC_ERROR) exit_("Message Queue Allocation Error");
    params->msg_id_td_1 = msg_id_td_1;

    const int msg_id_td_2 = message_queue_alloc(MSG_TRAIN_DOOR_2_KEY,IPC_GET);
    if (msg_id_td_2 == IPC_ERROR) exit_("Message Queue Allocation Error");
    params->msg_id_td_2 = msg_id_td_2;
}

void init_passenger(struct passenger *this) {
    this->id = getpid();

    srand(getpid());
    if (get_random_number(0, PASSENGER_BIKE_PROB - 1))
        this->has_bike = 0;
    else
        this->has_bike = 1;
}

void board_train(const struct passenger *this, struct params *params) {
    sem_wait(params->sem_id_td_p, this->has_bike, 0);

    struct message message;
    const int msg_id = this->has_bike ? params->msg_id_td_2 : params->msg_id_td_1;
    if(message_queue_receive(msg_id, &message, MSG_TYPE_EMPTY) == IPC_ERROR) exit_("Message Receive Error");

    int *shared_memory = this->has_bike ? params->shared_memory_2 : params->shared_memory_1;
    const int limit = this->has_bike ? TRAIN_B_LIMIT : TRAIN_P_LIMIT;

    const int save = shared_memory[limit + 1];
    shared_memory[save] = this->id;
    shared_memory[limit + 1] = (save + 1) % limit;

    log_message(PROCESS_NAME,
                "[BOARDING]   ID: %-8d BIKE: %-3d\n",
                this->id,
                this->has_bike);

    sleep(3);

    message.mtype = MSG_TYPE_FULL;
    if(message_queue_send(msg_id, &message) == IPC_ERROR) exit_("Message Send Error");
    sem_post(params->sem_id_td_c, this->has_bike);
}

void exit_(const char *message) {
    log_error(PROCESS_NAME, errno, message);
    exit(1);
}
