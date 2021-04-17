//
//  envcntr.c
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gsdefs.h"


/*
 * destroy_env_set - Frees an env_set struct and it child members
 */
void free_env_set(env_set *src)
{
    for (int i = 0; i < (src->num_env);i++) { destroy_env(&(src->data[i])); }
    free(src->data);
    free(src->dist_d);
    free(src);
    return;
}


/*
 * calc_dist_to_wall - Calculates distance from snake head to the walls in the 8 directions
 */
static void calc_dist_to_wall(env *src, dist_data *data)
{
    double d1 = (double) src->h->y - 1;
    double d2 = (double) src->env_dim - src->h->x;
    double d3 = (double) src->env_dim - src->h->y;
    double d4 = (double) src->h->x - 1;
            
    // set wall distances
    data->wall[0] = d1; // north
    data->wall[1] = d2; // east
    data->wall[2] = d3; // south
    data->wall[3] = d4; // west
    data->wall[4] = (d1 >= d2)? sqrt((d2 * d2) + (d2 * d2)): sqrt((d1 * d1) + (d1 * d1)); // north east
    data->wall[5] = (d2 >= d3)? sqrt((d3 * d3) + (d3 * d2)): sqrt((d2 * d2) + (d2 * d2)); // south east
    data->wall[6] = (d3 >= d4)? sqrt((d4 * d4) + (d4 * d4)): sqrt((d3 * d3) + (d3 * d3)); // south west
    data->wall[7] = (d4 >= d1)? sqrt((d1 * d1) + (d1 * d1)): sqrt((d4 * d4) + (d4 * d4)); // north west
    return;
}


/*
 * calc_dist_to_body - Calculates distance from snake head to its body in the 8 directions
 */
static void calc_dist_to_body(env *src, dist_data *data)
{
    int dx, dy;
    double d;
    snake_node *head = src->h;
    snake_node *curr = head;

    while (curr->c != NULL) {
        curr = curr->c;

        // calculate distance from head to current node
        dx = head->x - curr->x;
        dy = head->y - curr->y;
        
        // determine if current node intercepts a ray
        if (abs(dx) == abs(dy)) { // node is diagonal from snake head
            if ((dx > 0) && (dy > 0)) { // north west
                dx = abs(dx) - 1; 
                dy = abs(dy) - 1;
                d = sqrt((double)((dx * dx) + (dy * dy)));
                if ((data->body[4] == NOT_FOUND) || (d < data->body[4])) { 
                    data->body[4] = d;
                }
            } else if ((dx > 0) && (dy < 0)) { // south west
                dx = abs(dx) - 1; 
                dy = abs(dy) - 1;
                d = sqrt((double)((dx * dx) + (dy * dy)));
                if ((data->body[5] == NOT_FOUND) || (d < data->body[5])) {
                    data->body[5] = d;
                }
            } else if ((dx < 0) && (dy < 0)) { // south east
                dx = abs(dx) - 1; 
                dy = abs(dy) - 1;
                d = sqrt((double)((dx * dx) + (dy * dy)));
                if ((data->body[6] == NOT_FOUND) || (d < data->body[6])) {
                    data->body[6] = d;
                }
            } else if ((dx < 0) && (dy > 0)) { // north east
                dx = abs(dx) - 1; 
                dy = abs(dy) - 1;
                d = sqrt((double)((dx * dx) + (dy * dy)));
                if ((data->body[7] == NOT_FOUND) || (d < data->body[7])) {
                    data->body[7] = d;
                }
            }
        } else if (dx == 0) { // node is north or south of snake head
                if (dy > 0) { // north
                    if ((data->body[0] == NOT_FOUND) || (abs(dy) < data->body[0])) {
                        data->body[0] = abs(dy) - 1;
                    }
                } else if (dy < 0) { // south
                    if ((data->body[1] == NOT_FOUND) || (abs(dy) < data->body[1])) {
                        data->body[1] = abs(dy) - 1;
                    }
                }
        } else if (dy == 0) { // node is east or west of the snake head
            if (dx > 0) { // west
                if ((data->body[2] == NOT_FOUND) || (abs(dx) < data->body[2])) {
                    data->body[2] = abs(dx) - 1;
                }
            } else if (dx < 0) { // east
                if ((data->body[3] == NOT_FOUND) || (abs(dx) < data->body[3])) {
                    data->body[3] = abs(dx) - 1;
                }
            }
        }
    }
    return;
}


