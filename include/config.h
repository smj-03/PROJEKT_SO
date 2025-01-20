//
// Created by Szymon on 1/19/2025.
//

#ifndef CONFIG_H
#define CONFIG_H

// MAIN
#define MAIN_PROCESS_NUM 2

// PASSENGER
#define PASSENGER_MIN_INTERVAL 5
#define PASSENGER_MAX_INTERVAL 10
#define PASSENGER_MAX_CONCURRENCY 5
#define PASSENGER_BIKE_PROB 4 // 25%

// TRAIN DOORS
#define SEM_T_DOOR_NUM 2
#define SEM_T_DOOR_P 0x00000001
#define SEM_T_DOOR_C 0x00000002

#endif //CONFIG_H
