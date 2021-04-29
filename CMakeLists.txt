cmake_minimum_required(VERSION 3.19)
project(frontend)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)
add_library(parser grammar.cpp)