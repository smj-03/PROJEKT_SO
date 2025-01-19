cmake_minimum_required(VERSION 3.31)
project(PROJEKT_SO C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

add_executable(PASSENGER_FACTORY passenger_factory.c)

add_executable(PASSENGER passenger.c)

add_executable(PROJEKT_SO main.c)
add_dependencies(PROJEKT_SO PASSENGER_FACTORY)
add_dependencies(PROJEKT_SO PASSENGER)
