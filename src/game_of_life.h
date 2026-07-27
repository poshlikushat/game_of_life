#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#define W 80
#define H 25
#define MIN_D 10000
#define MAX_D 300000
#define STEP_D 30000

int read_field(int f[H][W]);
int count_neighbors(int f[H][W], int y, int x);
void cell_logic(int curr[H][W], int next[H][W], int y, int x, int* b_tk, long long* b_tot, long long* d_tot);
void update_field(int curr[H][W], int next[H][W], long long* b_tot, long long* d_tot, int* b_tk);
void copy_field(int src[H][W], int dest[H][W]);
int check_stable(const int f1[H][W], const int f2[H][W]);
void render_grid(int f[H][W]);
void render_stats(long long b_tot, long long d_tot, int b_tk, long long ticks, int show, int status);
void process_input(int ch, int* delay, int* show_stats, int* run);
int is_alive(const int curr[H][W]);
void check_game_status(int curr[H][W], int next[H][W], int p1[H][W], int p2[H][W], int p3[H][W], int* status);
void run_tick(int curr[H][W], int next[H][W], int p1[H][W], int p2[H][W], int p3[H][W], long long* born_total,
              long long* died_total, int* born_tick, long long* ticks, int* status, int* over);
void loop(int curr[H][W], int next[H][W], int p1[H][W], int p2[H][W], int p3[H][W]);

#endif
