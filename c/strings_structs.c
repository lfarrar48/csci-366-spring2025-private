#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void string_ptr_fun_0() {
    char * str = "Hello";
    *str = 'J';
    printf("%s", str);
}

void string_ptr_fun_heap_fixed() {
    char * str = "Hello";
    char *strCopy = calloc(1, strlen(str) + 1);
    strcpy(strCopy, str);
    *strCopy = 'J';
    printf("%s", strCopy);
}

void string_ptr_fun_stack_fixed() {
    char * str = "Hello";
    char strCopy[strlen(str) + 1];
    strcpy(strCopy, str);
    *strCopy = 'J';
    printf("%s", strCopy);
}

void string_ptr_fun_00() {

    char * str = "Hello";
    printf("Char at %d is %c\n", str, *str);
    str++;
    printf("Char at %d is %c\n", str, *str);
    str++;
    printf("Char at %d is %c\n", str, *str);


}

void string_ptr_fun_1() {
    char *str = "Hello, Strings!";
    for (int i = 0; i < 15; ++i) {
        printf("Char at %d is %c\n", i, *(str + i));
    }
}

void string_ptr_fun_2() {
    char *str = "Hello, Strings!";
    for (int i = 0; i < 16; ++i) {
        printf("Char at %d is %d\n", i, *(str + i));
    }
}

void string_ptr_fun_3() {
    char *str = "Hello, Strings!";
    *(str + 15) = 'a';
    printf(str);
}

void string_functions() {
    char buff[100];

    strcpy(buff, "Hello strcpy");
    printf("String is '%s'\n", buff);

    strcat(buff, " & strcat");
    printf("String is '%s'\n", buff);

    printf("String length is '%lu'\n", strlen(buff));

    printf("strcmp '%d'\n", strcmp(buff, "Hello strcpy & strcat"));

    printf("strcmp '%d'\n", strcmp(buff, "Hello strncpy & strncat"));

    char * str = strchr(buff, '&');
    printf("str is '%s'\n", str);

    str = strstr(buff, "strcpy");
    printf("str is '%s'\n", str);


}

void string_ptr_fun_4() {
    char *str = "Hello, Strings!";
    char *str_on_heap = malloc(16 * sizeof(char));
    strcpy(str_on_heap, str);
    printf(str_on_heap);
}

void str_tok_example() {
    char *str ="A Sample String";
    char * token;
    token = strtok (str, " ");
    while (token != NULL)
    {
        printf ("%s\n", token);
        token = strtok (NULL, " ");
    }
}

void str_tok_example_fixed() {
    char *str ="A Sample String";
    char *strCopy = calloc(1, strlen(str) + 1);
    strcpy(strCopy, str);
    char * token;
    token = strtok (strCopy, " ");
    while (token != NULL)
    {
        printf ("%s\n", token);
        token = strtok (NULL, " ");
    }
}

void buffer_overflow(){
    char buff[15];

    int pass = 0;

    printf("\n Enter the password : \n");
    gets(buff);

    if(strcmp(buff, "thegeekstuff") != 0)
    {
        printf ("\n Wrong Password \n");
    }
    else
    {
        printf ("\n Correct Password \n");
        pass = 1;
    }

    if(pass)
    {
        /* Now Give root or admin rights to user*/
        printf ("\n Root privileges given to the user \n");
    }
}

typedef struct Book {
    char  title[20];
    char  author[50];
    char  subject[100];
    int   book_id;
} Book;

void fun_with_structs() {

    Book bk;
    strncpy(bk.title, "Fun with C", 9);
    printf("Title: %s\n", bk.title);

    printf("Author: %s\n", bk.author);
    strncpy(bk.author, "Carson", 9);
    printf("Title: %s\n", bk.title);
    printf("Author: %s\n", bk.author);
}

void bk_print_title(Book *book){
    printf("Title: %s\n", book->title);
}

void fun_with_structs2() {
    Book bk;
    strcpy(bk.title, "Fun with C");
    bk_print_title(&bk);
}

void func3(int i) {
    i = 3;
}

void func2(Book b) {
    b.book_id = 10;
}

void struct_fun_2() {
    Book b;
    Book *bPtr = &b;
    b.book_id = 1;
    func2(b);
    printf("Book ID: %i\n", b.book_id);

    int i = 10;
    func3(i);
    printf("i: %i\n", i);

}

void update_title(Book bk) {
    printf("Address: %i\n", &bk);
    strcpy(bk.title, "Pain with C");
}

void demo_pass_by_value() {
    Book bk;
    strcpy(bk.title, "Fun with C");
    printf("Address: %i\n", &bk);
    printf("Title: %s\n", bk.title);
    update_title(bk);
    printf("Title: %s\n", bk.title);
}

void update_title_by_ref(Book *bookPointer) {
    printf("Address: %i\n", bookPointer);
    strcpy(bookPointer->title, "Pain with C");
}

void demo_pass_by_reference() {
    Book bk;
    strcpy(bk.title, "Fun with C");
    printf("Address: %i\n", &bk);
    printf("Title: %s\n", bk.title);
    Book *bookPointer = &bk;
    update_title_by_ref(bookPointer);
    printf("Title: %s\n", bk.title);
}

int main() {
    demo_pass_by_reference();
}
