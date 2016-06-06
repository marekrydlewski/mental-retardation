cmake_minimum_required(VERSION 3.5)
project(mental_retardation)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    LamportClock.h
    main_mpi.cpp
    Message.h)

add_executable(mental_retardation ${SOURCE_FILES})