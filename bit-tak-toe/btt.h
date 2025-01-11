
#ifndef BBB_H
#define BBB_H

static const int COMPUTER_PLAYER = 0;
static const int HUMAN_PLAYER = 1;
static const int CATS_GAME = 2;
static const int GAME_ONGOING = -1;

// Start the game
void bbb_start_game();

// Prints the board in a human-readable format
void bbb_print_board(int board);

// Prints the board as binary
void bbb_print_board_as_binary(int board);

// Returns "X" if the human has claimed the given cell, "O" if the computer has, and the cell number as a string
// otherwise
const char *bbb_get_cell_string(int board, int cell);

// Asks the human to enter a valid cell to claim
int bbb_get_human_move(int board);

// Gets a random valid cell for the computer to claim
int bbb_get_computer_move(int board);

// Returns 1 if the given player owns the given cell in the given board, 0 otherwise
int bbb_owns_cell(int board, int player, int cell);

// Returns an updated board representation, with the given player owning the given cell in the result
int bbb_update_board(int board, int player, int cell);

int bbb_cell_is_available(int board, int cell);

int bbb_cells_still_available(int board, int cell1, int cell2, int cell3);
int bbb_player_owns_at_least_one_cell(int board, int player, int cell1, int cell2, int cell3);
int bbb_player_owns_all_cells(int board, int player, int cell1, int cell2, int cell3);

int bbb_game_result(int board);


#endif