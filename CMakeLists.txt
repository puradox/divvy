cmake_minimum_required(VERSION 2.8)

# Project details
project(divvy)
include_directories(include)

# CMake configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")

# Add executable
add_executable(${PROJECT_NAME} tests/main.cpp)
