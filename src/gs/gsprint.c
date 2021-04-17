//
//  gsprint.c
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#include <stdio.h>
#include "gsdefs.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


/*
 * print_model_parameters - Prints model parameters from given gs_params struct
 */
void print_model_parameters(gs_params * params)
{
    // print spacer
    for (int i = 0; i < 15; i++ ) { printf("\n\n"); }

    // print program details
    printf("+++++++  PROGRAM DETAILS  +++++++\n\n");
    if (params->num_threads > 1) {
        printf("  EXECUTION TYPE          MULTI-THREAD\n");
        printf("  THREADS                 %d\n", params->num_threads);
    } else {
        printf("  EXECUTION TYPE          SEQUENTIAL\n");
        printf("  THREADS                 %d\n", params->num_threads);
    }
    printf("  REPLAY                  ");
    if (params->print_replay) {
        printf(">= %d APPLES\n\n", params->print_replay);
    } else {
        printf("OFF\n\n");
    }

    // print model parameters
    printf("+++++++  MODEL PARAMETERS  +++++++\n\n");
    printf("  POPULATION SIZE         %d\n", params->pop_size);
    printf("  NUM GENERATIONS         %d\n", params->gen_ct);
    printf("  MUTATE CHANCE           %0.2f%%\n", (params->mutate * 100));
    printf("  SURVIVAL CHANCE         %0.2f%%\n", (params->survive * 100));
    printf("  ENVIRONMENT DIM         %d x %d\n\n", (int) ENV_WIDTH, (int) ENV_WIDTH);

    // print details of each ann layer
    printf("+++++++  ANN PARAMETERS  +++++++\n\n");
    for (int i = 0; i < params->num_layers; i++) { 
        printf("  LAYER %d:  %d  %d \n", i + 1, params->shape[RIDX(i, 0, 2)], params->shape[RIDX(i, 1, 2)]); 
    }
    return;
}


/*
 * print_start_prompt - Prints a start prompt and waits for user input before continuing
 */
void print_start_prompt()
{
    printf("\n\n\n Press a key to begin...\n  (press control + c at any time to exit)\n\n");
    getchar();
    printf("starting\n\n");
    return;
}


/*
 * print_pop_stats - Prints the average of the current population's fitness, moves, and apples
 */
void print_pop_stats(ann_set *ann_s, env_set *env_s)
{
    double total_fitness = 0; 
    double total_apples = 0; 
    double total_moves = 0;

    // find the sum of the fitness, moves, and apples
    for (int i = 0; i < ann_s->num_net; i++) {
        total_fitness += ((double) env_s->data[i].m) * pow(2.0, (double) env_s->data[i].n);
        total_moves += env_s->data[i].m;
        total_apples += env_s->data[i].n;
    }

    // calculates and prints the average of the fitness, moves, and apples
    printf("avg fitness - %f, avg moves - %f, avg apples - %f \n", total_fitness/ann_s->num_net, total_moves/ann_s->num_net, total_apples/ann_s->num_net);
    return;
}

 
/*
 * print_env - Prints an env state from an array of ints
 */
static void print_env(int dim, int *d)
{
    // print top bar
    printf("   ");
    for (int i = 0; i < dim; i++){ printf("- "); }
    printf("\n");
    
    // print game board
    for (int i = 0; i < dim; i++){
        // print left bar
        printf(" | ");

        // print env board
        for (int j = 0; j < dim; j++) {
            switch (d[RIDX(i,j,dim)]) {
            case 1: // head
                printf("\033[0;92m# \033[0m");
                break;
            case 2: // body
                printf("\033[0;92m* \033[0m");
                break;
            case 3: // apple
                printf("\033[0;91m@ \033[0m");
                break;
            default:
                printf("  ");
                break;
            }
        }

        // print right bar
        printf("| \n");
    }
    
    // print bottom bar
    printf("   ");
    for (int i = 0; i<dim; i++){
        printf("- ");
    }
    printf("\n\n");
    return;
}


/*
 * print_env_set - Prints a given number of env in a set
 */
static void print_env_set(int dim, int n, env_set *src)
{
    int image_data[(dim*dim)];
    snake_node *curr;

    // prints every set member
    for (int e = 0; e < n; e++){
        // wipe image data array for every set member
        for (int i = 0; i < dim; i++){
            for (int j = 0; j < dim; j++){ image_data[RIDX(i,j,dim)] = 0;}
        }
        
        // record current apple in image data
        image_data[RIDX(((src->data[e].a->y) - 1), ((src->data[e].a->x) - 1), dim)] = 3;
        
        // record snake head if alive in image data
        curr = src->data[e].h;
        if (src->data[e].alive) { image_data[RIDX(((curr->y) - 1),((curr->x) - 1),dim)] = 1; }
        
        // record snake body nodes in image data
        while (curr->c != NULL) {
            curr = curr->c;
            image_data[RIDX(((curr->y)-1),((curr->x)-1),dim)] = 2;
        }
        
        // pass image data to the print env function to display the game board
        print_env(dim, (int *) &image_data);
    }
    return;
}


