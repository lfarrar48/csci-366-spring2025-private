project(test_msulib CXX)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(test_msu_parser test_parser.cxx)
target_link_libraries(test_msu_parser gtest gtest_main msulib)

add_executable(test_msu_string test_string.cxx)
target_link_libraries(test_msu_string gtest gtest_main msulib)

add_executable(test_btree test_btree.cxx)
target_link_libraries(test_btree gtest gtest_main msulib)