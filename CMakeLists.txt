cmake_minimum_required(VERSION 3.31)
project(PROJEKT_SO C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

add_custom_target(rm_semaphores ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Running Bash script..."
        COMMAND bash ${CMAKE_SOURCE_DIR}/rm_semaphores.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

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

add_library(UTILITIES STATIC utilities.c)
foreach(exe ${EXECUTABLES})
    target_link_libraries(${exe} UTILITIES)
endforeach()

foreach(exe ${EXECUTABLES})
    if(exe STREQUAL "PROJEKT_SO")
        continue()
    endif()
    add_dependencies(PROJEKT_SO ${exe})
endforeach()

add_dependencies(PROJEKT_SO rm_semaphores)