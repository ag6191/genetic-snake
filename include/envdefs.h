//
//  envdefs.h
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#ifndef snake_h
#define snake_h

#define MIN_SNAKE_LEN 3
#define MAX_MOVES_PER_APPLE 150

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

typedef struct snake_node snake_node;
typedef struct apple apple;
typedef struct env env;
typedef struct apple_data apple_data;
typedef struct move_data move_data;
typedef struct dist_data dist_data;
typedef struct env_set env_set;

struct snake_node {
    int x;
    int y;
    snake_node *p;
    snake_node *c;
};

struct apple_data {
    int x;
    int y;
    apple_data *next_apple;
};

struct move_data {
    int action;
    int apple_eaten;
    move_data *next_move;
    move_data *prev_move;
};

struct apple {
    int x;
    int y;
};

struct env {
    apple *a;
    snake_node *h;
    int m;
    int m_n;
    int n;
    int len;
    int env_dim;
    int alive;
    apple_data *a_data;
    move_data *m_data;
};

struct dist_data {
    double wall[8];
    double body[8];
    double apple[8];
};

struct env_set {
    int num_env;
    int is_active;
    env *data;
    dist_data *dist_d;
};

// env functions
void init_apple(int, env *);
void init_snake(int, int, env *);
void free_snake_chain(snake_node *);
void destroy_env(env *);
void reset_env(env *);
void init_env(int, env *);
void copy_env_data(env *, env *);
int run_env_action(int a, env *src);

// env controller functions
void free_env_set(env_set *);
void update_dist_data(env_set *, int);
void init_env_set(env_set *, int, int);
void reset_env_set(env_set *);
void run_env_set(env_set *, int *);

#endif /* snake_h */