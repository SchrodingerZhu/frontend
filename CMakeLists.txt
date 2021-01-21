cmake_minimum_required(VERSION 3.19)
project(frontend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -g -flto")
include_directories(include)
add_library(parser grammar.cpp)