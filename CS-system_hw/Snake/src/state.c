#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

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
game_state_t* create_default_state() {
  // TODO: Implement this function.
  game_state_t* state = (game_state_t*)malloc(sizeof(game_state_t));
  state->num_rows = 18;
  state->board = malloc(sizeof(char*)*18);
  state->num_snakes = 1;
  //创建初始board
  for(int i = 0;i < 18;i++){
    (state->board)[i] = malloc(sizeof(char)*21);
    (state->board)[i][0] = (state->board)[i][19] = '#';
    for(int j = 1;j < 19;j++){
      (state->board)[i][j] = (i == 0 || i == 17)?'#':' ';
    }
    (state->board)[i][20] = '\0';
  }
  (state->board)[2][2] = 'd';
  (state->board)[2][3] = '>';
  (state->board)[2][4] = 'D';
  (state->board)[2][9] = '*';
  
  //创建初始snake
  state->snakes = (snake_t*)malloc(sizeof(snake_t));
  state->snakes->tail_row = 2;
  state->snakes->tail_col = 2;
  state->snakes->head_row = 2;
  state->snakes->head_col = 4;
  state->snakes->live = true;
  if(state != NULL){
    return state;
  }
  return NULL;
}

/* Task 2 */
void free_state(game_state_t* state) {
  // TODO: Implement this function.
  if(state==NULL) return;
  if(state->snakes!=NULL)free(state->snakes);
  if(state->board==NULL)
  {
    for (int i = 0; i < 18; i++) {
      if(state->board[i]!=NULL)free(state->board[i]);
    }
    free(state->board);
  }
  free(state);

  return;
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
  // TODO: Implement this function.
  for(int i = 0;i < state->num_rows;i++){
    fprintf(fp,"%s\n",state->board[i]);
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
  // TODO: Implement this function.
  if(c == 'w' || c == 's' || c == 'a' || c == 'd') return true;
  return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  if(c == 'W' || c == 'S' || c == 'A' || c == 'D') return true;
  return false;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  return is_tail(c) || is_head(c) || c == '^' || c == '<' || c == 'v' || c == '>';
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  if(c=='^') return 'w';
  if(c=='<') return 'a';
  if(c=='v') return 's';
  if(c=='>') return 'd';
  return '?';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
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
  // TODO: Implement this function.
  if(c=='v'||c=='s'||c=='S') return cur_row+1;
  if(c=='^'||c=='w'||c=='W') return cur_row-1;
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  if(c=='>'||c=='d'||c=='D') return cur_col+1;
  if(c=='<'||c=='a'||c=='A') return cur_col-1;
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  char c = get_board_at(state,state->snakes[snum].head_row,state->snakes[snum].head_col);
  return get_board_at(state,get_next_row(state->snakes[snum].head_row,c),get_next_col(state->snakes[snum].head_col,c));
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
  char c=get_board_at(state,state->snakes[snum].head_row,state->snakes[snum].head_col);
  set_board_at(state,state->snakes[snum].head_row,state->snakes[snum].head_col,head_to_body(c));
  state->snakes[snum].head_row = get_next_row(state->snakes[snum].head_row,c);
  state->snakes[snum].head_col = get_next_col(state->snakes[snum].head_col,c);
  set_board_at(state,state->snakes[snum].head_row,state->snakes[snum].head_col,c);
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
  char c = get_board_at(state,state->snakes[snum].tail_row,state->snakes[snum].tail_col);
  set_board_at(state,state->snakes[snum].tail_row,state->snakes[snum].tail_col,' ');
  state->snakes[snum].tail_row = get_next_row(state->snakes[snum].tail_row,c);
  state->snakes[snum].tail_col = get_next_col(state->snakes[snum].tail_col,c);
  set_board_at(state,state->snakes[snum].tail_row,state->snakes[snum].tail_col,body_to_tail(get_board_at(state,state->snakes[snum].tail_row,state->snakes[snum].tail_col)));
  return;
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  // TODO: Implement this function.
  for(int i = 0;i < state->num_snakes;i++){
    if(!state->snakes[i].live) continue;
    char c = next_square(state,i);
    if(c == ' '){
      update_head(state,i);
      update_tail(state,i);
    }
    else if(c =='*'){
      update_head(state,i);
      add_food(state);
    }
    else{
      state->snakes[i].live=false;
      set_board_at(state,state->snakes[i].head_row,state->snakes[i].head_col,'x');
    }
  }
  return;
}

/* Task 5 */
game_state_t* load_board(FILE* fp) {
  // TODO: Implement this function.
  if(fp==NULL) return NULL;
  game_state_t* state=malloc(sizeof(game_state_t));
  state->num_rows=0;
  state->board=NULL;
  state->num_snakes=0;
  state->snakes=NULL;
  unsigned int row = 0;
  unsigned int col = 0;
  char s;
  bool flag = false;
  while(fscanf(fp,"%c",&s)!=EOF){
    if(flag){
      state->board = realloc(state->board,(row + 1)*(sizeof(char*)));
      state->board[row] = malloc(sizeof(char));
    }
    if(state->board == NULL){
      state->board = malloc(sizeof(char*));
      state->board[0] = malloc(sizeof(char));
    }
    state->board[row] = realloc(state->board[row],(col + 1)*(sizeof(char)));
    if(s == '\n'){
      state->board[row][col] = '\0';
      row++;
      col = 0;
      flag = true;
    }else{
      state->board[row][col] = s;
      col++;
      flag = false;
    }
  }
  state->num_rows = row;
  if(state == NULL) return NULL;
  return state;
  
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
  int row=state->snakes[snum].tail_row;
  int col=state->snakes[snum].tail_col;
  char c=get_board_at(state,row,col);
  while(true){
    if(c=='w') row--;
    if(c=='a') col--;
    if(c=='s') row++;
    if(c=='d') col++;
    c=get_board_at(state,row,col);
    if(is_head(c)) break;
    c=body_to_tail(c);
  }
  state->snakes[snum].head_row=row;
  state->snakes[snum].head_col=col;
  return;
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  // TODO: Implement this function.
  state->num_snakes=0;
  for(int i=0;i<state->num_rows;i++){
    for(int j=strlen(state->board[i])-1;j>=0;j--){
      if(is_tail(state->board[i][j])) state->num_snakes++;
    }
  }
  state->snakes=malloc(sizeof(snake_t)*state->num_snakes);
  int snum=0;
  for(int i=0;i<state->num_rows;i++){
    int len=strlen(state->board[i]);
    for(int j=0;j<len;j++){
      if(is_tail(state->board[i][j])){
        state->snakes[snum].tail_row=i;
        state->snakes[snum].tail_col=j;
        find_head(state,snum);
        state->snakes[snum].live=true;
        snum++;
      }
    }
  }
  return state;
}
