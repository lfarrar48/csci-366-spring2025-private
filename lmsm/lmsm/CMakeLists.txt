############################################################
### DO NOT MODIFY THIS FILE
############################################################
project(csci366 C)

add_library(ZORTRAN STATIC src/zortran.c inc/lmsm/zortran.h)
target_include_directories(ZORTRAN PUBLIC inc)
target_link_libraries(ZORTRAN PRIVATE msulib)

add_library(FIRTH STATIC src/firth.c inc/lmsm/firth.h)
target_include_directories(FIRTH PUBLIC inc)
target_link_libraries(FIRTH PRIVATE msulib)

add_library(SEA STATIC src/sea.c inc/lmsm/sea.h)
target_include_directories(SEA PUBLIC inc)
target_link_libraries(SEA PRIVATE msulib)

add_library(EMULATOR STATIC src/emulator.c inc/lmsm/emulator.h)
target_include_directories(EMULATOR PUBLIC inc)
target_link_libraries(EMULATOR PRIVATE msulib)

add_library(ASSEMBLER STATIC src/asm.c inc/lmsm/asm.h src/asm_insrlist.c inc/lmsm/asm_insrlist.h)
target_include_directories(ASSEMBLER PUBLIC inc)
target_link_libraries(ASSEMBLER PRIVATE msulib)

add_library(OPTIMIZER STATIC src/opt.c inc/lmsm/opt.h)
target_include_directories(OPTIMIZER PUBLIC inc)
target_link_libraries(OPTIMIZER PRIVATE msulib)

add_subdirectory(web)