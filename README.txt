
This model trains artificial neural networks (ANN) with a genetic
algorithm to play the game snake. Below are instructions on how to run and
customize the model.


HOW TO RUN THE MODEL:

    If you have gcc installed, you can run the program with:

        make build run

    or 

        make all

    If you have another compiler installed, you can specify the compiler
    and run the program with: (clang compiler as an example)

        make CC=clang build run

    or 

        make CC=clang all

    *** NOTE: mac users with Xcode installed can use clang



HOW TO CUSTOMIZE THE MODEL:

    If you want to change the model's parameters, you can do so in the 
    "parameters" file located in the root directory. The program reads model 
    parameters by line with a keyword identifier (i.e. MUTATE, LAYER, etc.) and 
    ignores blank lines and "// " comments.

    To change the genetic algorithm parameters, you can simply change the 
    integer and double values already present. Beware of setting unreasonable values
    or unrecognized keywords as the program will automatically exit or crash.


    PARAMETER KEYWORDS:

        - POP_WIDTH: An integer that sets the population width, or number of members in a populations

        - GEN_COUNT: An integer that sets the number of generations a population will live on for

        - MUTATE: A double that sets the probability of a mutation occurring per gene

        - SURVIVE: A double that sets the percentage of population individuals that will live on to the
            next generation

        - LAYER: Two integers and a string (always sigmoid) that sets the number of inputs, neurons/outputs, 
            and activation per layer

        - THREADS: An integer that sets the number of threads the program will use with

        - REPLAY: An integer that sets the minimum number of apples a snake will have to eat before 
            the high-scoring replays will be shown (a value of 0 means no replay will be displayed)


    PARAMETER SET EXPLANATION/EXAMPLES:

    If you wanted to have a model that has 500 generations, 
    3500 snakes per generation, a mutation change of 1.5%, and a survival 
    chance of 20%, the first portion of the parameters file would look like:

        POP_WIDTH 3500
        GEN_COUNT 500
        MUTATE 0.015
        SURVIVE 0.2

    To change the neural network parameters, you can do so by adding, 
    removing, and adjusting lines with the LAYER keyword identifier. Each line
    that starts with LAYER will add a new layer to the neural network 
    (max is 6). Currently, the only supported activation is sigmoid. The 
    convention for adding a new layer is as follows:

        LAYER {num input} {num neurons/output} {activation}

    The number of inputs in a layer must be the same as the number of neurons/output
    in the previous layer. For this model, make sure that the number of inputs in the
    first layer is 24 and the number of neurons/outputs in the last layer is 4. 

    If you wanted the neural networks to have 3 layers that had
    a shape of 16 neurons in the first layer, 6 neurons in the second layer, and 
    4 neurons in the third layer, the second portion of the parameters file 
    would look like this:

        LAYER 24 16 sigmoid
        LAYER 16 8 sigmoid
        LAYER 8 4 sigmoid

    *** NOTE: notice that the first layer number of inputs is 24 and the last
                layer number of neurons is 4. The reason for this is that each
                neural network receives 24 numeric inputs about the environment
                and outputs the probability for moving in each of the 4 directions.
                The highest probability is chosen as the snake's next move.

    To change the number of threads used by the program, you can do so by changing
    the number following the THREADS flag. A value of 1 will run the program in a
    conventional/sequential fashion, while a number over 1 will utilize multi-
    threading.

    To turn the high-score replay functionality off, set the value following the 
    REPLAY flag to be 0. To see a replay, set the value to be anything greater 
    than 0. A non-zero value set will be the number of apples that the high-scoring
    replay will start at. 
    
    If you wanted the program to run on 12 threads and show a replay after a snake 
    eats 26 apples, the last section of the parameters file would look like this:

        THREADS 12
        REPLAY 26