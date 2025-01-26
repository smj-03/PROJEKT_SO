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
#include <signal.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/prctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include <config.h>

#define IPC_ERROR -1
#define IPC_CREATE IPC_CREAT | IPC_EXCL | 0666
#define IPC_GET IPC_CREAT | 0666

#define MSG_TYPE_EMPTY 1
#define MSG_TYPE_FULL 2

struct message {
    long int mtype;
    int mvalue;
};

struct passenger_stack {
    int top;
    int data[TRAIN_MAX_CAPACITY];
};

int log_message(const char *_process_name, const char *_format, ...);

void throw_error(const char *_process_name, const char *_format, ...);

void get_process_path(char *path, const char *process_name);

int get_random_number(int min, int max);

int sem_alloc(key_t key, int number, int flags);

int sem_init(int sem_id, int number, int value);

int sem_post(int sem_id, int number);

int sem_wait(int sem_id, int number, int flags);

int sem_wait_no_op(int sem_id, int number, int flags);

int sem_destroy(int sem_id, int number);

int shared_block_alloc(key_t key, size_t size, int flags);

void *shared_block_attach(key_t key, int size);

int shared_block_detach(const void *block);

int shared_block_destroy(key_t key);

int message_queue_alloc(key_t key, int flags);

int message_queue_send(int msg_id, const struct message *message);

ssize_t message_queue_receive(int msg_id, struct message *message, long int mtype, int flags);

int message_queue_destroy(int msg_id);

int setup_signal_handler(int signal, void (*handler)(int));

int wait_for_signal(int signal);

int push(struct passenger_stack *stack, int value);

int pop(struct passenger_stack *stack);

#endif //UTILITIES_H
