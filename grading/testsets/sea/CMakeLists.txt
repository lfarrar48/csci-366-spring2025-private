
add_executable(sea_grading_tests test_sea.cxx)
target_link_libraries(sea_grading_tests gtest gtest_main grading_testbase msulib SEA ASSEMBLER EMULATOR)
