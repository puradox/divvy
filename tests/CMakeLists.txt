cmake_minimum_required(VERSION 2.8)

# Enable googletest
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Unit tests
add_executable(divvy_test main.cpp)

# Link to googletest
target_link_libraries(divvy_test gtest gtest_main)

# Add capability of 'make test'
add_test(sanity_test divvy_test)
