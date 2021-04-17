//
//  nndefs.h
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#ifndef nndefs_h
#define nndefs_h

#include <math.h>
#include "envdefs.h"

// macro to return index of 2D array in a 1D representation
#define RIDX(i,j,d) (j + i * d)

#define SHAPE_DIM 2

typedef struct ann ann;
typedef struct ann_set ann_set;
typedef double (*funct) (double);

struct ann {
    int num_l;
    int num_obs;
    int num_n;
    int num_w;
    int *shape;
    double *w;
    double *b;
    double *a_obs;
    funct *A;
};

struct ann_set {
    int num_net;
    int gen;
    double *fitness;
    ann *data;
};

// nn functs
double sigmoid(double);
void destroy_ann(ann *);
void init_ann(ann *, int, int *, funct *);
double * forward(ann *, int, int, double *);
void set_parameters(ann *, double *, double *);
void copy_parameters(ann *, ann *);

// nn controller functions
void free_ann_set(ann_set *);
void init_ann_set(ann_set *, int, int, int *, funct *);
void spawn_ann(double, ann *, ann *, ann *);
void determine_most_fit_parents(int, double, int *, double *, ann_set *);
void spawn_ann_gen(ann_set *, double, double, int, int, int *, funct *);
int run_ann(ann *, dist_data *);
void run_ann_set(ann_set *, int *, dist_data *);

#endif /* nndefs_h */
