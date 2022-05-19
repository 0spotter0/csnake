#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

int board_size = 16;
int delay_ms = 150;
bool game_over;
bool game_win;

typedef enum Slot {EMPTY_SLOT,
    PILL_SLOT,
    SNAKE_HEAD_SLOT,
    SNAKE_VLINE_SLOT,
    SNAKE_HLINE_SLOT,
    SNAKE_ULCORNER_SLOT,
    SNAKE_URCORNER_SLOT,
    SNAKE_LLCORNER_SLOT,
    SNAKE_LRCORNER_SLOT,
    SNAKE_TAIL_SLOT
} Slot;

typedef struct CoordTuple {
    int row;
    int col;
} CoordTuple;

typedef struct Snake {
    char direction;
    int length;
    CoordTuple *segments;
} Snake;

bool SnakeCollides(Snake *snake, CoordTuple *target) {
    for(int i = 0; i < snake->length; i++) {
        if(snake->segments[i].row == target->row && snake->segments[i].col == target->col) {
            return true;
        }
    }
    return false;
}

void UpdatePill(CoordTuple *pill, Snake *snake) {
    do {
        pill->row = (rand() % (board_size - 1)) + 1;
        pill->col = (rand() % (board_size - 1)) + 1;
    } while(SnakeCollides(snake, pill));
}

void UpdateSnake(Snake *snake, CoordTuple *pill) {
    CoordTuple newHeadPos;
    newHeadPos.row = snake->segments[snake->length - 1].row;
    newHeadPos.col = snake->segments[snake->length - 1].col;

    switch(snake->direction) {
        case 'n':
            newHeadPos.row--;
            break;
        case 's':
            newHeadPos.row++;
            break;
        case 'e':
            newHeadPos.col++;
            break;
        case 'w':
            newHeadPos.col--;
            break;
    }

    if(snake->length == (board_size-1) * (board_size-1)) {
        game_win = true;
    } else if(newHeadPos.row <= 0 || newHeadPos.row >= board_size || newHeadPos.col <= 0 || newHeadPos.col >= board_size) {
        game_over = true;
    } else if(snake->length > 1 && SnakeCollides(snake, &newHeadPos)) {
        game_over = true;
    } else {
        if(newHeadPos.row == pill->row && newHeadPos.col == pill->col) {
            UpdatePill(pill, snake);
            snake->segments[snake->length++] = newHeadPos;
        }
        else {
            for(int i = 0; i < snake->length-1; i++) {
                snake->segments[i] = snake->segments[i+1];
            }
            snake->segments[snake->length-1] = newHeadPos;
        }
    }
}

void ClearBoard(int board[board_size][board_size]) {
    for(int r = 0; r < board_size; r++) {
        for(int c = 0; c < board_size; c++) {
            board[r][c] = EMPTY_SLOT;
        }
    }
}

void UpdateBoard(int board[board_size][board_size], Snake *snake, CoordTuple *pill) {
    ClearBoard(board);
    for(int i = 0; i < snake->length; i++) {

        int SLOT = EMPTY_SLOT;

        //TODO: back item case
        
        // if at snake->length - 1 => HEAD
        if(i == snake->length-1) {
            SLOT = SNAKE_HEAD_SLOT;
        } else if (i == 0) {

            // if next piece is same row:
                // HLINE

            // if next piece is same col:
                // VLINE

            CoordTuple cur = snake->segments[i];
            CoordTuple next = snake->segments[i+1];

            if(cur.row == next.row) {
                SLOT = SNAKE_HLINE_SLOT;
            } else if(cur.col == next.col) {
                SLOT = SNAKE_VLINE_SLOT;
            }

            SLOT = SNAKE_TAIL_SLOT;

        } else {
            CoordTuple cur = snake->segments[i];
            CoordTuple next = snake->segments[i+1];
            CoordTuple prev = snake->segments[i-1];

            if(prev.row == cur.row) {
                if(next.row == cur.row) {
                    SLOT = SNAKE_HLINE_SLOT;
                } else if(prev.col < cur.col) {
                    if(next.row < cur.row) {
                        SLOT = SNAKE_LRCORNER_SLOT;
                    } else if(next.row > cur.row) {
                        SLOT = SNAKE_URCORNER_SLOT;
                    }
                } else if(prev.col > cur.col) {
                    if(next.row < cur.row) {
                        SLOT = SNAKE_LLCORNER_SLOT;
                    } else if(next.row > cur.row) {
                        SLOT = SNAKE_ULCORNER_SLOT;
                    }
                }
            } else if(prev.col == cur.col) {
                if(next.col == cur.col) {
                    SLOT = SNAKE_VLINE_SLOT;
                } else if(prev.row < cur.row) {
                    if(next.col > cur.col) {
                        SLOT = SNAKE_LLCORNER_SLOT;
                    } else if(next.col < cur.col) {
                        SLOT = SNAKE_LRCORNER_SLOT;
                    }
                } else if(prev.row > cur.row) {
                    if(next.col > cur.col) {
                        SLOT = SNAKE_ULCORNER_SLOT;
                    } else if(next.col < cur.col) {
                        SLOT = SNAKE_URCORNER_SLOT;
                    }
                }
            }
        }

        board[snake->segments[i].row][snake->segments[i].col] = SLOT;

    }
    board[pill->row][pill->col] = PILL_SLOT;
}

