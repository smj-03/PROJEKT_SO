//
// Created by Szymon on 1/19/2025.
//
#include "utilities.h"

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

    fprintf(stdout, "[%s%s%s] ",
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

int log_error(const char *_process_name, const int error_code, const char *_format, ...) {
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
            error_code,
            ANSI_COLOR_RESET);

    va_list args;
    va_start(args, _format);

    int result = vfprintf(stderr, _format, args);

    va_end(args);

    if (error_code != 0) {
        fprintf(stderr, ": %s\n", strerror(error_code));
    } else fprintf(stderr, "\n");


    fflush(stderr);

    return result;
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

int sem_destroy(const int sem_id, const int number) {
    return semctl(sem_id, number, IPC_RMID, NULL);
}

int init_semaphores(const int name, const int number, const char *process_name) {
    const key_t key = ftok(".", name);
    if (key == -1) {
        log_error(process_name, errno, "%s Key Creation Error", name);
        exit(1);
    }

    const int sem_id = sem_alloc(key, number, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id == -1) {
        log_error(process_name, errno, "%s Semaphore Allocation Error", name);
        exit(1);
    }

    for (int i = 0; i < number; i++) {
        const int init_res = sem_init(sem_id, i, 0);
        if (init_res == -1) {
            log_error(process_name, errno, "%s Semaphore Control Error", name);
            exit(1);
        }
    }

    return sem_id;
}

int get_semaphores(int name, const int number, const char *process_name) {
    const key_t key = ftok(".", name);
    if (key == -1) {
        log_error(process_name, errno, "%s Key Creation Error", name);
        exit(1);
    }
    const int sem_id = sem_alloc(key, number, IPC_EXCL | 0666);
    if (sem_id == -1) {
        log_error(process_name, errno, "%s Semaphore Allocation Error", name);
        exit(1);
    }

    return sem_id;
}
