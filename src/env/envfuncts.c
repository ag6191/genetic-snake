//
//  snake.c
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#include <stdlib.h>
#include <stdio.h>
#include "envdefs.h"
#include "gsdefs.h"


/*
 * record_move_data - Adds a move data node to the front of the move data linked list
 */
static void record_move_data(env *src, int action, int apple_eaten) 
{
    // set up new move data link
    move_data *curr_move = (move_data *) malloc(sizeof(move_data));
    curr_move->action = action;
    curr_move->apple_eaten = apple_eaten;
    curr_move->next_move = NULL;
    curr_move->prev_move = NULL;

    // links the last added move to the current move if it exists
    if (src->m_data != NULL) {
        curr_move->prev_move = src->m_data;
        src->m_data->next_move = curr_move;
    }
    
    // add new move to start of linked list
    src->m_data = curr_move;
    return;
}


/*
 * record_apple_data - Adds a apple data node to the end of the apple data linked list
 */
static void record_apple_data(env *src)
{
    apple_data *curr_a = src->a_data;

    // handles adding an apple node if it is the first to be added
    if (curr_a == NULL) {
        src->a_data = (apple_data *) malloc(sizeof(apple_data));
        src->a_data->x = src->a->x;
        src->a_data->y = src->a->y;
        src->a_data->next_apple = NULL;
        return;
    }

    // traverses apple node linked list to find the last added apple
    while (curr_a->next_apple != NULL) { curr_a = curr_a->next_apple; }

    // adds the new apple to the end of the linked list
    curr_a->next_apple = (apple_data *) malloc(sizeof(apple_data));
    curr_a->next_apple->x = src->a->x;
    curr_a->next_apple->y = src->a->y;
    curr_a->next_apple->next_apple = NULL;
    return;
}


/*
 * add_snake_node - Adds a snake body node to the end of the snake
 */
static void add_snake_node(snake_node *curr, int x, int y)
{
    curr->c = (snake_node *) malloc(sizeof(snake_node));
    curr->c->x = x;
    curr->c->y = y;
    curr->c->p = curr;
    curr->c->c = NULL;
    return;
}


/*
 * init_apple - Sets up a given apple struct and records its position
 */
void init_apple(int n, env *src )
{   
    // place an apple that does not conflict with the snake initial spawn locations
    do {
        src->a->x = rand_int(1, n);
        src->a->y = rand_int(1, n);
    } while ((src->a->x == (n/2)) && ((src->a->y == (n/2)) || 
        (src->a->y == (n/2)+1) || (src->a->y == (n/2)+2)));
    
    // record first apple in the memory chain
    record_apple_data(src);
    return;
}


/*
 * init_snake - Sets up n linked snake structs in memory for the given pointer
 */
void init_snake(int n, int len, env *src) 
{
    // head
    src->h = (snake_node *) malloc(sizeof(snake_node));
    src->h->x = n/2;
    src->h->y = n/2;
    src->h->p = NULL;

    // node 1
    src->h->c = (snake_node *) malloc(sizeof(snake_node));
    src->h->c->x = n/2;
    src->h->c->y = n/2 + 1;
    src->h->c->p = src->h;

    // node 2
    src->h->c->c = (snake_node *) malloc(sizeof(snake_node));
    src->h->c->c->x = n/2;
    src->h->c->c->y = n/2 + 2;
    src->h->c->c->p = src->h->c;
    src->h->c->c->c = NULL;
    return;
}


/*
 * free_snake_chain - Dealocates a given snake's linked list from the head
 */
void free_snake_chain(snake_node *head)
{
    snake_node *curr = head;
    snake_node *garbage;
    while (curr != NULL) {
        garbage = curr;
        curr = curr->c;
        free(garbage);
    }
    return;
}


/*
 * free_apple_data_chain - Frees an apple data chain from given node
 */
static void free_apple_data_chain(apple_data *src) 
{
    apple_data *curr = src;
    apple_data *garbage;
    while (curr != NULL) {
        garbage = curr;
        curr = curr->next_apple;
        free(garbage);
    }
    return;
}


/*
 * free_move_data_chain - Frees a move data chain from given node 
 */
static void free_move_data_chain(move_data *src) 
{
    move_data *curr = src;
    move_data *garbage;
    while(curr != NULL) {
        garbage = curr;
        curr = curr->prev_move;
        free(garbage);
    }
    return;
}


