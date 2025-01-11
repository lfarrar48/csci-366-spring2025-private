cmake_minimum_required(VERSION 3.22)
project(csci366-class C)

# we will be using c11
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# set artifact path to be binary build directory
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_subdirectory(bit-tak-toe)
add_subdirectory(c)
add_subdirectory(lmsm)