void DrawBoard(WINDOW *win, int board[board_size][board_size]) {
    for(int r = 0; r < board_size; r++) {
        for(int c = 0; c < board_size; c++) {
            switch(board[r][c]) {
                case EMPTY_SLOT:
                    waddch(win, ' ');
                    waddch(win, ' ');
                    break;
                case PILL_SLOT:
                    waddch(win, '*');
                    waddch(win, ' ');
                    break;
                case SNAKE_HEAD_SLOT:
                    waddch(win, '%');
                    waddch(win, ' ');
                    break;
                case SNAKE_VLINE_SLOT:
                    waddch(win, ACS_VLINE);
                    waddch(win, ' ');
                    break;
                case SNAKE_HLINE_SLOT:
                    waddch(win, ACS_HLINE);
                    waddch(win, ACS_HLINE);
                    break;
                case SNAKE_LLCORNER_SLOT:
                    waddch(win, ACS_LLCORNER);
                    waddch(win, ACS_HLINE);
                    break;
                case SNAKE_LRCORNER_SLOT:
                    waddch(win, ACS_LRCORNER);
                    waddch(win, ' ');
                    break;
                case SNAKE_ULCORNER_SLOT:
                    waddch(win, ACS_ULCORNER);
                    waddch(win, ACS_HLINE);
                    break;
                case SNAKE_URCORNER_SLOT:
                    waddch(win, ACS_URCORNER);
                    waddch(win, ' ');
                    break;
                case SNAKE_TAIL_SLOT:
                    waddch(win, ACS_BULLET);
                    waddch(win, ' ');
                    break;
            }
        }
        wprintw(win, "\n");
    }
}

void HandleResize(int s) {}

Snake *CreateSnake(int segLength) {
    Snake *newSnake = malloc(sizeof(Snake));

    newSnake->direction = '0'; //FIXME:
    newSnake->length = 1;
    newSnake->segments = malloc(sizeof(CoordTuple) * segLength);

    CoordTuple tempCoord;
    tempCoord.row = board_size / 2;
    tempCoord.col = board_size / 2;

    newSnake->segments[0] = tempCoord;

    return newSnake;
}

int main() { 
    game_over = false;
    game_win = false;

    Snake *snake = CreateSnake(board_size * board_size);
    CoordTuple *pill = malloc(sizeof(int) * 2);

    srand(time(NULL));
    
    initscr();

    timeout(delay_ms);


    signal(SIGWINCH, HandleResize);

    int tw = getmaxx(stdscr);
    int th = getmaxy(stdscr);

    WINDOW *boardWindow = newwin(board_size+1, (board_size*2)+1, 0, 0);    //(tw/2) - (board_size/2), (th/2) - (board_size/2)
    WINDOW *infoWindow = newwin(board_size+1, 50, 0, (board_size*2)+1);    //(tw/2) - (board_size/2), (th/2) - (board_size/2)

    int board[board_size][board_size];

    UpdatePill(pill, snake);
    
    for(int r = 0; r < board_size; r++) {
        for(int c = 0; c < board_size; c++) {
            board[r][c] = EMPTY_SLOT;
        }
    }

    while(true) {
        clear();
        refresh();

        wclear(boardWindow);
        wclear(infoWindow);

        DrawBoard(boardWindow, board);

        if(game_win || game_over) {
            wprintw(infoWindow, "\n  %s\n", (game_win) ? "YOU WIN!" : "GAME OVER");
            wprintw(infoWindow, "  Final length: %d\n", snake->length);
            wprintw(infoWindow, "  Press 'q' to quit or 'r' to restart.", snake->length);
        } else {
            wprintw(infoWindow, "\n length: %d\n headpos: (%d,%d)\n pill: (%d,%d)\n", snake->length, snake->segments[snake->length - 1].row, snake->segments[snake->length - 1].col, pill->row, pill->col);
        }

        box(boardWindow, ACS_VLINE, ACS_HLINE);
        box(infoWindow, ACS_VLINE, ACS_HLINE);
        wrefresh(boardWindow);
        wrefresh(infoWindow);

        int keyPressed = getch();
        switch(keyPressed) {
            case 'w':
                if(snake->length == 1 || snake->direction != 's') snake->direction = 'n';
                break;
            case 'a':
                if(snake->length == 1 || snake->direction != 'e') snake->direction = 'w';
                break;
            case 's':
                if(snake->length == 1 || snake->direction != 'n') snake->direction = 's';
                break;
            case 'd':
                if(snake->length == 1 || snake->direction != 'w') snake->direction = 'e';
                break;
            case 'q':
                return 0;
                break;
            case 'r':
                //TODO: implement reset
                break;
        }
        if(!game_over) {
            UpdateSnake(snake, pill);
            UpdateBoard(board, snake, pill);
        }
    }

    endwin();

    return 0;
}