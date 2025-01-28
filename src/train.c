//
// Created by Szymon on 1/19/2025.
//
#include <config.h>
#include <utilities.h>

#define PROCESS_NAME "TRAIN"

struct params {
    int sem_id_td;
    int sem_id_c;
    int msg_id_td_1;
    int msg_id_td_2;
    int *shared_memory_td_1;
    int *shared_memory_td_2;
    struct passenger_stack *stack_1;
    struct passenger_stack *stack_2;
    pthread_mutex_t *mutex;

    int sem_id_sm;
    int msg_id_sm;
    int *shared_memory_sm;
} *params;

struct train {
    int id;
    int passenger_count;
    int bike_count;
    int return_interval;
} *this;

struct thread_args {
    int door_number;
    struct train *this;
    struct params *params;
};

volatile int has_arrived;

volatile int received_depart_signal;

volatile int platform_open = 1;

void handle_sigcont(int);

void init_params();

void init_train();

int init_conductor();

void *open_doors(void *);

void arrive_and_depart();

int main(int argc, char *argv[]) {
    if (VERBOSE_LOGS) log_message(PROCESS_NAME, "[INIT] ID: %d\n", getpid());

    if (setup_signal_handler(SIGCONT, handle_sigcont) == IPC_ERROR)
        throw_error(PROCESS_NAME, "SIGCONT Handler Error");

    params = malloc(sizeof(struct params));
    if (params == NULL) throw_error(PROCESS_NAME, "Params Error");

    this = malloc(sizeof(struct train));
    if (this == NULL) throw_error(PROCESS_NAME, "Train Error");

    init_params();

    init_train(this);

    while (1) arrive_and_depart(this, params);

    free(this);
    free(params);
    shared_block_destroy(SHM_TRAIN_DOOR_1_KEY);
    shared_block_destroy(SHM_TRAIN_DOOR_2_KEY);
}

void handle_sigcont(int sig) {
    // write(STDOUT_FILENO, "Received SIGCONT, continuing...\n", 32);
    has_arrived = 1;
}

void init_train() {
    this->id = getpid();
    this->passenger_count = 0;
    this->bike_count = 0;

    srand(getpid());
    const int return_interval = get_random_number(TRAIN_MIN_RETURN_INTERVAL, TRAIN_MAX_RETURN_INTERVAL);
    this->return_interval = return_interval;
}

int init_conductor() {
    const int fork_val = fork();
    switch (fork_val) {
        case IPC_ERROR:
            throw_error(PROCESS_NAME, "Execl Error");

        case 0:
            const int exec_val = execl("./CONDUCTOR", "CONDUCTOR", NULL);
            if (exec_val == IPC_ERROR) throw_error(PROCESS_NAME, "Execl Error");
            return getpid();

        default:
            if (VERBOSE_LOGS)
                log_message(
                    PROCESS_NAME,
                    "[SPAWN] CONDUCTOR: %d\n",
                    fork_val);
            return fork_val;
    }
}

/**
 * Funkcja wątku otwierającego drzwi pociągu.
 * @param _args
 * @return
 */
