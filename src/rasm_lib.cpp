#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <cstdio>
#include <cstring>
#include <random>
#include "rasm_lib.h"

// global variables needed for random number generation

// holds the last random number generated
int last_rand;
// holds the offset (which bit of last_rand) is being read
int offset; // at most 32 for 32-bit code

int **sample_asm(const int order, int initial=128, const bool verbose=false) {
    // declare variables
    int count;
    int seeds[256]; // seeds for coupling from the past
    int random_seed; // random seed
    // note height matrix is 1 bigger in each dimension than the desired ASM
    // for now deal only with square matrices
    int n_rows=order+1, n_cols=order+1;

    if(1 << log2_int(initial) != initial) {
        initial = (1 << log2_int(initial));
        std::cerr << "Warning, initial is not a power of two. Increasing initial to "
                  << initial << std::endl;
    }


    // TODO: make a check for a maximum order
    if(order < 1) {
        std::cerr << "Invalid order " << order << std::endl;
        //exit(1);
        return NULL;
    }

    // declare the min and max height functions
    // TODO(dan): smart pointers

    int **minimum_ht = new int*[n_rows];
    int **maximum_ht = new int*[n_rows];
    for(int i=0; i<n_rows; ++i) {
        minimum_ht[i] = new int[n_cols];
        maximum_ht[i] = new int[n_cols];
    }

    // create a random seed to be used just below
    std::random_device rd; // use to seed the rng
    std::mt19937 rng0(rd()); // rng
    std::uniform_int_distribution<> dist0(-INT_MAX-1, INT_MAX);
    random_seed = dist0(rng0);

    // initialize the random number generator used throughout
    // note: as soon as the main loop starts running, it will be reinitialized
    // this initialization is then only used to generate 256 random seeds used
    // in the coupling from the past construction down the line
    std::mt19937 rn_gen(random_seed);
    offset = 32; // technically not needed here

    // get 256 seeds, to be used by the random number generator in the
    // coupling from the past main loop
    for(count=0; count<256; ++count) {
        std::uniform_int_distribution<> dist(-INT_MAX-1, INT_MAX);
        seeds[count] = dist(rn_gen);
    }

    // run coupling from the past
    run_cftp(minimum_ht, maximum_ht, n_rows, n_cols, rn_gen,
             seeds, initial, verbose, false);

    // deallocate memory for minimum_ht
    for(int i = 0; i < n_rows; ++i)
        delete [] minimum_ht[i];
    delete [] minimum_ht;

    // done, now return maximum_ht
    return maximum_ht;
}

int log2_int(int x) {
    // e.g.: log2_int(17)=5, log2_int(16) = 4, log2_int(9)=4, log2_int(8)=3
    int ans = 0;
    if (x)
        --x;
    while (x > 0) {
        x >>= 1;
        ++ans;
    }
    return ans;
}

inline bool is_extreme(int **matrix_ht, const int row, const int col) {
    return (matrix_ht[row-1][col] == matrix_ht[row][col+1] &&
            matrix_ht[row][col+1] == matrix_ht[row+1][col] &&
            matrix_ht[row+1][col] == matrix_ht[row][col-1]);
}

void initialize_ht(int **minimum_ht, int **maximum_ht, const int n_rows,
                   const int n_cols) {
    int row, col;
    int value;
    for(row=0; row<n_rows; ++row) {
        /* for(col=0; col<n_cols; ++col)
            minimum_ht[row][col] = std::abs((int) (row - col)) + 1;*/
        // Optimization assuming a square matrix
        for(col=0; col<row; ++col) {
            minimum_ht[row][col] = (row - col) + 1;
            minimum_ht[col][row] = (row - col) + 1;

            // TODO: change to allow for rectangular matrices
            // for now this assumes n_rows = n_cols
            value = n_rows - std::abs((int) (n_rows - col - row - 1));
            maximum_ht[row][col] = value;
            maximum_ht[col][row] = value;
        }
        minimum_ht[row][row] = 1;
        maximum_ht[row][row] = n_rows - std::abs((int) (n_rows - 2*row - 1));
    }
}

