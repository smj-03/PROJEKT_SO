//
// Created by Szymon on 1/19/2025.
//

#ifndef CONFIG_H
#define CONFIG_H

// GENERAL
#define VERBOSE_LOGS 0

// PLATFORM
#define PLATFORM_CLOSE_AFTER 15

// PASSENGER
#define PASSENGER_MIN_INTERVAL 1
#define PASSENGER_MAX_INTERVAL 10
#define PASSENGER_SOLO_PROB 3 // 66%
#define PASSENGER_MAX_CONCURRENCY 5
#define PASSENGER_BIKE_PROB 4 // 25%
#define PASSENGER_BOARDING_TIME 3

// TRAIN
#define TRAIN_NUM 2 // N
#define TRAIN_DEPART_TIME 10 // T
#define TRAIN_P_LIMIT 100 // P
#define TRAIN_B_LIMIT 50  // R
#define TRAIN_MIN_RETURN_INTERVAL 30
#define TRAIN_MAX_RETURN_INTERVAL 40

// TRAIN-PASSENGER IPC
#define SEM_TRAIN_DOOR_NUM 2
#define SEM_TRAIN_ARRIVAL_KEY 0x00000000
#define SEM_TRAIN_DOOR_KEY 0x00000001

#define SHM_TRAIN_DOOR_1_KEY 0x00000002
#define SHM_TRAIN_DOOR_2_KEY 0x00000003

#define MSG_TRAIN_DOOR_1_KEY 0x00000004
#define MSG_TRAIN_DOOR_2_KEY 0x00000005

#define SHM_TRAIN_STACK_1_KEY 0x00000006
#define SHM_TRAIN_STACK_2_KEY 0x00000007

// STATION MASTER-TRAIN IPC
#define SEM_STATION_MASTER_KEY 0x00000008
#define MSG_STATION_MASTER_KEY 0x00000009
#define SHM_STATION_MASTER_TRAIN_KEY 0x0000000A
#define SHM_STATION_MASTER_PLATFORM_KEY 0x0000000B

// PLATFORM
#define SEM_PLATFORM_KEY 0x0000000C

#endif //CONFIG_H
