project(bit-tac-toe C CXX)

add_library(bbb btt.c)
target_include_directories(bbb PUBLIC .)

add_executable(bbb_cli btt_cli.c)
target_link_libraries(bbb_cli bbb)

add_executable(bbb-tests btt_tests.c)
target_link_libraries(bbb-tests bbb)