/*
 * print_ann_run_replay - Runs and prints a snake game replay with a given ann and env struct
 */
static void print_ann_run_replay(ann *a_src, env *e_src)
{
    // set up temp run memory
    int n = 0;
    env_set *tmp_env_s = (env_set *) malloc(sizeof(env_set));
    ann_set *tmp_ann_s = (ann_set *) malloc(sizeof(ann_set));
    int *tmp_action_set = (int *) malloc(sizeof(int));
    apple_data *curr_a = e_src->a_data->next_apple;
    move_data *curr_m = e_src->m_data;
   
    // init and copy set memory
    init_env_set(tmp_env_s, 1, e_src->env_dim);
    init_ann_set(tmp_ann_s, 1, a_src->num_l, a_src->shape, a_src->A);
    copy_parameters(a_src, &tmp_ann_s->data[0]);
   
   // setup first apple
    tmp_env_s->data[0].a->x = e_src->a_data->x;
    tmp_env_s->data[0].a->y = e_src->a_data->y;
    
    // get first move from the end of the move struct linked list
    while (curr_m->prev_move != NULL) { curr_m = curr_m->prev_move; }

    // print ann run in env
    do {
        // print the game board
        printf("apples:  %d   |   moves:  %d\n", tmp_env_s->data[0].n, tmp_env_s->data[0].m);
        print_env_set(e_src->env_dim, 1, tmp_env_s);

        // run coupled ann/env struct once
        run_ann_set(tmp_ann_s, tmp_action_set, tmp_env_s->dist_d);
        run_env_set(tmp_env_s, &curr_m->action);
        
        // set next replay apple if current apple was eaten
        if (n != tmp_env_s->data[0].n) {
            n = tmp_env_s->data[0].n;
            tmp_env_s->data[0].a->x = curr_a->x;
            tmp_env_s->data[0].a->y = curr_a->y;
            curr_a = curr_a->next_apple;
        }

        // increment move
        curr_m = curr_m->next_move;

        // sleep between env frames
        if (e_src->m < 200) {
            usleep(REPLAY_TIME_5);
        } else if (e_src->m < 800) {
            usleep(REPLAY_TIME_4);
        } else if (e_src->m < 1400) {
            usleep(REPLAY_TIME_3);
        } else if (e_src->m < 2000) {
            usleep(REPLAY_TIME_2);
        } else {
            usleep(REPLAY_TIME_1);
        }
    } while ((tmp_env_s->is_active) && (curr_m != NULL)); // exit if it is the last move or if the snake died
   
    // print the final game board
    printf("apples:  %d   |   moves:  %d        :( \n", tmp_env_s->data[0].n, tmp_env_s->data[0].m);
    print_env_set(e_src->env_dim, 1, tmp_env_s);
    
    // print cause of death
    if (tmp_env_s->data[0].m_n > MAX_MOVES_PER_APPLE) {
        printf("\n Snake ran out of moves (%d without next apple)\n", (int) MAX_MOVES_PER_APPLE);
    } else {
        printf("\n Snake made a bad move\n\n");
    }

    // print final stats
    printf("    moves: %d\n", tmp_env_s->data[0].m);
    printf("    apples eaten: %d\n\n\n", tmp_env_s->data[0].n);
    printf("    moves this apple: %d\n", tmp_env_s->data[0].m_n);

    // check if there is an inconsistency error in the reply and exit if so
    if (tmp_env_s->data[0].n != e_src->n) {
        printf("\n\nERR: inconsistent number of replay apples eaten\n\n\n");
        exit(127);
    }

    // final cleanup and exit
    free_env_set(tmp_env_s); 
    free_ann_set(tmp_ann_s); 
    free(tmp_action_set);
    return;
}


/*
 * print_gen_stats - Prints the metrics and replays for a generation
 */ 
void print_gen_stats(int gen_n, int *highscore, ann_set *ann_s, env_set *env_s, int print_replay) 
{
    // find highscoring snake
    int highscore_idx = 0;
    for (int i = 0; i < env_s->num_env; i++) {
        highscore_idx = (env_s->data[i].n > env_s->data[highscore_idx].n)? i: highscore_idx;
    }

    // print pop stats every print batch generation
    if ((gen_n + 1) % PRINT_BATCH == 0) {
        printf(":: GEN %d ::  ", gen_n + 1);
        print_pop_stats(ann_s, env_s);
    }

    // print highscoring stats if there is a new highscore
    if (env_s->data[highscore_idx].n > *highscore) {
        *highscore = env_s->data[highscore_idx].n;
        printf("\033[0;32m--  [GEN %d] New Highscore:  %d  --\033[0m\n", gen_n + 1, *highscore);

        // print highscore replay
        if ((print_replay) && (*highscore >= print_replay)) {
            print_ann_run_replay(&ann_s->data[highscore_idx], &env_s->data[highscore_idx]);
        }
    }
    return;
}