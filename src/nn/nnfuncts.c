//
//  nnfuncts.c
//  genetic-snake
//
//  Created by Alexander Gonsalves 
//  04/17/2021

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "nndefs.h"
#include "gsdefs.h"


/*
 * sigmoid - Returns sigmoid function result of given value
 */
double sigmoid(double x) 
{
    return (1/(1+exp(-x)));
}


/*
 * ann_init - Initializes a network with a given set of parameters
 */
void init_ann(ann *net, int num_l, int *shape, funct *A)
{
    int idx_n = 0;
    int idx_w = 0;
    int num_n = 0; 
    int num_w = 0;

    // count num nodes and weights
    for (int l = 0; l < num_l; l++) {
        num_n += shape[RIDX(l,1,SHAPE_DIM)];
        num_w += shape[RIDX(l,0,SHAPE_DIM)] * shape[RIDX(l,1,SHAPE_DIM)];
    }

    // exit if net size is invalid
    if ((num_n == 0) || (num_w == 0)) {
        printf("\n\nERR: net size is invalid\n");
        exit(127);
    } 
    
    // set ann numeric values
    net->num_l = num_l;
    net->num_obs = 1;
    net->num_n = num_n;
    net->num_w = num_w;

    // setup ann tensors
    net->shape = (int *) malloc(SHAPE_DIM * num_l * sizeof(int));
    net->w = (double *) malloc(num_w * sizeof(double));
    net->b = (double *) malloc(num_n * sizeof(double));
    net->a_obs = (double *) malloc(num_n * sizeof(double));
    net->A = (funct *) malloc(num_l * sizeof(funct));
    
    // idx_n: node idx of current layer
    // idx_w: weight idx of current layer
    
    // init tensors by layer and setup node/weight values
    for (int l = 0; l < num_l; l++) {
        // copy activation and shape by layer
        net->A[l] = A[l];
        net->shape[RIDX(l,0,SHAPE_DIM)] = shape[RIDX(l,0,SHAPE_DIM)];
        net->shape[RIDX(l,1,SHAPE_DIM)] = shape[RIDX(l,1,SHAPE_DIM)];

        // setup net parameter values by layer
        for (int j = 0; j < (shape[RIDX(l,1,SHAPE_DIM)]); j++) { // nodes per layer
            net->a_obs[idx_n + j] = 0;
            net->b[idx_n + j] = rand_norm() / 3.0;
            for(int k = 0; k < (shape[RIDX(l,0,SHAPE_DIM)]); k++) { // weights per node
                net->w[idx_w + RIDX(j, k, shape[RIDX(l,0,SHAPE_DIM)])] = rand_norm()/3.0;
            }
        }

        // increment node and weight index by current layer's shape
        idx_n += shape[RIDX(l,1,SHAPE_DIM)];
        idx_w += shape[RIDX(l,0,SHAPE_DIM)] * shape[RIDX(l,1,SHAPE_DIM)];
    }
    return;
}


/*
 * rebuild_obs_set - Frees and allocates a new activation set for a given number of observations
 */
static void rebuild_obs_sets(ann *net)
{
    int idx_n;
    free(net->a_obs);
    
    // build a new set wrt given num observations in ann struct
    net->a_obs = (double *) malloc(net->num_n * net->num_obs * sizeof(double));
    for (int o = 0; o < net->num_obs; o++) { // per observation
        idx_n = 0;
        for (int l = 0; l < (net->num_l); l++) { // per layer
            for (int j = 0; j < (net->shape[RIDX(l,1,SHAPE_DIM)]); j++) { // per node
                net->a_obs[RIDX(o,(idx_n + j), (net->num_n))] = 0;
            }

            // increment node index by current layer's shape
            idx_n += net->shape[RIDX(l,1,SHAPE_DIM)];
        }
    }
    return;
}


/*
 * forward - Forward propagation with a given ann and observation set
 */
