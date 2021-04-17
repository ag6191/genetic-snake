//
//  gsutils.c
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#include <stdlib.h>
#include <stdio.h>
#include "gsdefs.h"


/*
 * rand_int - Returns a random int between two specified values
 */
int rand_int(int low, int high)
{
    return (rand()% (high - low)) + low;
}


/*
 * rand_double - Returns a random double between two specified values
 */
double rand_double(int low, int high)
{
    return ((double) ((rand()% (high - low)) + (double) low));
}


/*
 * rand_norm - Returns a random number in a normal distribution
 */
double rand_norm()
{
    double U1, U2, W;
    do {
        U1 = -1 + ((double) rand () / RAND_MAX) * 2;
        U2 = -1 + ((double) rand () / RAND_MAX) * 2;
        W = pow (U1, 2) + pow (U2, 2);
    } while (W >= 1 || W == 0);
    return ((sqrt ((-2 * log (W)) / W)) * U1);
}


/*
 * rand_roulette - Returns a random index using roulette selection
 */
int rand_roulette(int ct_prob, double *prob)
{
    int i;
    double offset = 0;
    double r = rand() / (RAND_MAX + 1.0);
    for (i = 0; i < ct_prob; i++) {
        offset += prob[i];
        if (r < offset) break;
    }
    return i;
}


/*
 * compute_ann_fitness - Computes and updates the fitness of a specified ann with its respective env struct
 */
void compute_ann_fitness(ann_set *ann_s, env_set *env_s, int i)
{
    // Fitness(m,n) = num_moves * 2 ^ (num_apples)
    ann_s->fitness[i] = ((double) env_s->data[i].m) * pow(2.0, (double) env_s->data[i].n);
    return;
}


/*
 * compute_set_fitness - Computes and updates the fitness of each ann with their respective env structs
 */
void compute_set_fitness(ann_set *ann_s, env_set *env_s)
{
    for (int i = 0; i < ann_s->num_net; i++) {
        compute_ann_fitness(ann_s, env_s, i);
    }
    return;
}


/*
 * init_thread_data_struct - Initializes a thread data struct with a given parameter struct
 */ 
void init_thread_data_struct(thread_data *t_data, gs_params *params)
{
    int ct_surv = params->pop_size * params->survive;
    t_data->action_set = (int *) malloc(params->pop_size * sizeof(int));
    t_data->env_s = (env_set *) malloc(sizeof(env_set));
    t_data->ann_s = (ann_set *) malloc(sizeof(ann_set));
    t_data->params = params;
    t_data->surv_idx = (int *) malloc(params->pop_size * sizeof(int));
    t_data->fitness_prob = (double *) malloc(ct_surv * sizeof(double));
    return;
}


/*
 * define_thread_variables - Defines the initial values for the used thread variables
 */
void init_thread_variables()
{
    finished_flag = 0;
    model_flag = 0;
    curr_target = 0;
    spawn_setup_flag = 0;

    wait_run_flag = 0;
    wait_compute_flag = 0;
    wait_spawn_flag = 0;
    wait_reset_flag = 0;

    run_flag = 0;
    compute_flag = 0;
    spawn_flag = 0;
    reset_flag = 0;
    return;
}


/*
 * init_parameters_struct - Constructor for gs_params struct with flag/default values
 */
static gs_params * init_parameters_struct()
{
    gs_params *params = (gs_params *) malloc(sizeof(gs_params));
    params->pop_size = NOT_SET;
    params->gen_ct = NOT_SET;
    params->env_width = ENV_WIDTH;
    params->num_layers = 0;
    params->num_threads = 1;
    params->print_replay = 0;
    params->mutate = (float) NOT_SET;
    params->survive = (float) NOT_SET;
    return params;
}


/*
 * verify_parameters - Verify that all parameters have been properly set
 */
