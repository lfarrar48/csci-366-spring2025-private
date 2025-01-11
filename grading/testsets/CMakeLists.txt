
add_library(grading_testbase testbase.cxx)
target_include_directories(grading_testbase PUBLIC .)
target_link_libraries(grading_testbase msulib gtest)

add_subdirectory(asm)
add_subdirectory(bit-tak-toe)
add_subdirectory(emulator)
add_subdirectory(firth)
add_subdirectory(sea)
add_subdirectory(zortran)