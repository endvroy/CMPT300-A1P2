cmake_minimum_required(VERSION 3.6)
project(CMPT300-A1P2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES shell.c)
add_executable(CMPT300-A1P2 ${SOURCE_FILES})