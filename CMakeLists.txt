cmake_minimum_required(VERSION 3.6)
project(3000warriors)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp threadpool.c)
add_executable(3000warriors ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(3000warriors
        pthread
        event
        )