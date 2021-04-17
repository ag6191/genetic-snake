//
//  gsthread.c
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "gsdefs.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t snake_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t model_cond = PTHREAD_COND_INITIALIZER;


/*
 * sync_snake_thread - Barrier synchronization for the snake controller thread with a given flag
 */
static void sync_snake_thread(int* snake_flag)
{
    // return if the model controller thread signaled it has finished
    if (finished_flag) { return; }
    
    // signal to the model controller that a snake controller thread is at the sync barrier
    pthread_mutex_lock(&mutex);
    model_flag++;
    pthread_cond_signal(&model_cond);

    // wait for model controller thread to signal release
    while(!*snake_flag) {
        pthread_cond_wait(&snake_cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    return;
}


/*
 * sync_model_thread - Barrier synchronization for the model controller thread with given snake wait and sync flags
 */
static void sync_model_thread(int num_threads, int *curr_wait_flag, int *next_wait_flag, int *sync_flag)
{
    // wait for all snake threads to reach the sync barrier
    pthread_mutex_lock(&mutex);
    while (model_flag < num_threads) {
        pthread_cond_wait(&model_cond, &mutex);
    }

    // adjust flags and target for the snake controller threads' next operation before releasing them
    model_flag = 0;
    curr_target = 0;
    *curr_wait_flag = 1;
    *next_wait_flag = 0;
    *sync_flag = 1;

    // release all snake controller threads
    pthread_cond_broadcast(&snake_cond);
    pthread_mutex_unlock(&mutex);
    return;
}


/*
 * run_snake_thread - Snake controller thread function to run snakes with a barrier synchronization
 */
static void run_snake_thread(thread_data *t_data)
{
    int target;

    // return if the model controller thread signaled it has finished
    if (finished_flag) { return; }

    // wait for all other snake controller threads to reach the sync barrier
    sync_snake_thread(&wait_run_flag);

    while (run_flag) {
        pthread_mutex_lock(&mutex);

        // acquire and run a target if there are targets remaining
        if (curr_target < t_data->params->pop_size) {
            // get target and increment the shared current target
            target = curr_target;
            curr_target++;
            pthread_mutex_unlock(&mutex);

            // run ann/env until target snake is dead concurrently with other snake controller threads
            while (t_data->env_s->data[target].alive) {
                t_data->action_set[target] = run_ann(&t_data->ann_s->data[target], &t_data->env_s->dist_d[target]);
                run_env_action(t_data->action_set[target], &t_data->env_s->data[target]);

                // update the distance data for target ann/env
                update_dist_data(t_data->env_s, target);
            }
        } else {
            // singal to other snake controller threads that no target snakes remain 
            run_flag = 0;
            pthread_mutex_unlock(&mutex);
        }
    }
    return;
}


/*
 * compute_fitness_thread - Snake controller thread function to compute snake fitness with a barrier synchronization
 */
static void compute_fitness_thread(ann_set *ann_s, env_set *env_s)
{
    int target;

    // return if the model controller thread signaled it has finished
    if (finished_flag) { return; }

    // wait for all other snake controller threads to reach the sync barrier
    sync_snake_thread(&wait_compute_flag);

    while (compute_flag) {
        pthread_mutex_lock(&mutex);

        // acquire and run a target if there are targets remaining
        if (curr_target < ann_s->num_net) {
            // get target and increment the shared current target
            target = curr_target;
            curr_target++;            
            pthread_mutex_unlock(&mutex);

            // compute fitness of the target concurrently with other snake controller threads
            compute_ann_fitness(ann_s, env_s, target);
        } else {
            // singal to other snake controller threads that no target snakes remain
            compute_flag = 0;
            pthread_mutex_unlock(&mutex);
        }
    }
    return;
}


/*
 * spawn_ann_gen_thread - Snake controller thread function to spawn new snakes with a barrier synchronization
 */
static void spawn_ann_gen_thread(thread_data *t_data)
{
    int target;
    ann *parent_a;
    ann *parent_b;
    int ct = t_data->params->pop_size;
    int ct_surv = ct * t_data->params->survive;
    int ct_die = ct - ct_surv;

    // return if the model controller thread signaled it has finished
    if (finished_flag) { return; }

    // wait for all other snake controller threads to reach the sync barrier
    sync_snake_thread(&wait_spawn_flag);

    // select first snake controller thread to setup spawn variables while all other threads wait
    pthread_mutex_lock(&mutex);
    if (!spawn_setup_flag) { // first thread computes spawn variables
        spawn_setup_flag = 1;
        pthread_mutex_unlock(&mutex);

        // determine the most fit parents
        determine_most_fit_parents(t_data->params->pop_size, t_data->params->survive, t_data->surv_idx, t_data->fitness_prob, t_data->ann_s);
        
        // adjust flag and release all other snake controller threads
        pthread_mutex_lock(&mutex);
        spawn_setup_flag = 2;
        pthread_cond_broadcast(&snake_cond);
    } else if (spawn_setup_flag == 1) { // all snake controller threads besides the first one wait
        while (spawn_setup_flag < 2) {
            pthread_cond_wait(&snake_cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);

    while (spawn_flag) {
        pthread_mutex_lock(&mutex);

        // acquire and run a target if there are targets remaining
        if (curr_target < t_data->params->pop_size) {
            // get target and increment the shared current target by two
            target = curr_target;
            curr_target = curr_target + 2;
            pthread_mutex_unlock(&mutex);

            // determine two parents for the children snakes
            parent_a = &t_data->ann_s->data[t_data->surv_idx[(ct - 1 ) - rand_roulette(ct_surv, t_data->fitness_prob)]];
            parent_b = &t_data->ann_s->data[t_data->surv_idx[(ct - 1 ) - rand_roulette(ct_surv, t_data->fitness_prob)]];

            // spawn one/two snakes depending on the current target number
            spawn_ann(t_data->params->mutate, parent_a, parent_b, &t_data->ann_s->data[t_data->surv_idx[target]]);
            if ((target) + 1 < ct_die) { // prevents an extra snake from being spawned in the last snake controller thread to spawn children
                spawn_ann(t_data->params->mutate, parent_a, parent_b, &t_data->ann_s->data[t_data->surv_idx[target + 1]]);
            }
        } else {
            // singal to other snake controller threads that no target snakes remain
            spawn_flag = 0;
            pthread_mutex_unlock(&mutex);
        }
    }
    return;
}


/*
 * reset_env_thread - Snake controller thread function to reset snake environments with a barrier synchronization
 */
static void reset_env_thread(env_set * env_s)
{
    int target;

    // return if the model controller thread signaled it has finished
    if (finished_flag) { return; }

    // wait for all other snake controller threads to reach the sync barrier
    sync_snake_thread(&wait_reset_flag);

    while (reset_flag) {
        pthread_mutex_lock(&mutex);

        // acquire and run a target if there are targets remaining
        if (curr_target < env_s->num_env) {
            // get target and increment the shared current target
            target = curr_target;
            curr_target++;
            pthread_mutex_unlock(&mutex);

            // reset env target concurrently with other snake controller threads for the next generation
            reset_env(&env_s->data[target]);
            update_dist_data(env_s, target);
            env_s->is_active = 1;
        } else {
            // singal to other snake controller threads that no target snakes remain
            reset_flag = 0;
            pthread_mutex_unlock(&mutex);

        }
    }
    return;
}


/*
 * snake_controller_thread - Function that allows threads to run the genetic algorithm in a concurrent/parallel manner
 */
void * snake_controller_thread(void *void_t_data)
{
    thread_data *t_data = (thread_data *) void_t_data;

    while (!finished_flag) {
        // concurrently run all snakes
        run_snake_thread(t_data);

        // concurrently compute every snake's fitness
        compute_fitness_thread(t_data->ann_s, t_data->env_s);

        // concurrently spawn new snakes
        spawn_ann_gen_thread(t_data);

        // concurrently reset all env structs
        reset_env_thread(t_data->env_s);
    }
    pthread_exit ((void *) 0);
}


/*
 * model_controller_thread - Model controller thread function that manages all snake controller threads via barrier syncronization
 */
void * model_controller_thread(void *void_t_data)
{
    thread_data *t_data = (thread_data *) void_t_data;
    int highscore = 0;

    for (int gen_i = 0; gen_i < t_data->params->gen_ct; gen_i++) {
        spawn_setup_flag = 0;

        // wait for all snake controller threads to sync before running all snakes
        sync_model_thread(t_data->params->num_threads, &wait_run_flag, &wait_compute_flag, &run_flag);
        
        // wait for all snake controller threads to finish running all snakes
        pthread_mutex_lock(&mutex);
        while (model_flag < t_data->params->num_threads) { pthread_cond_wait(&model_cond, &mutex); }
        print_gen_stats(gen_i, &highscore, t_data->ann_s, t_data->env_s, t_data->params->print_replay);
        pthread_mutex_unlock(&mutex);

        // check if its time to skip last gen spawn/reset operations
        if ((gen_i + 1) == t_data->params->gen_ct) { break; }

        //wait for all snake controller threads to sync before computing snake fitness
        sync_model_thread(t_data->params->num_threads, &wait_compute_flag, &wait_spawn_flag, &compute_flag);
        
        // wait for all snake controller threads to sync before spawning new snakes
        sync_model_thread(t_data->params->num_threads, &wait_spawn_flag, &wait_reset_flag, &spawn_flag);

        // wait for all snake controller threads to sync before reseting snake environments
        sync_model_thread(t_data->params->num_threads, &wait_reset_flag, &wait_run_flag, &reset_flag);

        // increment ann set generation number
        t_data->ann_s->gen += 1;
    }

    // signal that all other threads should exit
    pthread_mutex_lock(&mutex);
    finished_flag = 1;
    wait_compute_flag = 1;
    pthread_cond_broadcast(&snake_cond);
    pthread_mutex_unlock(&mutex);

    pthread_exit ((void *) 0);
}