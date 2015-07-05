cmake_minimum_required(VERSION 2.8.5)

# Project details
project(divvy)
include_directories(include)

# Compiler configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")

# Enable testing
enable_testing()
add_subdirectory(dep/googletest)
add_subdirectory(test)
