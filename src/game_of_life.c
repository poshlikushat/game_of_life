#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>

#define W 80
#define H 25
#define MIN_D 10000
#define MAX_D 300000
#define STEP_D 30000

void print_usage(const char* prog_name) {
    fprintf(stderr, "Usage: %s < <state_file>\n", prog_name);
    fprintf(stderr, "The initial %dx%d field (0/1 per cell) must be piped in via stdin.\n", W, H);
    fprintf(stderr, "Example: %s < ../states/state3.txt\n", prog_name);
}

int read_field(int f[H][W]) {
    int res = 1;
    for (int y = 0; y < H && res; y++) {
        for (int x = 0; x < W && res; x++) {
            if (scanf("%1d", &f[y][x]) != 1) {
                res = 0;
            }
        }
    }
    return res;
}

int count_neighbors(int f[H][W], int y, int x) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy || dx) {
                count += f[(y + dy + H) % H][(x + dx + W) % W];
            }
        }
    }
    return count;
}

void cell_logic(int curr[H][W], int next[H][W], int y, int x, int* b_tk, long long* b_tot, long long* d_tot) {
    int n = count_neighbors(curr, y, x);
    if (curr[y][x]) {
        next[y][x] = (n == 2 || n == 3);
        if (!next[y][x]) {
            (*d_tot)++;
        }
    } else {
        next[y][x] = (n == 3);
        if (next[y][x]) {
            (*b_tot)++;
            (*b_tk)++;
        }
    }
}

void update_field(int curr[H][W], int next[H][W], long long* b_tot, long long* d_tot, int* b_tk) {
    *b_tk = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            cell_logic(curr, next, y, x, b_tk, b_tot, d_tot);
        }
    }
}

void copy_field(int src[H][W], int dest[H][W]) {
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            dest[y][x] = src[y][x];
        }
    }
}

int check_stable(const int f1[H][W], const int f2[H][W]) {
    int stable = 1;
    for (int y = 0; y < H && stable; y++) {
        for (int x = 0; x < W && stable; x++) {
            if (f1[y][x] != f2[y][x]) {
                stable = 0;
            }
        }
    }
    return stable;
}

void render_grid(int f[H][W]) {
    mvaddch(0, 0, ACS_ULCORNER);
    for (int x = 1; x <= W; x++) mvaddch(0, x, ACS_HLINE);
    mvaddch(0, W + 1, ACS_URCORNER);
    for (int y = 1; y <= H; y++) {
        mvaddch(y, 0, ACS_VLINE);
        mvaddch(y, W + 1, ACS_VLINE);
        for (int x = 1; x <= W; x++) {
            move(y, x);
            addch(f[y - 1][x - 1] ? '@' : ' ');
        }
    }
    mvaddch(H + 1, 0, ACS_LLCORNER);
    for (int x = 1; x <= W; x++) mvaddch(H + 1, x, ACS_HLINE);
    mvaddch(H + 1, W + 1, ACS_LRCORNER);
}

void render_stats(long long b_tot, long long d_tot, int b_tk, long long ticks, int show, int status) {
    if (show) {
        mvprintw(H + 3, 0, "Ticks: %lld | Born: %lld | Died: %lld | Birth Rate: %d/tick", ticks, b_tot, d_tot,
                 b_tk);
        if (status == 1) {
            mvprintw(H + 4, 0, "Status: static objects formed ");
        } else if (status == 2) {
            mvprintw(H + 4, 0, "Status: infinite loop ");
        } else {
            mvprintw(H + 4, 0, " ");
        }
    }
}

void process_input(int ch, int* delay, int* show_stats, int* run) {
    if (ch == 'a' || ch == 'A') {
        *delay = (*delay > MIN_D) ? *delay - STEP_D : MIN_D;
    } else if (ch == 'z' || ch == 'Z') {
        *delay = (*delay < MAX_D) ? *delay + STEP_D : MAX_D;
    } else if (ch == 's' || ch == 'S') {
        *show_stats = !(*show_stats);
    } else if (ch == ' ') {
        *run = 0;
    }
}

int is_alive(const int curr[H][W]) {
    int alive = 0;
    for (int y = 0; y < H && !alive; y++) {
        for (int x = 0; x < W; x++) {
            if (curr[y][x]) {
                alive = 1;
            }
        }
    }
    return alive;
}

void check_game_status(int curr[H][W], int next[H][W], int p1[H][W], int p2[H][W], int p3[H][W],
                       int* status) {
    if (check_stable(curr, next)) {
        *status = 1;
    } else if (check_stable(next, p1) || check_stable(next, p2) || check_stable(next, p3)) {
        *status = 2;
    } else {
        *status = 0;
    }
}

void run_tick(int curr[H][W], int next[H][W], int p1[H][W], int p2[H][W], int p3[H][W], long long* born_total,
              long long* died_total, int* born_tick, long long* ticks, int* status, int* over) {
    update_field(curr, next, born_total, died_total, born_tick);
    (*ticks)++;
    check_game_status(curr, next, p1, p2, p3, status);
    copy_field(p2, p3);
    copy_field(p1, p2);
    copy_field(curr, p1);
    copy_field(next, curr);
    if (!is_alive(curr)) {
        *over = 1;
    }
}

void loop(int curr[H][W], int next[H][W], int p1[H][W], int p2[H][W], int p3[H][W]) {
    int delay = 100000, show_stats = 0, run = 1, status = 0, over = 0, b_tk = 0;
    long long born_total = 0, died_total = 0, ticks = 0;
    copy_field(curr, p1);
    copy_field(curr, p2);
    copy_field(curr, p3);
    while (run) {
        process_input(getch(), &delay, &show_stats, &run);
        if (!over) {
            run_tick(curr, next, p1, p2, p3, &born_total, &died_total, &b_tk, &ticks, &status, &over);
        } else {
            status = 0;
            b_tk = 0;
        }
        erase();
        render_grid(curr);
        render_stats(born_total, died_total, b_tk, ticks, show_stats, status);
        refresh();
        napms(delay / 1000);
    }
}

int main(int argc, char* argv[]) {
    int curr[H][W], next[H][W], p1[H][W], p2[H][W], p3[H][W];
    int status = 0;
    if (isatty(STDIN_FILENO)) {
        print_usage(argc > 0 ? argv[0] : "./game_of_life");
        status = 1;
    } else if (!read_field(curr)) {
        status = 1;
    } else if (freopen("/dev/tty", "r", stdin) == NULL) {
        status = 1;
    } else {
        initscr();
        cbreak();
        noecho();
        nodelay(stdscr, TRUE);
        curs_set(0);
        loop(curr, next, p1, p2, p3);
        endwin();
    }
    return status;
}
