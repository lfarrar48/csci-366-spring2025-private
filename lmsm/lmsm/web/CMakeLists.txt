############################################################
### DO NOT MODIFY THIS FILE
############################################################

add_executable(lmsm_ui main.c src/endpoints.c src/lang.c)
target_include_directories(lmsm_ui PRIVATE .)
target_link_libraries(lmsm_ui PRIVATE
        msulib                          # stdlib
        EMULATOR ASSEMBLER OPTIMIZER    # infrastructure
        SEA ZORTRAN FIRTH               # languages
)
