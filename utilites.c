//
// Created by Szymon on 1/19/2025.
//

#include <stdio.h>

#include "utilities.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

int printf(const char *_format, ...) {
    const time_t now = time(NULL);
    const struct tm *local_time = localtime(&now);

    fprintf(stdout, "[%02d:%02d:%02d] ",
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec);

    va_list args;
    va_start(args, _format);

    int result = vfprintf(stdout, _format, args);

    va_end(args);

    fflush(stdout);

    return result;
}

void get_process_path(char *path, const char *process_name) {
    path[0] = '\0';
    strcat(path, "./");
    strcat(path, process_name);
}

int get_random_number(int min, int max) {
    return rand() % (max - min + 1) + min;
}