/*
 * destroy_env - Dealocates contents of env data
 */
void destroy_env(env *src)
{
    free_apple_data_chain(src->a_data);
    free_move_data_chain(src->m_data);
    free_snake_chain(src->h);
    free(src->a);
    return;
}


/*
 * reset_env - Deallocates env struct members and resets them
 */
void reset_env(env *src)
{
    free_apple_data_chain(src->a_data);
    free_move_data_chain(src->m_data);
    free_snake_chain(src->h);
    init_env(src->env_dim, src);
    return;
}


/*
 * init_env - Initializes a given env struct
 */
void init_env(int dim, env *src)
{
    // init snake
    init_snake(dim, MIN_SNAKE_LEN, src);

    // init apple
    src->a_data = NULL;
    init_apple(dim, src);

    // set env values
    src->m = 0;
    src->m_n = 0;
    src->n = 0;
    src->len = MIN_SNAKE_LEN;
    src->env_dim = dim;
    src->alive = 1;
    src->m_data = NULL;
    return;
}


/*
 * eat_apple - Spawns a new apple and adjusts score and apple data
 */
static void eat_apple(env *src) 
{
    int bad_position;
    snake_node *curr_n;
    
    do {
        // get new apple position
        bad_position = 0;
        src->a->x = rand_int(1, src->env_dim);
        src->a->y = rand_int(1, src->env_dim);
        curr_n = src->h;

        // check for conflict with snake body
        while (curr_n != NULL) {
            if ((curr_n->x == src->a->x) && (curr_n->y == src->a->y)) { 
                bad_position = 1; 
                break; 
            }
            curr_n = curr_n->c;
        }
    } while (bad_position);
    src->n++;
    src->m_n = 0;
    
    // record the new apple
    record_apple_data(src);
    return;
}


/*
 * interpret_action - Interprets a given action integer and adjusts change in x and y by reference
 */
static void interpret_action(int *a, int *dx, int *dy)
{
    // determine head position change
    switch (*a) {
        case UP: // move up
            *dx = 0;
            *dy = -1;
            break;
        case DOWN: // move down
            *dx = 0;
            *dy = 1;
            break;
        case LEFT: //move left
            *dx = -1;
            *dy = 0;
            break;
        case RIGHT: //move right
            *dx = 1;
            *dy = 0;
            break;
        default:
            printf("\n\nERR: - undefiend move - %d\n", *a);
            exit(127);
    }
    return;
}


/*
 * run_env_action - Modifies an env struct according to a given action and returns 1 if snake is alive after the action
 */
int run_env_action(int a, env *src)
{
    snake_node *head = src->h;
    snake_node *curr = src->h;
    int x2,y2,dx,dy;
    int x1 = curr->x;
    int y1 = curr->y;
    
    // adjust move counter (total moves and moves per apple)
    src->m++;
    src->m_n++;

    // kill snake if it is out of moves for this apple
    if (src->m_n > MAX_MOVES_PER_APPLE) {
        src->alive = 0;
        return (src->alive);
    }
    
    // interpret action to determine snake position change
    interpret_action(&a, &dx, &dy);

    // apply the move to snake head
    if ((((curr->x) + dx) != curr->c->x) || (((curr->y) + dy) != curr->c->y)) {
        curr->x = curr->x + dx;
        curr->y = curr->y + dy;
    } else { // snake went backwards (invalid move)
        src->alive = 0;
        return (src->alive);
    }
    
    // check if snake went off the board
    if ((head->x < 1) || (head->x > src->env_dim) || (head->y < 1) || (head->y > src->env_dim)){
        src->alive = 0;
    }
    
    // move linked snake nodes
    do {
        curr = curr->c;
        x2 = curr->x;
        y2 = curr->y;
        curr->x = x1;
        curr->y = y1;
        x1 = x2;
        y1 = y2;
        
        // check if snake ate itself
        if ((head->x == curr->x) && (head->y == curr->y)){
            src->alive = 0;
        }
    } while (curr->c != NULL);
    
    // check if the apple was eaten and add a new move node and apple node if applicable
    if ((head->x == src->a->x) && (head->y == src->a->y)) {
        record_move_data(src, a, 1);
        add_snake_node(curr, x1, y1);
        eat_apple(src);
    } else {
        record_move_data(src, a, 0);
    }
    
    return (src->alive);
}