void *open_doors(void *_args) {
    const struct thread_args *args = _args;
    const struct params *params = args->params;
    struct train *this = args->this;

    int handled_depart_signal = 0;

    // Drzwi są otwarte o ile pociąg przyjechał, nie otrzymał sygnały o odjeździe i nie jest pełny.
    while (has_arrived && !received_depart_signal && this->passenger_count < TRAIN_MAX_CAPACITY) {
        struct message message;
        const int msg_id = args->door_number ? params->msg_id_td_2 : params->msg_id_td_1;

        // Pobieranie pasażera z kolejki komunikatów.
        if (message_queue_receive(msg_id, &message, MSG_TYPE_FULL, 0) == IPC_ERROR)
            throw_error(PROCESS_NAME, "Message Receive Error");

        // Obsułga sygnału o odjeździe jeśli drzwi były na etapie message_queue_receive.
        if (received_depart_signal) {
            handled_depart_signal = 1;
            break;
        }

        int *shared_memory = args->door_number ? params->shared_memory_td_2 : params->shared_memory_td_1;
        const int limit = args->door_number ? TRAIN_B_LIMIT : TRAIN_P_LIMIT;

        // Sekcja krtytyczna
        sem_wait(params->sem_id_td, args->door_number, 0);

        // Czytanie ID pasażera z pamięci współdzielonej.
        const int read = shared_memory[limit];
        const int passenger_id = shared_memory[read];
        shared_memory[limit] = (shared_memory[limit] + 1) % limit;

        // Oczekiwanie na wsiadanie pasażera.
        sleep(PASSENGER_BOARDING_TIME);

        pthread_mutex_lock(params->mutex);

        this->passenger_count++;
        if (args->door_number == 0) {
            if (push(params->stack_1, passenger_id) == -1) throw_error(PROCESS_NAME, "Stack Push Error");
        } else {
            this->bike_count++;
            if (push(params->stack_2, passenger_id) == -1) throw_error(PROCESS_NAME, "Stack Push Error");
        }
        pthread_mutex_unlock(params->mutex);
        sem_post(params->sem_id_td, args->door_number);

        log_message(PROCESS_NAME,
                    "[%d][DOOR %d] Welcome Passenger %d!\n",
                    this->id,
                    args->door_number + 1,
                    passenger_id);

        message.mtype = MSG_TYPE_EMPTY;
        if (message_queue_send(msg_id, &message) == IPC_ERROR) throw_error(PROCESS_NAME, "Message Send Error");
    }

    if (this->passenger_count == TRAIN_MAX_CAPACITY)
        log_warning(PROCESS_NAME, "[%d][INFO] Train is full! Doors %d have closed!\n", this->id, args->door_number + 1);

    // Obsługa sygnału o odjeździe jeśli drzwi nie były na etapie message_queue_receive.
    if (!handled_depart_signal && this->passenger_count < TRAIN_MAX_CAPACITY) {
        struct message poison_message;
        const int msg_id = args->door_number ? params->msg_id_td_2 : params->msg_id_td_1;
        if (message_queue_receive(msg_id, &poison_message, MSG_TYPE_FULL, 0) == IPC_ERROR)
            throw_error(PROCESS_NAME, "Message Receive Error");
        handled_depart_signal = 1;
    }
}

