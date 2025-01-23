//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "TRAIN"

struct params {
    int sem_id_td;
    int msg_id_td_1;
    int msg_id_td_2;
    int *shared_memory_td_1;
    int *shared_memory_td_2;
    struct passenger_stack_1 *stack_1;
    struct passenger_stack_2 *stack_2;
    pthread_mutex_t *mutex;

    int sem_id_sm;
    int msg_id_sm;
    int *shared_memory_sm;
};

struct train {
    int id;
    int passenger_count;
    int bike_count;
    int return_interval;
};

struct thread_args {
    int door_number;
    struct train *this;
    struct params *params;
};

void init_params(struct params *);

void init_train(struct train *);

void init_conductor();

void *open_doors(void *);

// TODO: CHANGE FOR SIGNAL HANDLER
void wait_for_departure(struct train *, struct params *);

int main(int argc, char *argv[]) {
    log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    struct params *params = malloc(sizeof(struct params));
    if (params == NULL) throw_error(PROCESS_NAME, "Params Error");

    struct train *this = malloc(sizeof(struct train));
    if (this == NULL) throw_error(PROCESS_NAME, "Train Error");

    init_params(params);

    init_train(this);

    init_conductor();

    // 1 BAGGAGE 2 BIKE
    pthread_t id_thread_door_1, id_thread_door_2;

    struct thread_args *args_1 = malloc(sizeof(struct thread_args));
    if (args_1 == NULL) throw_error(PROCESS_NAME, "Thread Arguments Creation");
    args_1->door_number = 0;
    args_1->this = this;
    args_1->params = params;

    struct thread_args *args_2 = malloc(sizeof(struct thread_args));
    if (args_2 == NULL) throw_error(PROCESS_NAME, "Thread Arguments Creation");
    args_2->door_number = 1;
    args_2->this = this;
    args_2->params = params;

    if (VERBOSE_LOGS)
        log_message(
            PROCESS_NAME,
            "[INIT]   ID: %-8d SEM_ID: %d MSG_IDs: %d %d\n",
            this->id,
            params->sem_id_td,
            params->msg_id_td_1,
            params->msg_id_td_2
        );

    if (pthread_create(&id_thread_door_1, NULL, open_doors, args_1)) throw_error(PROCESS_NAME, "Thread 1 Creation");
    if (pthread_create(&id_thread_door_2, NULL, open_doors, args_2)) throw_error(PROCESS_NAME, "Thread 2 Creation");

    while(1) wait_for_departure(this, params);


    if (pthread_join(id_thread_door_1, NULL)) throw_error(PROCESS_NAME, "Thread 1 Join");
    if (pthread_join(id_thread_door_2, NULL)) throw_error(PROCESS_NAME, "Thread 2 Join");

    if (!pthread_detach(id_thread_door_1)) throw_error(PROCESS_NAME, "Thread 1 Detach");
    if (!pthread_detach(id_thread_door_2)) throw_error(PROCESS_NAME, "Thread 2 Detach");

    free(this);
    free(params);
    free(args_1);
    free(args_2);
    shared_block_destroy(SHM_TRAIN_DOOR_1_KEY);
    shared_block_destroy(SHM_TRAIN_DOOR_2_KEY);
}

