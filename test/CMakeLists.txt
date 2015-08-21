cmake_minimum_required(VERSION 2.8.5)

# Unit tests
add_executable(divvy_test main.cpp cases.cpp)

# Add capability of 'make test'
add_test(sanity_test divvy_test)