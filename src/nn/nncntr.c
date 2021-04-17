//
//  nncntr.c
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#include "gsdefs.h"


/*
 * init_ann_set - Initializes a set of ann of a given size and shape
 */
void init_ann_set(ann_set *ann_s, int ct, int num_l, int *shape, funct *A)
{
    ann_s->gen = 0;
    ann_s->num_net = ct;

    // malloc fitness and ann data array
    ann_s->fitness = (double *) malloc(ct * sizeof(double));
    ann_s->data = (ann *) malloc(ct * sizeof(ann));
    
    // init each ann set member
    for (int i = 0; i < ct; i++) { 
        ann_s->fitness[i] = NOT_SET;
        init_ann(&(ann_s->data[i]), num_l, shape, A);
    }
    return;
}


/*
 * spawn_ann - Initializes a child ann from two parent ann
 */
void spawn_ann(double mutate, ann *parent_a, ann *parent_b, ann *child)
{
    double m;
    
    // copy parent(a or b) weights or mutate
    for (int k = 0; k < parent_a->num_w; k++) {
        m = rand_double(0, 101);
        if (m > 100.0 * mutate) { // no mutation
            child->w[k] = (rand_int(0, 2))? parent_a->w[k]: parent_b->w[k];
        } else { // mutation
            child->w[k] = rand_norm();
        }
    }

    // copy parent(a or b) bias or mutate
    for (int k = 0; k < parent_a->num_n; k++) {
        m = rand_double(0, 101);
        if (m > 100.0 * mutate) { // no mutation
            child->b[k] = (rand_int(0, 2))? parent_a->b[k]: parent_b->b[k];
        } else { // mutation
            child->b[k] = rand_norm();
        }
    }
    return;
}


/*
 * determine_most_fit_parents - Calculates the most fit snakes and their fitness probability with respect to the sum of all snake's fitness
 */
void determine_most_fit_parents(int ct, double survive, int *surv_idx, double *fitness_prob, ann_set *ann_s)
{
    int tmp_s; double tmp_d;
    int ct_surv = ct * survive;
    double sum_fitness = 0;

    // determine (sort by) most fit members
    for (int i = 0; i < ct; i++) { surv_idx[i] = i; }
    for(int i = 0; i < ct; i++) {
        for (int j = i + 1; j < ct; j++) {
            if (ann_s->fitness[j] < ann_s->fitness[i]) {
                tmp_s = surv_idx[i];
                tmp_d = ann_s->fitness[i];
                
                surv_idx[i] = surv_idx[j];
                ann_s->fitness[i] = ann_s->fitness[j];
                
                surv_idx[j] = tmp_s;
                ann_s->fitness[j] = tmp_d;
            }
        }
    }
    
    // determine fitness probability of each surviving ann  fitness wrt total population fitness
    for (int i = (ct - 1); i >= (ct - ct_surv); i--) { sum_fitness += ann_s->fitness[i]; }
    for (int i = (ct - 1), k=0; i >= (ct - ct_surv); i--, k++) {
        fitness_prob[k] = ann_s->fitness[i] / sum_fitness;
    }
    return;
}


/*
 * spawn_ann_gen - Initializes a generation of ann structs of a given size with respect to spawn parameters and fitness
 */
void spawn_ann_gen(ann_set *ann_s, double survive, double mutate, int ct, int num_l, int *shape, funct *A) 
{
    int ct_surv = ct * survive;
    int ct_die = ct - ct_surv;
    int *surv_idx = (int *) malloc(ct * sizeof(int));
    double *fitness_prob = (double *) malloc(ct_surv * sizeof(double));
    ann *parent_a;
    ann *parent_b;
    
    // determine the most fit snake parents and calculate their fitness probability
    determine_most_fit_parents(ct, survive, surv_idx, fitness_prob, ann_s);
    
    // spawn two children from two parents and overwrite old ann
    for (int i = 0; i < ct_die; i+=2) {
        parent_a = &ann_s->data[surv_idx[(ct - 1 ) - rand_roulette(ct_surv, fitness_prob)]];
        parent_b = &ann_s->data[surv_idx[(ct - 1 ) - rand_roulette(ct_surv, fitness_prob)]];

        spawn_ann(mutate, parent_a, parent_b, &ann_s->data[surv_idx[i]]);
        if ((i) + 1 >= ct_die) break; // prevents an extra snake from being spawned
        spawn_ann(mutate, parent_a, parent_b, &ann_s->data[surv_idx[i + 1]]);
    }
    
    // cleanup and exit
    free(surv_idx); free(fitness_prob);
    return;
}


/*
 * run_ann - Runs a given ann with env out data and returns the best predicted move
 */
int run_ann(ann *net, dist_data *o_data)
{
    // return if out data is not set
    if (o_data->wall[0] == NOT_SET) { return NOT_SET; }

    // cast out data struct to array of doubles
    double *o = (double *) malloc(sizeof(dist_data));
    memcpy(o, o_data, sizeof(dist_data));
    
    // forward prop with given ann
    int max_idx = 0;
    double *y = forward(net, 1, 24, o);

    // determine max probability idx
    for (int i = 0; i < 4; i++) {
        if (y[i] > y[max_idx]) {
            max_idx = i; 
        }
    }
    free(y); free(o);
    return (max_idx + 1);
}


/*
 * run_ann_set - Runs a given set of ann with env out data and updates given actions set accordingly
 */
void run_ann_set(ann_set *net_s, int *actions, dist_data *o)
{
    for (int i = 0; i < (net_s->num_net); i++) { actions[i] = run_ann(&(net_s->data[i]), &(o[i])); }
    return;
}


/*
 * free_ann_set - Frees all ann members of a set and the set
 */
void free_ann_set(ann_set *src)
{
    for (int i = 0; i < (src->num_net); i++) {
        destroy_ann(&(src->data[i]));
    }
    free(src->data);
    free(src->fitness);
    free(src);
    return;
}