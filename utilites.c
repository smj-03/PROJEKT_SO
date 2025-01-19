//
// Created by Szymon on 1/19/2025.
//

#include "utilities.h"
#include <string.h>

void get_process_path(char* path, const char* process_name) {
    path[0] = '\0';
    strcat(path, "./");
    strcat(path, process_name);
}

