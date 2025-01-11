#include <stdio.h>

int my_int_function(int *pointer_to_int);

void pointer_fun_1() {
    int i = 10;
    int *pointer_to_i = &i;

    printf("i is %u\n", i);
    printf("pointer_to_i is %u\n",
            pointer_to_i);
    printf(" value at pointer_to_i is %d\n",
            my_int_function(pointer_to_i));
    return;
}

void pointer_fun_2() {
    int i = 10;
    int j = 10;
    char *str = "Hello Pointers...";

    printf("i addr is %u\n", &i);
    printf("j addr is %u\n", &j);
    printf("str addr is %u\n", str);
}

int* get_addr_of_10(){
    int i = 10;
    return &i;
}

int my_int_function(int *pointer_to_int){
    // what can I do with pointer_to_int?
    return *pointer_to_int;
}

void show_array_1() {
    int int_arr[] = {1, 2, 3};
    for (int i = 0; i < 3; ++i) {
        printf("Value: %d\n", int_arr[i]);
    }
}

void show_array_2() {
    int int_arr[4] = {0};
    for (int i = 0; i < 4; ++i) {
        printf("Value: %d\n", int_arr[i]);
    }
}

void show_array_3() {
    int int_arr[4];
    for (int i = 0; i < 4; ++i) {
        printf("Value: %d\n", int_arr[i]);
    }
}

void show_array2() {
    int int_arr[4] = {0};
    for (int i = 0; i < 4; ++i) {
        printf("Value: %d\n", int_arr[i]);
    }
}

void pointers_and_arrays() {
    int int_arr[] = {1, 2, 3};
    int *int_ptr = int_arr;
    for (int i = 0; i < 3; ++i) {
        printf("Value from arr: %d\n", int_arr[i]);
        int_ptr++;
        printf("Value from ptr: %d\n", *int_ptr);
    }
}

int update_it1(int i) {
    i = 42;
}

int update_it2(int *i) {
    *i = 42;
}

struct demo {
    int value;
};

int update_it3(struct demo s) {
    printf("Address : %u\n", &s);
    s.value = 42;
}

int update_it4(struct demo *s) {
    s->value = 42;
}

int addOne(int i) {
    return i + 1;
}

int addTwo(int i) {
    return i + 2;
}

void function_ptr_example() {

    int (*func)(int) = addOne;

    printf("func(1): %i\n", func(1));

    func = addTwo;
    printf("func(1): %i\n", func(1));
}


void read_file(char *filename, char* buffer, int maxlen) {
    // TODO implement
}

void process(char* buffer) {
    // TODO implement
}

void process_file_content() {
    int max_input = 2000;
    char input_buffer[max_input];
    read_file("/tmp/foo.txt", input_buffer, max_input);

    process(input_buffer);
    // do something w/ the input_buffer

    return; // on exit, the buffer is automatically reclaimed
}



#include <string.h>
void strtok_example() {
    char str[] = "Strings in C is fun!";
    char *token;
    char *str_ptr = str;
    char **rest_of_str_ptr = &str;

    while ((token = strtok(str_ptr, " ")))
        printf("%s\n", token);


}

int main() {
    pointer_fun_1();
}

