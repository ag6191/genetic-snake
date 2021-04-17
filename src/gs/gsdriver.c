//
//  gsdriver.c
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#include <pthread.h>
#include "gsdefs.h"


/*
 * threaded_genetic_snake - Starts the specified number of threads to run the genetic algorithm model with given parameters
 */
static void threaded_genetic_snake(gs_params *params)
{
    pthread_t tid[MAX_NUM_THREADS + 1];
    thread_data t_data;
    model_flag = params->num_threads;

    // setup thread data struct
    init_thread_data_struct(&t_data, params);

    // setup thread variables
    init_thread_variables();
    
    // init ann and env set with given params
    init_env_set(t_data.env_s, params->pop_size, params->env_width);
    init_ann_set(t_data.ann_s, params->pop_size, params->num_layers, (int *) &params->shape, (funct *) &params->activation);

    // create model controller thread
    if (pthread_create(&(tid[0]), NULL, model_controller_thread, &t_data) != 0) {
        printf ("\n\nERR: pthread_create error for model controller thread\n"); 
        exit (1); 
    }

    // create snake controller threads
    for (int i = 0; i < params->num_threads; i++) {
        if (pthread_create(&(tid[i + 1]), NULL, snake_controller_thread, &t_data) != 0) {
            printf ("\n\nERR: pthread_create error for snake controller thread (%d)\n", i); 
            exit (1); 
        }
    }

    // wait for all threads to finish execution
    for (int i = 0; i < (params->num_threads + 1); i++) {
        if (pthread_join (tid[i], NULL) != 0) {
            if (i == 0) { printf ( "\n\nERR: pthread_join error for model controller thread\n"); } 
            else { printf ( "\n\nERR: pthread_join error for snake controller thread (%d)\n", i); }
            exit(127); 
        }
    }

    // final cleanup
    free(t_data.action_set);
    free_env_set(t_data.env_s);
    free_ann_set(t_data.ann_s);
    free(t_data.surv_idx);
    free(t_data.fitness_prob);
    return;
}


/*
 * sequential_genetic_snake - Runs the specified genetic algorithm model on a single thread with given parameters
 */
static void sequential_genetic_snake(gs_params *params)
{
    int highscore = 0;
    int *action_set = (int *) malloc(params->pop_size * sizeof(int));
    env_set *env_s = (env_set *) malloc(sizeof(env_set));
    ann_set *ann_s = (ann_set *) malloc(sizeof(ann_set));

    // init ann and env set with given parameters
    init_env_set(env_s, params->pop_size, params->env_width);
    init_ann_set(ann_s, params->pop_size, params->num_layers, (int *) &params->shape, (funct *) &params->activation);
    
    // run specififed number of generations
    for (int gen_i = 0; gen_i < params->gen_ct; gen_i++) {
        // runs coupled ann/env sets until there are no active snakes left
        do {
            run_ann_set(ann_s, action_set, env_s->dist_d);
            run_env_set(env_s, action_set);
        } while (env_s->is_active);

        // prints gen stats and any highscoring run data
        print_gen_stats(gen_i, &highscore, ann_s, env_s, params->print_replay);
        
        // skips spawning last gen
        if ((gen_i + 1) == params->gen_ct) break;
        
        // compute fitness of current generation
        compute_set_fitness(ann_s, env_s);

        // spawn next generation
        spawn_ann_gen(ann_s, params->survive, params->mutate, params->pop_size, params->num_layers, (int *) &params->shape, (funct *) &params->activation);

        // reset environment for next generation
        reset_env_set(env_s);

        // increment ann set generation number
        ann_s->gen += 1;
    }
    
    // final cleanup
    free(action_set);
    free_env_set(env_s);
    free_ann_set(ann_s);
    return;
}


/*
 * genetic_snake - Reads model parameters from a given file and starts the appropriate model with specified computation type
 */ 
void genetic_snake(const char *file_name)
{
    // read parameters from input file
    gs_params *params = read_parameters_from_file(file_name);

    // print model parameters
    print_model_parameters(params);

    // ask user to start the model
    print_start_prompt();

    // start model by number of threads specified
    if (params->num_threads == 1) {
        sequential_genetic_snake(params);
    } else {
        threaded_genetic_snake(params);
    }
    
    // cleanup
    free(params);
    return;
}