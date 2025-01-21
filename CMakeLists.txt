cmake_minimum_required(VERSION 3.31)
project(PROJEKT_SO C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_target(rm_ipc_ids ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Running Bash script..."
        COMMAND bash ${CMAKE_SOURCE_DIR}/rm_ipc_ids.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

include_directories(include)

set(EXECUTABLES
        TRAIN
        PASSENGER_FACTORY
        PASSENGER
        MAIN
)

add_executable(TRAIN src/train.c)
add_executable(PASSENGER_FACTORY src/passenger_factory.c)
add_executable(PASSENGER src/passenger.c)
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

add_dependencies(MAIN rm_ipc_ids)