cmake_minimum_required(VERSION 3.8)
project(queue_thread)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        queue_calc.cpp
        queue_calc.h)

include_directories(.)

add_executable(queue_calc ${SOURCE_FILES})