//
//  main.c
//  genetic-snake
//
//  Created by Alexander Gonsalves
//  04/17/2021

#include "gsdefs.h"


int main(int argc, const char *argv[])
{
    struct timespec start, finish;
    double elapsed;
    srand(time(NULL));

    // get start time
    clock_gettime(CLOCK_MONOTONIC, &start);

    // start the model -- good luck
    genetic_snake(argv[1]);
    
    // get end time
    clock_gettime(CLOCK_MONOTONIC, &finish);

    //calculate run time and exit program
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("\n\n\n  Good run   -   %f min // %f seconds\n\n\n", elapsed/60.0, elapsed);
    return 0;
}