//
// Created by Szymon on 1/19/2025.
//
#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define IPC_ERROR -1

int log_message(const char *_process_name, const char *_format, ...);

int log_error(const char *_process_name, int error_code, const char *_format, ...);

void get_process_path(char *path, const char *process_name);

int get_random_number(int min, int max);

int sem_alloc(key_t key, int number, int flags);

int sem_init(int sem_id, int number, int value);

int sem_post(int sem_id, int number);

int sem_wait(int sem_id, int number, int flags);

int sem_destroy(int sem_id, int number);

int shared_block_alloc(key_t key, size_t size);

void *shared_block_attach(key_t key, int size);

int shared_block_detach(const void *block);

int shared_block_destroy(key_t key);

#endif //UTILITIES_H
