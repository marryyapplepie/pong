#include <ncurses.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

#define DELAY 50000

class Paddle {
public:
    int y, x;
    int height;
    
    Paddle(int start_y, int start_x, int h) : y(start_y), x(start_x), height(h) {}
    
    void move_up(int limit) {
        if (y > limit) y--;
    }
    
    void move_down(int limit) {
        if (y < limit - height) y++;
    }
    
    void draw() const {
        for (int p = 0; p < height; ++p) {
            mvprintw(y + p, x, "|");
        }
    }
};

class Ball {
public:
    int x, y;
    int direction_x, direction_y;
    
    Ball(int start_x, int start_y) : x(start_x), y(start_y), direction_x(1), direction_y(1) {}
    
    void move() {
        x += direction_x;
        y += direction_y;
    }
    
    void draw() const {
        mvprintw(y, x, "o");
    }
    
    bool check_collisions(int max_x, int max_y, const Paddle& left, const Paddle& right, int &score_left, int &score_right) {
        if (y <= 0 || y >= max_y - 1) {
            direction_y = -direction_y;
        }
        
        if (x == 1) {
            if (y >= left.y && y < left.y + left.height) {
                direction_x = -direction_x;
            } else {
                score_right++;
                return true; // Ball missed the paddle
            } 
        } else if (x == max_x - 2) {
            if (y >= right.y && y < right.y + right.height) {
                direction_x = -direction_x;
            } else {
                score_left++;
                return true; // Ball missed the paddle
            }
        }
        
        return false;
    }
};

void draw_score(int score_left, int score_right, int max_x) {
    mvprintw(0, max_x/2 - 10, "Player 1: %d", score_left);
    mvprintw(0, max_x/2 + 5, "Player 2: %d", score_right);
}

bool display_winner(int player) {
    clear();
    mvprintw(10, 10, "Player %d wins! \n"
                    "         Press SPACE to start again\n"
                    "         Press q to quit the game", player);

    mvprintw(14, 10,
        "/\\_/\\\n"
        "         ( o.o )\n"
        "          > ^ <\n");
  
    refresh();
    
    // Wait for 'q' to quit or ' ' (space) to start anew
    int ch;
    while ((ch = getch()) != 'q') {
        if (ch == ' ') return true; // Restart game
    }
    return false; // Exit game
}

int main() {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    start:
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    Paddle left_paddle(10, 1, 4);
    Paddle right_paddle(10, max_x - 2, 4);
    Ball ball(max_x / 2, max_y / 2);

    int score_left = 0, score_right = 0;

    timeout(1); // Non-blocking input

    while (true) {
        clear();

        ball.draw();
        left_paddle.draw();
        right_paddle.draw();
        draw_score(score_left, score_right, max_x);

        if (ball.check_collisions(max_x, max_y, left_paddle, right_paddle, score_left, score_right)) {
            ball = Ball(max_x / 2, max_y / 2); // Reset ball at the center
        }
        
        ball.move();

        if (score_left == 2 || score_right == 2) {
            if (!display_winner(score_left == 2 ? 1 : 2)) {
                endwin();
                return 0;
            } else {
                goto start; // Restart game
            }
        }

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                right_paddle.move_up(0);
                break;
            case KEY_DOWN:
                right_paddle.move_down(max_y);
                break;
            case 'w':
            case 'W':
                left_paddle.move_up(0);
                break;
            case 's':
            case 'S':
                left_paddle.move_down(max_y);
                break;
            case 'q':
            case 'Q':
            endwin();
                return 0;
            default:
                break;
        }

        refresh();
        usleep(DELAY);
    }
}