void arrive_and_depart() {
    has_arrived = 0;
    received_depart_signal = 0;

    struct message message;
    if (message_queue_receive(params->msg_id_sm, &message, MSG_TYPE_EMPTY, 0) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Receive Error");

    int *shared_memory = params->shared_memory_sm;

    // Zapisywanie ID pociągu do kolejki pociągów.
    const int save = shared_memory[TRAIN_NUM + 1];
    shared_memory[save] = getpid();
    shared_memory[TRAIN_NUM + 1] = (save + 1) % TRAIN_NUM;

    message.mtype = MSG_TYPE_FULL;
    if (message_queue_send(params->msg_id_sm, &message) == IPC_ERROR)
        throw_error(PROCESS_NAME, "Message Send Error");

    // Czekanie na sygnał do przyjazdu.
    pause();

    // Inicjalizowanie konduktora.
    const int conductor_pid = init_conductor();

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

    // Tworzenie wątków drzwi.
    if (pthread_create(&id_thread_door_1, NULL, open_doors, args_1)) throw_error(PROCESS_NAME, "Thread 1 Creation");
    if (pthread_create(&id_thread_door_2, NULL, open_doors, args_2)) throw_error(PROCESS_NAME, "Thread 2 Creation");

    // Oczekiwanie na sygnał odjazdu.
    if (wait_for_signal(SIGUSR1) == IPC_ERROR) throw_error(PROCESS_NAME, "Signal Wait Error");
    received_depart_signal = 1;

    // Wysyłanie sygnału do sekcji message_queue_receive żeby przerwać oczekiwanie.
    struct message poison_message;
    poison_message.mtype = MSG_TYPE_FULL;
    message_queue_send(params->msg_id_td_1, &poison_message);
    message_queue_send(params->msg_id_td_2, &poison_message);

    has_arrived = 0;

    if (pthread_join(id_thread_door_1, NULL)) throw_error(PROCESS_NAME, "Thread 1 Join");
    if (pthread_join(id_thread_door_2, NULL)) throw_error(PROCESS_NAME, "Thread 2 Join");

    // Podnoszenie semaforu dla kondutkora.
    sem_post(params->sem_id_c, 0);
    sem_wait(params->sem_id_c, 1, 0);

    // Kończenie procesu konduktora.
    if (kill(conductor_pid, SIGKILL) == IPC_ERROR) throw_error(PROCESS_NAME, "Sigkill Error");
    if (waitpid(conductor_pid, NULL, 0) == -1) throw_error(PROCESS_NAME, "Waitpid Error");

    // Kończenie procesów pasażerów którzy odjeżdzają.
    for (int i = 0; i < params->stack_1->top; i++) {
        kill(params->stack_1->data[i], SIGTERM);
        log_message(PROCESS_NAME, "[INFO] Passenger %d is on the way!\n", params->stack_1->data[i]);
    }
    for (int i = 0; i < params->stack_2->top; i++) {
        kill(params->stack_2->data[i], SIGTERM);
        log_message(PROCESS_NAME, "[INFO] Passenger %d is on the way!\n", params->stack_2->data[i]);
    }

    log_message(PROCESS_NAME,
                "[%d][INFO] Passengers taken: %d, Passengers with bikes taken: %d, Returns in %d seconds\n",
                this->id,
                params->stack_1->top,
                params->stack_2->top,
                this->return_interval);


    this->passenger_count = 0;
    this->bike_count = 0;
    params->stack_1->top = 0;
    params->stack_2->top = 0;

    // Podnoszenie semafora o odjeździe.
    sem_post(params->sem_id_sm, 2);

    free(args_1);
    free(args_2);

    sleep(this->return_interval);
}

void init_params() {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    params->mutex = &mutex;

    const int sem_id_td = sem_alloc(SEM_TRAIN_DOOR_KEY, SEM_TRAIN_DOOR_NUM, IPC_GET);
    if (sem_id_td == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");
    params->sem_id_td = sem_id_td;

    const int sem_id_c = sem_alloc(SEM_CONDUCTOR_KEY, 2, IPC_GET);
    if (sem_id_c == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation");
    params->sem_id_c = sem_id_c;

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

    struct passenger_stack *stack_1 = shared_block_attach(SHM_TRAIN_STACK_1_KEY, sizeof(struct passenger_stack));
    if (stack_1 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    stack_1->top = 0;
    params->stack_1 = stack_1;

    struct passenger_stack *stack_2 = shared_block_attach(SHM_TRAIN_STACK_2_KEY, sizeof(struct passenger_stack));
    if (stack_2 == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    stack_2->top = 0;
    params->stack_2 = stack_2;

    const int sem_id_sm = sem_alloc(SEM_STATION_MASTER_KEY, 3, IPC_GET);
    if (sem_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Semaphore Allocation Error");
    params->sem_id_sm = sem_id_sm;

    int *shared_memory = shared_block_attach(SHM_STATION_MASTER_TRAIN_KEY, (TRAIN_NUM + 2) * sizeof(int));
    if (shared_memory == NULL) throw_error(PROCESS_NAME, "Shared Memory Attach Error");
    params->shared_memory_sm = shared_memory;

    const int msg_id_sm = message_queue_alloc(MSG_STATION_MASTER_KEY,IPC_GET);
    if (msg_id_sm == IPC_ERROR) throw_error(PROCESS_NAME, "Message Queue Allocation Error");
    params->msg_id_sm = msg_id_sm;
}
