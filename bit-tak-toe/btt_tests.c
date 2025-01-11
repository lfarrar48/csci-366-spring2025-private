#include "utest.h"
#include "btt.h"

// a helper that prints the values out in binary, making it easier to see what's wrong
int bin_eq(int expected, int actual);

UTEST(update_board, player0_cell1) {
    ASSERT_TRUE(bin_eq(0b000001, bbb_update_board(0b0, 0, 1)));
}

UTEST(update_board, player1_cell1) {
    ASSERT_TRUE(bin_eq(0b000010, bbb_update_board(0b0, 1, 1)));
}

UTEST(update_board, player0_cell2) {
    ASSERT_TRUE(bin_eq(0b000100, bbb_update_board(0b0, 0, 2)));
}

UTEST(update_board, player1_cell2) {
    ASSERT_TRUE(bin_eq(0b001000, bbb_update_board(0b0, 1, 2)));
}

UTEST(update_board, player0_cell3) {
    ASSERT_TRUE(bin_eq(0b010000, bbb_update_board(0b0, 0, 3)));
}

UTEST(update_board, player1_cell3) {
    ASSERT_TRUE(bin_eq(0b100000, bbb_update_board(0b0, 1, 3)));
}

UTEST(update_board, player0_cell9) {
    ASSERT_TRUE(bin_eq(0b010000000000000000, bbb_update_board(0b0, 0, 9)));
}

UTEST(update_board, player1_cell9) {
    ASSERT_TRUE(bin_eq(0b100000000000000000, bbb_update_board(0b0, 1, 9)));
}

UTEST(owns_cell, player0_cell1) {
    ASSERT_TRUE(bbb_owns_cell(0b000001, 0, 1));
    ASSERT_FALSE(bbb_owns_cell(0b000010, 0, 1));
}

UTEST(owns_cell, player1_cell1) {
    ASSERT_FALSE(bbb_owns_cell(0b000001, 1, 1));
    ASSERT_TRUE(bbb_owns_cell(0b000010, 1, 1));
}

UTEST(owns_cell, player0_cell9) {
    ASSERT_TRUE(bbb_owns_cell(0b010000000000000000, 0, 9));
    ASSERT_FALSE(bbb_owns_cell(0b100000000000000000, 0, 9));
}

UTEST(owns_cell, player1_cell9) {
    ASSERT_FALSE(bbb_owns_cell(0b010000000000000000, 1, 9));
    ASSERT_TRUE(bbb_owns_cell(0b100000000000000000, 1, 9));
}

UTEST(get_cell_string, player0_cell1) {
    ASSERT_STREQ("O", bbb_get_cell_string(0b000001, 1));
}

UTEST(get_cell_string, player1_cell1) {
    ASSERT_STREQ("X", bbb_get_cell_string(0b000010, 1));
}

UTEST(get_cell_string, noone_cell1) {
    ASSERT_STREQ("1", bbb_get_cell_string(0b000000, 1));
}

UTEST(get_cell_string, player0_cell9) {
    ASSERT_STREQ("O", bbb_get_cell_string(0b010000000000000000, 9));
}

UTEST(get_cell_string, player1_cell9) {
    ASSERT_STREQ("X", bbb_get_cell_string(0b100000000000000000, 9));
}

UTEST(get_cell_string, noone_cell9) {
    ASSERT_STREQ("9", bbb_get_cell_string(0b000000000000000000, 9));
}

UTEST(cell_is_available, player0_cell1) {
    ASSERT_FALSE(bbb_cell_is_available(0b000001, 1));
}

UTEST(cell_is_available, player1_cell1) {
    ASSERT_FALSE(bbb_cell_is_available(0b000010, 1));
}

UTEST(cell_is_available, noone_cell1) {
    ASSERT_TRUE(bbb_cell_is_available(0b000000, 1));
}

UTEST(cell_is_available, player0_cell9) {
    ASSERT_FALSE(bbb_cell_is_available(0b010000000000000000, 9));
}

UTEST(cell_is_available, player1_cell9) {
    ASSERT_FALSE(bbb_cell_is_available(0b100000000000000000, 9));
}

UTEST(cell_is_available, noone_cell9) {
    ASSERT_TRUE(bbb_cell_is_available(0b000000000000000000, 9));
}

UTEST_MAIN()

int bin_eq(int expected, int actual) {
    if (expected != actual) {
        printf("  Expected:\n   ");
        bbb_print_board_as_binary(expected);
        printf("  Found:\n   ");
        bbb_print_board_as_binary(actual);
        return 0;
    }
    return 1;
}
