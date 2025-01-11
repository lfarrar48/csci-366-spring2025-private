cmake_minimum_required(VERSION 3.22)
project(csci366-grader)

if(NOT DEFINED STUDENT_DIR)
    message(FATAL_ERROR "STUDENT_DIR is not set!")
endif()

# set artifact path to be binary build directory
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_subdirectory("${STUDENT_DIR}" "build")
add_subdirectory(testsets)
