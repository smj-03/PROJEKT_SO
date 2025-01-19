cmake_minimum_required(VERSION 3.31)
project(PROJEKT_SO C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

add_library(UTILITES STATIC utilites.c)

add_executable(PASSENGER_FACTORY passenger_factory.c)
target_link_libraries(PASSENGER_FACTORY UTILITES)

add_executable(PASSENGER passenger.c)

add_executable(PROJEKT_SO main.c utilities.h)
target_link_libraries(PROJEKT_SO UTILITES)

add_dependencies(PROJEKT_SO PASSENGER_FACTORY)
add_dependencies(PROJEKT_SO PASSENGER)
