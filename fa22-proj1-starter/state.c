#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"
#define BOARD_ROW (18)
#define BOARD_COL (20)

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Task 1 */
/*
 * Helper function to
 */
/*Helper function to draw a row,'*' on eachend
*/

void drawRow (game_state_t *state, int col, int row) {
    state->board[row] = (char*) malloc(sizeof(char) * (BOARD_COL + 1));
  for (int i = 1; i < col - 1; i++) {
    state -> board[row][i] = ' ';
  }
  state -> board[row][0] = '#';
  state -> board[row][col - 1] = '#';
  state -> board[row][col] = '\0';
}
/*Helper function to draw a column
*/
void drawWall (game_state_t *state, int col, int row) {
    state->board[row] = (char*) malloc(sizeof(char) * (BOARD_COL + 1));
  for (int i = 0; i < col; i++) {
    state -> board[row][i] = '#';
  }
    state -> board[row][col] = '\0';
}
game_state_t* create_default_state() {
  game_state_t* newGame = (game_state_t *)malloc(sizeof(game_state_t));
  newGame->board = (char **) malloc((sizeof(char *)) * BOARD_ROW);
  newGame -> num_rows = BOARD_ROW;
  drawWall(newGame, BOARD_COL, 0);
  drawWall(newGame, BOARD_COL, BOARD_ROW - 1);
  for (int i = 1; i < BOARD_ROW - 1; i++) {
    drawRow(newGame, BOARD_COL, i);
  }
  /* add fruit */
  newGame -> board[2][9] = '*';
  /*add snake*/
  newGame->num_snakes = 1;
  newGame->snakes = (snake_t *) malloc(sizeof(snake_t));
  newGame->snakes->head_col = 4;
  newGame->snakes->head_row = 2;
  newGame->snakes->tail_col = 2;
  newGame->snakes->tail_row = 2;
  newGame->snakes->live = true;
  newGame->board[2][4] = 'D';
  newGame->board[2][2] = 'd';
  newGame->board[2][3] = '>';
  return newGame;
}

/* Task 2 */
void free_state(game_state_t* state) {
    if (state) {
        /*free snakes*/
        free(state->snakes);
        /*free board*/
        for (int i = 0; i < BOARD_ROW; i++) {
            if (state->board[i]) {
                free(state->board[i]);
            }
        }
        if (state->board) {
            free(state->board);
        }
        free(state);
    }
    return;
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
    for (int i = 0; i < BOARD_ROW; i++) {
        for (int j = 0; j < BOARD_COL; j++) {
            fprintf(fp, "%c", state->board[i][j]);
        }

            fprintf(fp, "\n");
    }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  return c == 'w' || c == 'a' || c == 's' || c == 'd';
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {

  return c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x' ;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {

  return is_tail(c) || is_head(c) || c == '^' || c == '<' || c == 'v' || c == '>';
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
    if(c=='^') return 'w';
    if(c=='<') return 'a';
    if(c=='v') return 's';
    if(c=='>') return 'd';
    return '?'; // if happens, means somthing wrong
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
    if(c=='W') return '^';
    if(c=='A') return '<';
    if(c=='S') return 'v';
    if(c=='D') return '>';
    return '?';
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
    if(c=='v'||c=='s'||c=='S') return cur_row + 1;
    if(c=='^'||c=='w'||c=='W') return cur_row - 1;
    return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
    if(c=='>'||c=='d'||c=='D') return cur_col + 1;
    if(c=='<'||c=='a'||c=='A') return cur_col - 1;
    return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  return '?';
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  // TODO: Implement this function.
  return;
}

/* Task 5 */
game_state_t* load_board(char* filename) {
  // TODO: Implement this function.
  return NULL;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  // TODO: Implement this function.
  return NULL;
}
