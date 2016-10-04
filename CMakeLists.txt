cmake_minimum_required(VERSION 3.6)
project(CMPT300-A1P2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES shell.h shell.cpp job_list.h job_list.c)
add_executable(CMPT300-A1P2 ${SOURCE_FILES})