cmake_minimum_required(VERSION 3.31)
project(PROJEKT_SO C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

set(EXECUTABLES
        TRAIN
        PLATFORM
        PASSENGER
        STATION_MASTER
        CONDUCTOR
        MAIN
)

add_executable(TRAIN src/train.c)
add_executable(PLATFORM src/platform.c)
add_executable(PASSENGER src/passenger.c)
add_executable(CONDUCTOR src/conductor.c)
add_executable(STATION_MASTER src/station_master.c)
add_executable(MAIN src/main.c)

add_library(UTILITIES STATIC src/utilities.c)
add_library(CONFIG STATIC src/config.c)

foreach(exe ${EXECUTABLES})
    target_link_libraries(${exe} UTILITIES)
    target_link_libraries(${exe} CONFIG)
endforeach()

foreach(exe ${EXECUTABLES})
    if(exe STREQUAL "MAIN")
        continue()
    endif()
    add_dependencies(MAIN ${exe})
endforeach()