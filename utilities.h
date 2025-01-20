//
// Created by Szymon on 1/19/2025.
//
#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/errno.h>

int log_message(const char *_process_name, const char *_format, ...);

int log_error(const char *_process_name, int error_code, const char *_format, ...);

void get_process_path(char *path, const char *process_name);

int get_random_number(int min, int max);

int sem_alloc(key_t key, int number, int flags);

int sem_init(int sem_id, int number, int value);

int sem_post(int sem_id, int number);

int sem_wait(int sem_id, int number, int flags);

int sem_destroy(int sem_id, int number);

int init_semaphores(int name,  int number, const char *process_name);

int get_semaphores(int name,  int number, const char *process_name);

#endif //UTILITIES_H
