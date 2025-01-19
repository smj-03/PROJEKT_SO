cmake_minimum_required(VERSION 3.31)
project(PROJEKT_SO C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

set(EXECUTABLES
        TRAIN
        PASSENGER_FACTORY
        PASSENGER
        PROJEKT_SO
)

add_executable(TRAIN train.c)
add_executable(PASSENGER_FACTORY passenger_factory.c)
add_executable(PASSENGER passenger.c)
add_executable(PROJEKT_SO main.c)

add_library(UTILITES STATIC utilites.c)
foreach(exe ${EXECUTABLES})
    target_link_libraries(${exe} UTILITES)
endforeach()

foreach(exe ${EXECUTABLES})
    if(exe STREQUAL "PROJEKT_SO")
        continue()
    endif()
    add_dependencies(PROJEKT_SO ${exe})
endforeach()