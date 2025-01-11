project(Google_tests CXX)

# add gtest
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_library(testbase testbase.cxx testbase.hxx)
target_sources(testbase PUBLIC testbase.hxx)
target_link_libraries(testbase PRIVATE msulib gtest)

add_executable(asm_tests test_asm.cxx)
target_link_libraries(asm_tests gtest gtest_main msulib ASSEMBLER testbase)

add_executable(zortran_tests test_zortran.cxx)
target_link_libraries(zortran_tests gtest gtest_main msulib ZORTRAN ASSEMBLER EMULATOR testbase)

add_executable(firth_tests test_firth.cxx)
target_link_libraries(firth_tests gtest gtest_main msulib FIRTH ASSEMBLER EMULATOR testbase)

add_executable(sea_tests test_sea.cxx)
target_link_libraries(sea_tests gtest gtest_main msulib SEA ASSEMBLER EMULATOR testbase)

add_executable(emulator_tests test_emulator.cxx)
target_link_libraries(emulator_tests gtest gtest_main msulib EMULATOR testbase)