double * forward(ann *net, int num_obs, int num_feat, double *x)
{
    int idx_w, idx_n, idx_n_prev;
    double z;
    double *y = (double *) malloc(num_obs * net->shape[RIDX((net->num_l) - 1, 1, SHAPE_DIM)] * sizeof(double));
    
    // check if num features is compatible with input layer
    if (num_feat != net->shape[RIDX(0,0,SHAPE_DIM)]) {
        printf("\n\nERR: forward prop input features incompatible with net input layer\n");
        exit(127);
    }

    // restructure net activation if num obs is different
    if (net->num_obs != num_obs) {
        net->num_obs = num_obs;
        rebuild_obs_sets(net);
    }
    
    // forward propagation
    for (int o = 0; o < num_obs; o++) { // by observation -- not used in current version (each ann only has 1 game/observation)
        idx_w = 0;
        idx_n = 0;

        // input layer activation
        for (int j = 0; j < (net->shape[RIDX(0,1,SHAPE_DIM)]); j++){ // by first layer node
            z = net->b[j]; // add bias
            for (int k = 0; k < (net->shape[RIDX(0,0,SHAPE_DIM)]); k++) { // by input feature
                z += net->w[RIDX(j,k,(net->shape[RIDX(0,0,SHAPE_DIM)]))] * x[RIDX(o, k, num_feat)]; // add product of weight and feature input
            }
            net->a_obs[RIDX(o, j, net->num_n)] = ((net->A[0])(z)); // apply activation function
        }
        
        // increment node and weight index by first layer shape
        idx_n_prev = idx_n;
        idx_n += net->shape[RIDX(0,1,SHAPE_DIM)];
        idx_w += net->shape[RIDX(0,1,SHAPE_DIM)] * net->shape[RIDX(0,0,SHAPE_DIM)];
        
        // following layer activation
        for (int l = 1; l < net->num_l; l++) { // by layer
            for (int j = 0; j < (net->shape[RIDX(l,1,SHAPE_DIM)]); j++) { // by node
                z = net->b[idx_n + j]; // add bias
                for (int k = 0; k < (net->shape[RIDX(l,0,SHAPE_DIM)]); k++) { // by previous layer activation
                    z += net->w[idx_w + RIDX(j,k,net->shape[RIDX(l,0,SHAPE_DIM)])] * net->a_obs[RIDX(o,idx_n_prev + k, net->num_n)]; // add product of weight and previous activation
                }
                net->a_obs[RIDX(o, idx_n + j, (net->num_n))] = ((net->A[l])(z)); // apply activation function
                
                // build output array if the final layer activation was computed
                if ((l + 1) == net->num_l) {
                    y[RIDX(o, j, net->shape[RIDX((net->num_l-1), 1, SHAPE_DIM)])] = ((net->A[l])(z));
                }
            }

            // increment node and weight index by current layer's shape
            idx_n_prev = idx_n; 
            idx_n += net->shape[RIDX(l,1,SHAPE_DIM)]; 
            idx_w += net->shape[RIDX(l,1,SHAPE_DIM)] * net->shape[RIDX(l,0,SHAPE_DIM)];
        }
    }
    return y;
}


/*
 * set_parameters - Copies given weight and bias values to an ann
 */
void set_parameters(ann *net, double *w, double *b)
{
    int idx_w = 0;
    int idx_n = 0;
    
    // deepcopy weights and bias
    for (int l = 0; l < net->num_l; l++) { // by layer
        for (int j = 0; j < (net->shape[RIDX(l,1,SHAPE_DIM)]); j++) { // by node
            net->b[idx_n + j] = b[idx_n + j];
            for (int k = 0; k < (net->shape[RIDX(l,0,SHAPE_DIM)]); k++) { // by weight
                net->w[idx_w + RIDX(j,k,net->shape[RIDX(l,0,SHAPE_DIM)])] = w[idx_w + RIDX(j,k,net->shape[RIDX(l,0,SHAPE_DIM)])];
            }
        }

        // increment node and weight index by current layer's shape
       idx_n += net->shape[RIDX(l,1,SHAPE_DIM)];
       idx_w += net->shape[RIDX(l,0,SHAPE_DIM)] * net->shape[RIDX(l,1,SHAPE_DIM)];
    }
    return;
}


/*
 * copy_parameters - Copies given source ann parameters to destination ann
 */
void copy_parameters(ann *src, ann *dst)
{
    src->num_l = src->num_l;
    dst->num_obs = src->num_obs;
    dst->num_n = src->num_n;
    dst->num_w = src->num_w;
    for (int k = 0; k < src->num_l; k++) {dst->shape[k] = src->shape[k];} // shape
    for (int k = 0; k < src->num_w; k++) {dst->w[k] = src->w[k];} // weight
    for (int k = 0; k < src->num_n; k++) {dst->b[k] = src->b[k];} // bias
    for (int k = 0; k < src->num_l; k++) { dst->A[k] = src->A[k];} // activation
    return;
}


/*
 * destroy_ann - Frees all values in a given ann
 */
void destroy_ann(ann *net)
{
    free(net->shape);
    free(net->w);
    free(net->b);
    free(net->a_obs);
    free(net->A);
    return;
}