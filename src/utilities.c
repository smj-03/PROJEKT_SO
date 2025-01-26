//
// Created by Szymon on 1/19/2025.
//
#include <utilities.h>

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int log_message(const char *_process_name, const char *_format, ...) {
    const time_t now = time(NULL);
    const struct tm *local_time = localtime(&now);

    fprintf(stdout, "[%02d:%02d:%02d]",
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec);

    fprintf(stdout, "[%s%s%s]",
            ANSI_COLOR_GREEN,
            _process_name,
            ANSI_COLOR_RESET);

    va_list args;
    va_start(args, _format);

    const int result = vfprintf(stdout, _format, args);

    va_end(args);

    fflush(stdout);

    return result;
}

void throw_error(const char *_process_name, const char *_format, ...) {
    const time_t now = time(NULL);
    const struct tm *local_time = localtime(&now);

    fprintf(stderr, "[%s%02d:%02d:%02d%s]",
            ANSI_COLOR_RED,
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec,
            ANSI_COLOR_RESET);

    fprintf(stderr, "[%s%s%s]",
            ANSI_COLOR_RED,
            _process_name,
            ANSI_COLOR_RESET);

    fprintf(stderr, "[%sERROR %d%s] ",
            ANSI_COLOR_RED,
            errno,
            ANSI_COLOR_RESET);

    va_list args;
    va_start(args, _format);

    vfprintf(stderr, _format, args);

    va_end(args);

    if (errno != 0) {
        fprintf(stderr, ": %s\n", strerror(errno));
    } else fprintf(stderr, "\n");


    fflush(stderr);

    exit(1);
}

void get_process_path(char *path, const char *process_name) {
    path[0] = '\0';
    strcat(path, "./");
    strcat(path, process_name);
}

int get_random_number(const int min, const int max) {
    return rand() % (max - min + 1) + min;
}

int sem_alloc(const key_t key, const int number, const int flags) {
    return semget(key, number, flags);
}

int sem_init(const int sem_id, const int number, const int value) {
    return semctl(sem_id, number, SETVAL, value);
}

int sem_post(const int sem_id, const int number) {
    struct sembuf operations[1];
    operations[0].sem_num = number;
    operations[0].sem_op = 1;

    return semop(sem_id, operations, 1);
}

int sem_wait(const int sem_id, const int number, const int flags) {
    struct sembuf operations[1];
    operations[0].sem_num = number;
    operations[0].sem_op = -1;
    operations[0].sem_flg = 0 | flags;

    return semop(sem_id, operations, 1);
}

int sem_wait_no_op(const int sem_id, const int number, const int flags) {
    struct sembuf operations[1];
    operations[0].sem_num = number;
    operations[0].sem_op = 0;
    operations[0].sem_flg = 0 | flags;

    return semop(sem_id, operations, 1);
}

int sem_destroy(const int sem_id, const int number) {
    return semctl(sem_id, number, IPC_RMID, NULL);
}

int shared_block_alloc(const key_t key, const size_t size, const int flags) {
    return shmget(key, size, flags);
}

void *shared_block_attach(const key_t key, int size) {
    const int shared_block_id = shared_block_alloc(key, size, IPC_GET);
    if(shared_block_id == IPC_ERROR) return NULL;

    char *result = shmat(shared_block_id, NULL, 0);
    if(result == (char *) IPC_ERROR) return NULL;

    return result;
}

int shared_block_detach(const void *block) {
    return shmdt(block);
}

int shared_block_destroy(const key_t key) {
    const int shared_block_id = shared_block_alloc(key, 0, IPC_GET);
    if(shared_block_id == IPC_ERROR) return -1;

    return shmctl(shared_block_id, IPC_RMID, NULL);
}

int message_queue_alloc(const key_t key, const int flags) {
    return msgget(key, flags);
}

int message_queue_send(const int msg_id, const struct message *message) {
    return msgsnd(msg_id, message, sizeof(message->mvalue), 0);
}

ssize_t message_queue_receive(const int msg_id, struct message *message, const long int mtype, const int flags) {
    return msgrcv(msg_id, message, sizeof(message->mvalue), mtype, flags);
}

int message_queue_destroy(const int msg_id) {
    return msgctl(msg_id, IPC_RMID, NULL);
}

int setup_signal_handler(int signal, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(signal, &sa, NULL) == -1) return -1;
    return 0;
}

int wait_for_signal(const int signal) {
    sigset_t new_set, old_set;
    int sig;

    sigemptyset(&new_set);
    sigaddset(&new_set, signal);

    if (sigprocmask(SIG_BLOCK, &new_set, &old_set) == -1) return -1;

    if (sigwait(&new_set, &sig) == -1) {
        sigprocmask(SIG_SETMASK, &old_set, NULL);
        return -1;
    }

    if(sigprocmask(SIG_SETMASK, &old_set, NULL) == -1) return -1;

    return sig;
}