// This function could be eleminated by having this as a variable
//   and modifying it in evolve_ht().
// However, this is not called very often, so it doesn't contribute much to the timings.
int volume_diff(int **minimum_ht, int **maximum_ht, const int n_rows,
                const int n_cols) {
    int diff = 0;
    for(int row=0; row<n_rows; ++row) {
        for(int col=0; col<n_cols; ++col)
        diff += (maximum_ht[row][col] - minimum_ht[row][col]);
    }
    return diff;
}

// int random_pm1(std::mt19937& rn_gen){
//     std::bernoulli_distribution dist(0.5);
//     int coin_flip = dist(rn_gen) ? 1 : -1;
//     // std::cout << "generated " << coin_flip << std::endl;
//     return coin_flip;
// }

short random_pm1(std::mt19937& rn_gen) {
    // generates a 32 bit random uint and then reads off its bits one
    // by one for faster speed
    if(offset == 32) {
        std::uniform_int_distribution<unsigned int> dist(0, UINT_MAX);
        last_rand = dist(rn_gen);
        offset = 0;
    }
    // read off the individual bits and increase the offset for next read
    return (last_rand & (1 << offset++)) ? 1 : -1;
}

void evolve_ht(int **minimum_ht, int **maximum_ht,
               const int n_rows, const int n_cols, std::mt19937& rn_gen) {

    short coin_flip;

    // go through the height matrix
    // look for local extremes
    // start at 1 and end at order - 1 to stay off boundaries
    // update where possible

    for(int phase=0; phase<2; ++phase) {
        for(int row=1; row<n_rows-1; ++row) {
            for(int col=(row%2==phase ? 2 : 1); col<n_cols-1; col+=2) {
                // invariant: (row + col) % 2 == phase
                /* if((row+col)%2 != phase)
                    std::cout<<"bad row and col"<<row<<", "<<col<<std::endl; */
                coin_flip = random_pm1(rn_gen); // uniform random +1 or -1
                if(is_extreme(minimum_ht, row, col))
                    minimum_ht[row][col] = minimum_ht[row-1][col] + coin_flip;
                if(is_extreme(maximum_ht, row, col))
                    maximum_ht[row][col] = maximum_ht[row-1][col] + coin_flip;
            }
        }
    }
}

void run_cftp(int **minimum_ht, int **maximum_ht, const int n_rows,
              const int n_cols, std::mt19937 &rn_gen, const int seeds[256],
              const int initial, const bool report, const bool timing) {

    int step;
    std::time_t start, end; // for elapsed time

    if(timing)
        start = std::clock(); // start the clock

    // we now run the coupling from the past main loop
    // starting from time = -initial all the way to time 0
    // and restarting with doubling time if it hasn't converged
    int time_steps = initial;
    while(volume_diff(minimum_ht, maximum_ht, n_rows, n_cols)) {
        step = time_steps;

        /* reset min and max heights */
        initialize_ht(minimum_ht, maximum_ht, n_rows, n_cols);

        int power_of_two = -2;

        // the main coupling from the past loop, runs for a power_of_two steps
        while(step > 0) {
            if(log2_int(step) != power_of_two) {
                power_of_two = log2_int(step);
                // declare and initialize random number generator
                // with correct seeds so we use same randomness throughout
                rn_gen = std::mt19937(seeds[power_of_two]);
                offset = 32; // needed as we generate random bits
                             // from random 32-bit ints

                if(report)
                    std::cerr << "Using max number of steps " << time_steps
                        << " and difference in volume at time "
                        << step << " is "
                        << volume_diff(maximum_ht, minimum_ht, n_rows, n_cols)
                        << std::endl;
            }
            evolve_ht(minimum_ht, maximum_ht, n_rows, n_cols, rn_gen);
            --step;
        }

        if(report)
            std::cerr << "Volume of difference at time 0 is "
                      << volume_diff(maximum_ht, minimum_ht, n_rows, n_cols)
                      << std::endl;

        time_steps *= 2;
    }
    if(timing) {
        std::cerr << "Random ASM of order " << n_rows-1 << " x " << n_cols-1
                    << " generated after "
                    << time_steps / 2 << " steps." << std::endl;
        end = std::clock();
        double total_time = (double) (end - start)/CLOCKS_PER_SEC;
        std::fprintf(stderr, "Elapsed time: %.4f seconds.\n", total_time);
    }
}