void init_params(struct params *params) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    params->mutex = &mutex;

    const int sem_id_td = sem_alloc(SEM_TRAIN_DOOR_KEY, SEM_TRAIN_DOOR_NUM, IPC_GET);
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

    struct passenger_stack_1 *stack_1 = shared_block_attach(SHM_TRAIN_STACK_1_KEY, sizeof(struct passenger_stack_1));
    if (stack_1 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    stack_1->top = 0;
    params->stack_1 = stack_1;

    struct passenger_stack_2 *stack_2 = shared_block_attach(SHM_TRAIN_STACK_2_KEY, sizeof(struct passenger_stack_2));
    if (stack_2 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    stack_2->top = 0;
    params->stack_2 = stack_2;

    const int sem_id_sm = sem_alloc(SEM_STATION_MASTER_KEY, 3, IPC_GET);
    if (sem_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");
    params->sem_id_sm = sem_id_sm;

    int *shared_memory = shared_block_attach(SHM_STATION_MASTER_KEY, (TRAIN_NUM + 2) * sizeof(int));
    if (shared_memory == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->shared_memory_sm = shared_memory;

    const int msg_id_sm = message_queue_alloc(MSG_STATION_MASTER_KEY,IPC_GET);
    if (msg_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");
    params->msg_id_sm = msg_id_sm;
}

void init_train(struct train *this) {
    this->id = getpid();
    this->passenger_count = 0;
    this->bike_count = 0;

    srand(getpid());
    int return_interval = get_random_number(TRAIN_MIN_RETURN_INTERVAL, TRAIN_MAX_RETURN_INTERVAL);
    this->return_interval = return_interval;
}

void init_conductor() {
    const int fork_val = fork();
    switch (fork_val) {
        case IPC_ERROR:
            throw_error(PROCESS_NAME, "Execl Error");

        case 0:
            const int exec_val = execl("./CONDUCTOR", "CONDUCTOR", NULL);
            if (exec_val == IPC_ERROR) throw_error(PROCESS_NAME, "Execl Error");

        default:
            if(VERBOSE_LOGS) log_message(
                PROCESS_NAME,
                "[SPAWN] CONDUCTOR: %d\n",
                fork_val);
    }
}

void *open_doors(void *_args) {
    const struct thread_args *args = _args;
    const struct params *params = args->params;
    struct train *this = args->this;
    if(VERBOSE_LOGS) (PROCESS_NAME, "[THREAD] Doors %d\n", args->door_number + 1);

    while (1) {
        struct message message;
        const int msg_id = args->door_number ? params->msg_id_td_2 : params->msg_id_td_1;
        if (message_queue_receive(msg_id, &message, MSG_TYPE_FULL) == IPC_ERROR)
            throw_error(
                PROCESS_NAME, "Message Receive Error");

        int *shared_memory = args->door_number ? params->shared_memory_td_2 : params->shared_memory_td_1;
        const int limit = args->door_number ? TRAIN_B_LIMIT : TRAIN_P_LIMIT;

        sem_wait(params->sem_id_td, args->door_number, 0);
        const int read = shared_memory[limit];
        const int passenger_id = shared_memory[read];

        shared_memory[limit] = (shared_memory[limit] + 1) % limit;

        pthread_mutex_lock(params->mutex);
        this->passenger_count++;
        if (params->stack_1->top != TRAIN_P_LIMIT - 1)
            params->stack_1->data[params->stack_1->top++] = passenger_id;

        if (args->door_number == 1) {
            this->bike_count++;
            if (params->stack_2->top != TRAIN_P_LIMIT - 1)
                params->stack_2->data[params->stack_2->top++] = passenger_id;
        }
        pthread_mutex_unlock(params->mutex);

        sleep(PASSENGER_BOARDING_TIME);
        log_message(PROCESS_NAME,
                    "[DOOR %d] Welcome Passenger %d!\n",
                    args->door_number + 1,
                    passenger_id);
        sem_post(params->sem_id_td, args->door_number);

        message.mtype = MSG_TYPE_EMPTY;
        if (message_queue_send(msg_id, &message) == IPC_ERROR) throw_error(PROCESS_NAME, "Message Send Error");
    }
}

void wait_for_departure(struct train *this, struct params *params) {
    struct message message;
    if (message_queue_receive(params->msg_id_sm, &message, MSG_TYPE_EMPTY) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Receive Error");

    int *shared_memory = params->shared_memory_sm;

    const int save = shared_memory[TRAIN_NUM + 1];
    shared_memory[save] = getpid();
    shared_memory[TRAIN_NUM + 1] = (save + 1) % TRAIN_NUM;

    message.mtype = MSG_TYPE_FULL;
    if (message_queue_send(params->msg_id_sm, &message) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Send Error");

    sem_wait(params->sem_id_sm, 1, 0);

    // TODO: CALL/WAIT CONDUCTOR

    sem_post(params->sem_id_sm, 2);

    // TODO: CLEAN PASSENGERS, STACK ETC.

    params->stack_1->top = 0;
    params->stack_2->top = 0;
    params->shared_memory_td_1[TRAIN_P_LIMIT] = 0;
    params->shared_memory_td_1[TRAIN_P_LIMIT + 1] = 0;
    params->shared_memory_td_2[TRAIN_B_LIMIT] = 0;
    params->shared_memory_td_2[TRAIN_B_LIMIT + 1] = 0;

    log_message(PROCESS_NAME, "[INFO] Returns` in %d\n", this->return_interval);
    sleep(this->return_interval);
}
