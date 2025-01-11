project(366_demos C CXX)

add_executable(intro intro.c)
add_executable(functions functions.c functions.h)
add_executable(pointers pointers.c)
add_executable(bit_manipulation bit_manipulation.c)
add_executable(strings_structs strings_structs.c)
#target_compile_options(strings_structs PRIVATE -fno-stack-protector)
add_executable(stack_and_heap stack_and_heap.c)
add_executable(sockets sockets.c)
add_executable(concurrency concurrency.c)
target_link_libraries(concurrency PUBLIC pthread)

add_executable(linking linking.c linking2.c linking.h)

add_executable(cpp CPPDemo.h CPPDemo.cpp)