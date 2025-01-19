//
// Created by Szymon on 1/19/2025.
//
#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int log_message(const char *_process_name, const char *_format, ...);

int log_error(const char *_process_name, int error_code, const char *_format, ...);

void get_process_path(char *path, const char *process_name);

int get_random_number(int min, int max);

#endif //UTILITIES_H
