############################################################
### DO NOT MODIFY THIS FILE
############################################################
project(msulib C)

add_library(msulib STATIC
        src/str.c inc/msulib/str.h
        src/strlist.c inc/msulib/strlist.h
        src/fs.c inc/msulib/fs.h
        src/parser.c inc/msulib/parser.h
        src/parsenodelist.c inc/msulib/parsenodelist.h
        src/http.c inc/msulib/http.h
        src/ws.c inc/msulib/ws.h
        src/socket.c inc/msulib/socket.h
        src/hash.c inc/msulib/hash.h
        src/alloc.c inc/msulib/alloc.h
)
target_include_directories(msulib PUBLIC inc)

if (WIN32) # windows socket things
    target_link_libraries(msulib PRIVATE ws2_32)
endif()


# testing for msulib
add_subdirectory(test)