/*
 * calc_dist_to_apple - Calculates distance from snake head to apple in the 8 directions
 */
static void calc_dist_to_apple(env *src, dist_data *data) 
{
    // calculate distance from head to apple
    int dx = src->a->x - src->h->x;
    int dy = src->a->y - src->h->y;
    
    // determine if apple intercepts a ray
    if (abs(dx) == abs(dy)) { // apple is diagonal from snake head
        if ((dx > 0) && (dy > 0)) { // north west
            dx = abs(dx) - 1; 
            dy = abs(dy) - 1;
            data->apple[4] = sqrt((double)((dx * dx) + (dy * dy)));
        } else if ((dx > 0) && (dy < 0)) { //south west
            dx = abs(dx) - 1; 
            dy = abs(dy) - 1;
            data->apple[5] = sqrt((double)((dx * dx) + (dy * dy)));
        } else if ((dx < 0) && (dy < 0)) { // south east
            dx = abs(dx) - 1; 
            dy = abs(dy) - 1;
            data->apple[6] = sqrt((double)((dx * dx) + (dy * dy)));
        } else if ((dx < 0) && (dy > 0)) { // north east
            dx = abs(dx) - 1; 
            dy = abs(dy) - 1;
            data->apple[7] = sqrt((double)((dx * dx) + (dy * dy)));
        }
    } else if (dx == 0) { // apple is north or south of snake head
            if (dy > 0) { // north
                data->apple[0] = abs(dy) - 1;
            } else if (dy < 0) { // south
                data->apple[1] = abs(dy) - 1;
            }
    } else if (dy == 0) { // apple is east or west of snake head
        if (dx > 0) { // west
            data->apple[2] = abs(dx) - 1;
        } else if (dx < 0) { // east
            data->apple[3] = abs(dx) - 1;
        }
    }
    return;
}


/*
 * update_dist_data - Updates the env dist data struct member from a specified given env data
 */
void update_dist_data(env_set * src, int i) 
{
    if (src->data[i].alive == 0) {
        // set flag values for dead env
        src->dist_d[i].wall[0] = NOT_SET; 
        return;
    }

    // calculate env data for alive env
    for (int j = 0; j < 8; j++) { // set all distance rays to default values
        src->dist_d[i].wall[j] = NOT_FOUND;
        src->dist_d[i].body[j] = NOT_FOUND; 
        src->dist_d[i].apple[j] = NOT_FOUND;
    }
    
    // calculate distance to walls
    calc_dist_to_wall(&src->data[i], &src->dist_d[i]);

    // calculate distance to snake body
    calc_dist_to_body(&src->data[i], &src->dist_d[i]);

    // calculate distance to apple
    calc_dist_to_apple(&src->data[i], &src->dist_d[i]);
    return;
}


/*
 * init_env_set - Initializes an env set of a specified env dimesion and env count
 */
void init_env_set(env_set *src, int ct, int dim)
{
    src->data = (env *) malloc(ct * sizeof(env));
    src->dist_d = (dist_data *) malloc(ct * sizeof(dist_data));
    src->num_env = ct;
    src->is_active = 1;
    
    // build specified number of environments
    for (int i = 0; i< ct; i++) {
        src->data[i].a = (apple *) malloc(sizeof(apple));
        init_env(dim, &src->data[i]);
        update_dist_data(src, i);
    }
    return;
}


/*
 * reset_env_set - Resets given env set struct
 */
void reset_env_set(env_set *src)
{
    for (int i = 0; i < src->num_env; i++) {
        reset_env(&src->data[i]);
        update_dist_data(src, i);
    }
    src->is_active = 1;
    return;
}


/*
 * run_env_set - Runs an env set according to a given action set
 */
void run_env_set(env_set *src, int *a)
{
    src->is_active = 0;
    for (int i = 0; i < (src->num_env); i++) {
        // runs env action if env is still active
        if ((src->data[i].alive)? run_env_action(a[i], &(src->data[i])): 0) {
            src->is_active = 1;
            update_dist_data(src, i);
        }
    }
    return;
}