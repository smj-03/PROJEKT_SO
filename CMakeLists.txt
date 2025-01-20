cmake_minimum_required(VERSION 3.31)
project(PROJEKT_SO C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_target(rm_semaphores ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Running Bash script..."
        COMMAND bash ${CMAKE_SOURCE_DIR}/rm_semaphores.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set(EXECUTABLES
        MAIN
        TRAIN
        PASSENGER_FACTORY
        PASSENGER
)

add_executable(MAIN main.c)
add_executable(TRAIN train.c)
add_executable(PASSENGER_FACTORY passenger_factory.c)
add_executable(PASSENGER passenger.c)

add_library(UTILITIES STATIC utilities.c)
add_library(CONFIG STATIC config.c)
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

add_dependencies(MAIN rm_semaphores)