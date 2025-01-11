#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "btt.h"

char *NUMBER_STRS[10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

void bbb_start_game() {
    srandom(time(NULL)); // seed random()
    int game_board = 0;  // reset the board
    printf("Welcome to bit-bac-boe!\n\n");
    bbb_print_board(game_board);
    do {
        // player moves
        int human_move = bbb_get_human_move(game_board);
        printf("Player picks %i...\n", human_move);
        game_board = bbb_update_board(game_board, HUMAN_PLAYER, human_move);

        // print state
        bbb_print_board(game_board);
        printf("  Board As Binary: ");
        bbb_print_board_as_binary(game_board);

        if (bbb_game_result(game_board) == GAME_ONGOING) {
            printf("Computer moving...\n");
            sleep(2);
            int computer_move = bbb_get_computer_move(game_board);
            printf("Computer picks %i...\n", computer_move);
            game_board = bbb_update_board(game_board, COMPUTER_PLAYER, computer_move);

            bbb_print_board(game_board);
            printf("  Board As Binary: ");
            bbb_print_board_as_binary(game_board);
        }
    } while (bbb_game_result(game_board) == GAME_ONGOING);

    int result = bbb_game_result(game_board);
    if (result == COMPUTER_PLAYER) {
        printf("Computer won!");
    } else if (result == HUMAN_PLAYER) {
        printf("You won!");
    } else {
        printf("Cat's Game!");
    }
}

int bbb_game_result(int board) {
    // test if player has won
    for (int player = COMPUTER_PLAYER; player <= HUMAN_PLAYER; ++player) {
        if (bbb_player_owns_all_cells(board, player, 1, 2, 3) || // rows
            bbb_player_owns_all_cells(board, player, 4, 5, 6) ||
            bbb_player_owns_all_cells(board, player, 7, 8, 9) ||
            bbb_player_owns_all_cells(board, player, 1, 4, 7) || // columns
            bbb_player_owns_all_cells(board, player, 2, 5, 8) ||
            bbb_player_owns_all_cells(board, player, 3, 6, 9) ||
            bbb_player_owns_all_cells(board, player, 1, 5, 9) || // diagonals
            bbb_player_owns_all_cells(board, player, 3, 5, 7)) {
            return player;
        }
    }
    // test if cats game
    if (!(bbb_cells_still_available(board, 1, 2, 3) || // rows
          bbb_cells_still_available(board, 4, 5, 6) ||
          bbb_cells_still_available(board, 7, 8, 9) ||
          bbb_cells_still_available(board, 1, 4, 7) || // columns
          bbb_cells_still_available(board, 2, 5, 8) ||
          bbb_cells_still_available(board, 3, 6, 9) ||
          bbb_cells_still_available(board, 1, 5, 9) || // diagonals
          bbb_cells_still_available(board, 3, 5, 7))) {
        return CATS_GAME;
    }
    // game can continue
    return GAME_ONGOING;
}

int bbb_get_computer_move(int board) {
    while(1) {
        long rand = random();
        int random_cell = rand % 8 + 1;
        if (bbb_cell_is_available(board, random_cell)) {
            return random_cell;
        }
    }
}

int bbb_get_human_move(int board) {
    while (1) {
        int selected_cell;
        printf("Enter an empty cell between 1 & 9: ");
        if (scanf("%d", &selected_cell) == 1 && 0 < selected_cell && selected_cell < 10 &&
            bbb_cell_is_available(board, selected_cell)) {
            return selected_cell;
        } else {
            printf("Please enter a valid empty cell value between 1 and 9\n\n");
        }
    }
}

int bbb_player_owns_all_cells(int board, int player, int cell1, int cell2, int cell3) {
    int owns_cell1 = bbb_owns_cell(board, player, cell1);
    int owns_cell2 = bbb_owns_cell(board, player, cell2);
    int owns_cell3 = bbb_owns_cell(board, player, cell3);
    return owns_cell1 && owns_cell2 && owns_cell3;
}

int bbb_cells_still_available(int board, int cell1, int cell2, int cell3) {
    int computer_owns_at_least_one = bbb_player_owns_at_least_one_cell(board, COMPUTER_PLAYER, cell1, cell2, cell3);
    int human_owns_at_least_one = bbb_player_owns_at_least_one_cell(board, HUMAN_PLAYER, cell1, cell2, cell3);
    return !(computer_owns_at_least_one && human_owns_at_least_one);
}

int bbb_player_owns_at_least_one_cell(int board, int player, int cell1, int cell2, int cell3) {
    int owns_cell1 = bbb_owns_cell(board, player, cell1);
    int owns_cell2 = bbb_owns_cell(board, player, cell2);
    int owns_cell3 = bbb_owns_cell(board, player, cell3);
    return owns_cell1 || owns_cell2 || owns_cell3;
}

int bbb_cell_is_available(int board, int cell) {
    int computer_owns_cell = bbb_owns_cell(board, COMPUTER_PLAYER, cell);
    int human_owns_cell = bbb_owns_cell(board, HUMAN_PLAYER, cell);
    return !computer_owns_cell && !human_owns_cell;
}

int bbb_owns_cell(int board, int player, int cell) {
    // TODO compute a mask for the offset for the given player return 0 if the player owns the bit and non-zero otherwise
    return 0;
}

int bbb_update_board(int board, int player, int cell) {
    // TODO compute a mask for the offset for the given player and cell and flip that bit to 1
    return board;
}

const char *bbb_get_cell_string(int board, int cell) {
    // TODO - test if the cell belongs to COMPUTER_PLAYER and return an O or the human player and return an X
    return NUMBER_STRS[cell];
}

void bbb_print_board(int board) {
    printf("\nBoard State:\n\n");
    printf("  ");
    printf("%s", bbb_get_cell_string(board, 1));
    printf(" | ");
    printf("%s", bbb_get_cell_string(board, 2));
    printf(" | ");
    printf("%s\n", bbb_get_cell_string(board, 3));
    printf("  ----------\n");
    printf("  ");
    printf("%s", bbb_get_cell_string(board, 4));
    printf(" | ");
    printf("%s", bbb_get_cell_string(board, 5));
    printf(" | ");
    printf("%s\n", bbb_get_cell_string(board, 6));
    printf("  ----------\n");
    printf("  ");
    printf("%s", bbb_get_cell_string(board, 7));
    printf(" | ");
    printf("%s", bbb_get_cell_string(board, 8));
    printf(" | ");
    printf("%s\n\n", bbb_get_cell_string(board, 9));
}

void bbb_print_board_as_binary(int board) {
    printf("0b");
    for (int i = (sizeof(int) * 8) - 1; i >= 0; i--) {
        int bit = (board >> i) & 1;
        printf("%d", bit);
        if (i % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}