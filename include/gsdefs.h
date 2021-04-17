//
//  gsdefs.h
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#ifndef gsdefs_h
#define gsdefs_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "nndefs.h"
#include "envdefs.h"

#define MAX_LINE_SIZE 512
#define MAX_STR_SIZE 32

#define MAX_NUM_LAYERS 6
#define ENV_WIDTH 20
#define MIN_POP_SIZE 100
#define MIN_GEN_CT 100
#define MIN_MUTATE 0.0
#define MIN_SURVIVE 0.0
#define MAX_MUTATE 1.0
#define MAX_SURVIVE 1.0

#define MAX_NUM_THREADS 100

#define PRINT_BATCH 10
#define REPLAY_TIME_5 30000
#define REPLAY_TIME_4 27500
#define REPLAY_TIME_3 25000
#define REPLAY_TIME_2 22500
#define REPLAY_TIME_1 20000

#define NOT_FOUND -1
#define NOT_SET -5

typedef struct gs_params gs_params;
typedef struct thread_data thread_data;

struct gs_params {
    int pop_size;
    int gen_ct;
    int env_width;
    int num_layers;
    int num_threads;
    int print_replay;
    float mutate;
    float survive;
    int shape[(2 * MAX_NUM_LAYERS)];
    funct activation [MAX_NUM_LAYERS];
};

struct thread_data {
    ann_set *ann_s;
    env_set *env_s;
    int *action_set;
    gs_params *params;
    int *surv_idx;
    double *fitness_prob;
};

// multi-threading variables
int finished_flag;
int model_flag;
int curr_target;
int spawn_setup_flag;
int wait_run_flag;
int wait_compute_flag;
int wait_spawn_flag;
int wait_reset_flag;
int run_flag;
int compute_flag;
int spawn_flag;
int reset_flag;

//gs driver functions
void genetic_snake(const char *);

// gs utils functions
int rand_int(int, int);
double rand_double(int, int);
double rand_norm(void);
int rand_roulette(int, double *);
void compute_set_fitness(ann_set *, env_set *);
void compute_ann_fitness(ann_set *, env_set *, int);
void init_thread_data_struct(thread_data *t_data, gs_params *params);
void init_thread_variables();
gs_params * read_parameters_from_file(const char *);

// gs print functions
void print_model_parameters(gs_params *);
void print_start_prompt();
void print_pop_stats(ann_set *, env_set *);
void print_gen_stats(int, int *, ann_set *, env_set *, int);

// gs thread functions
void * snake_controller_thread(void *);
void * model_controller_thread(void *);

#endif /* gsdefs_h */