static void verify_parameters(gs_params *params)
{
    // check that all parameters have been set
    if (params->pop_size == NOT_SET) { printf("\n\nERR: Did not set POP_WIDTH parameter\n"); exit(127); }
    if (params->gen_ct == NOT_SET) { printf("\n\nERR: Did not set GEN_COUNT parameter\n"); exit(127); }
    if (params->mutate == (float) NOT_SET) { printf("\n\nERR: Did not set MUTATE parameter\n"); exit(127); }
    if (params->survive == (float) NOT_SET) { printf("\n\nERR: Did not set SURVIVE parameter\n"); exit(127); }
    if (params->num_layers == 0) { printf("\n\nERR: Did not set LAYER parameter(s)\n"); exit(127); }
    
    // verify that all parameter values are within specified limits
    if (params->pop_size < MIN_POP_SIZE) {
        printf("\n\nERR: Invalid population width (needs to be >= %d)\n", (int) MIN_POP_SIZE); 
        exit(127);
    }

    if (params->gen_ct < MIN_GEN_CT) {
        printf("\n\nERR: Invalid generation count (needs to be >= %d)\n", (int) MIN_GEN_CT);
        exit(127);
    }

    if (params->mutate < MIN_MUTATE) {
        printf("\n\nERR: Invalid mutate percentage (needs to be >= %3f)\n", (float) MIN_MUTATE);
        exit(127);
    } else if (params->mutate > MAX_MUTATE) {
        printf("\n\nERR: Invalid mutate percentage (needs to be <= %3f)\n", (float) MAX_MUTATE);
        exit(127);
    }

    if (params->survive < MIN_SURVIVE) {
        printf("\n\nERR: Invalid survive percentage (needs to be >= %3f)\n\n\n", (float) MIN_SURVIVE);
        exit(127);
    } else if (params->survive > MAX_SURVIVE) {
        printf("\n\nERR: Invalid survive percentage (needs to be <= %3f)\n\n\n", (float) MAX_SURVIVE);
        exit(127);
    }

    if (params->num_layers > MAX_NUM_LAYERS) {
        printf("\n\nERR: Too many layers  (max number of layers is %d)\n\n\n", (int) MAX_NUM_LAYERS);
        exit(127);
    }

    if (params->num_threads > MAX_NUM_THREADS) {
        printf("\n\nERR: Too many threads (max number of threads is %d)\n\n\n", (int) MAX_NUM_THREADS);
        exit(127);
    } else if (params->num_threads < 1) {
        printf("\n\nERR: Too few threads (min number of threads is 1)\n\n\n");
        exit(127);
    }

    if (params->print_replay < 0) {
        printf("\n\nERR: Replay parameter must be 0 or greater (currently set to %d)\n\n\n", params->print_replay);
        exit(127);
    }

    // check that ANN shape is valid between layers
    for (int i = 1; i < params->num_layers; i++) {
        if (params->shape[RIDX(i, 0, 2)] != params->shape[RIDX((i - 1), 1, 2)]) {
            printf("\n\nERR: Invalid ANN shape -- incompatable number of neurons/inputs between LAYER %d and LAYER %d \n", i, i + 1);
            exit(127);
        }
    }

    // force input and output to be 24 and 4 neurons
    params->shape[RIDX(0, 0, 2)] = 24;
    if (params->num_layers > 1) { // multi-layer net
        params->shape[RIDX(2 * params->num_layers - 1, 1, 2)] = 4;
    } else { // 1 layer net
        params->shape[RIDX(0, 1, 2)] = 4;
    }

    return;
}


/*
 * read_parameters_from_file - Updates given gs_params struct with values found in file from argv[1]
 */ 
gs_params * read_parameters_from_file(const char *file_name)
{
    char line [MAX_LINE_SIZE];
    char param [MAX_STR_SIZE]; 
    char activation [MAX_STR_SIZE];
    int line_num = 1;

    // initialize a new parameter struct
    gs_params * params = init_parameters_struct();

    // open the parameter file
    FILE *file = fopen (file_name, "r" );
    if (file == NULL) {
        perror(file_name);
        exit(127);
    }

    // read through the file by line and populate the parameter struct with values
    while (fgets(line, sizeof(line), file) != NULL ) {
        // read parameter flag
        sscanf(line, "%s", param);

        // skip blank or commented lines
        if ((line[0] == '\n') || (strcmp(param, "//") == 0)) { line_num++; continue; }
        
        // identify parameter flag and set parameter variable(s) from line
        if (strcmp(param, "POP_WIDTH") == 0) { // population size flag
            sscanf(line, "%s %d\n", param, &params->pop_size);
        } else if (strcmp(param, "GEN_COUNT") == 0) { // number of generations flag
            sscanf(line, "%s %d\n", param, &params->gen_ct);
        } else if (strcmp(param, "MUTATE") == 0) { // mutation chance flag
            sscanf(line, "%s %f\n", param, &params->mutate);
        } else if (strcmp(param, "SURVIVE") == 0) { // survival chance flag
            sscanf(line, "%s %f\n", param, &params->survive);
        } else if (strcmp(param, "THREADS") == 0) { // number of threads flag
            sscanf(line, "%s %d\n", param, &params->num_threads);
        } else if (strcmp(param, "REPLAY") == 0) { // highscore replay number flag
            sscanf(line, "%s %d\n", param, &params->print_replay);
        } else if (strcmp(param, "LAYER") == 0) { // ann layer flag
            sscanf(line, "%s %d %d %s\n", param, &params->shape[RIDX(params->num_layers, 0, 2)], &params->shape[RIDX(params->num_layers, 1, 2)], activation);
            if (strcmp(activation, "sigmoid") == 0) { params->activation[params->num_layers] = sigmoid; }
            else { params->activation[params->num_layers] = sigmoid; } // everything is forced to be sigmoid -- WIP
            params->num_layers++;
        } else { // unknown symbol
            perror(line);
            printf("\n\nERR: Unknown symbol on line %d (please fix/remove) -- each line must start with of { MODEL, POP_WIDTH, GEN_COUNT, MUTATE, SURVIVE, LAYER, ACTIVATION, or '//' }\n\n\n", line_num);
            exit(127);
        }
        line_num++;
    }
    fclose(file);
    
    // verify the parameters are valid
    verify_parameters(params);
    return params;
}