cmake_minimum_required(VERSION 3.6)
project(my_shell)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES shell.h shell.cpp)
add_executable(my_shell ${SOURCE_FILES})
