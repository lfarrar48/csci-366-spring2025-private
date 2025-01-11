# bit-tac-toe

bit-tac-toe is a variation of the classic game tic-tac-toe.  It is a command-line implementation
where a human plays a computer, with each takeing turns selecting a cell in a 3x3 grid:

```
1 | 2 | 3
---------
4 | 5 | 6
---------
7 | 8 | 9
```

The human player is X's goes first.  The computer is 0's.

The game will print the current state of the board and ask the human to make a move.  The computer
will then make a move.  This will repeat until the human or computer wins, or a "Cat's Game" is declared.

When a board is printed, each cell will either display the number of that cell, `X` if the human has claimed it or
`O` (capital oh) if the computer has claimed it.  

Here is an example in which the human has claimed cell 1, and the computer has claimed cell 8:

```
X | 2 | 3
---------
4 | 5 | 6
---------
7 | O | 9
```

## Game State

Game state in bit-tac-toe is stored in a *single* 32-bit integer, `GAME_BOARD`, found in `bbb.h`.  This 
makes the game very memory efficient and will give you an opportunity to do some raw bit manipulation as you implement
the game.

Below are the meaning of the first 18 bits in the game state integer:

| Bit   | Meaning         | Values                                 |
|-------|-----------------|----------------------------------------|
| 0     | Cell 1 Computer | 0 if Computer has claimed, 0 otherwise |
| 1     | Cell 1 Human    | 0 if Human has claimed, 0 otherwise    |
| 2     | Cell 2 Computer | 0 if Computer has claimed, 0 otherwise |
| 3     | Cell 2 Human    | 0 if Human has claimed, 0 otherwise    |
| 4     | Cell 3 Computer | 0 if Computer has claimed, 0 otherwise |
| 5     | Cell 3 Human    | 0 if Human has claimed, 0 otherwise    |
| 6     | Cell 4 Computer | 0 if Computer has claimed, 0 otherwise |
| 7     | Cell 4 Human    | 0 if Human has claimed, 0 otherwise    |
| 8     | Cell 5 Computer | 0 if Computer has claimed, 0 otherwise |
| 9     | Cell 5 Human    | 0 if Human has claimed, 0 otherwise    |
| 10    | Cell 6 Computer | 0 if Computer has claimed, 0 otherwise |
| 11    | Cell 6 Human    | 0 if Human has claimed, 0 otherwise    |
| 12    | Cell 7 Computer | 0 if Computer has claimed, 0 otherwise |
| 13    | Cell 7 Human    | 0 if Human has claimed, 0 otherwise    |
| 14    | Cell 8 Computer | 0 if Computer has claimed, 0 otherwise |
| 15    | Cell 8 Human    | 0 if Human has claimed, 0 otherwise    |
| 16    | Cell 9 Computer | 0 if Computer has claimed, 0 otherwise |
| 17    | Cell 9 Human    | 0 if Human has claimed, 0 otherwise    |
| 18-31 | Unused          |                                        |

Your job is to implement the places labeled with `TODO` in `bbb.c`, to get all the tests passing in `bbb_tests.c`.

When you have all tests passing you should be able to play the computer by running the main method in `bbb_cli.c`.

Enjoy!