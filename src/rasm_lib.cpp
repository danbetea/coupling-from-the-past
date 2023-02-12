#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <cstdio>
#include <cstring>
#include <random>

/// @brief Computes the ceiling of log base 2 of x
/// @param x an int
/// @return = (int) ceiling log2(x), where log2 is log base 2
/// e.g.: log2_int(17)=5, log2_int(16) = 4, log2_int(9)=4, log2_int(8)=3
int log2_int(int x);

/// @brief Checks if site (row, col) in the matrix can be flipped
/// @param matrix_ht an int matrix, the height function
/// @param row the row being checked
/// @param col the column being checked
/// @return true if site can be flipped
bool is_extreme(int** matrix_ht, const int row, const int col);

/// @brief Initializes the minimum and maximum height functions
/// @param minimum_ht the min height function
/// @param maximum_ht the max height function
/// @param n_rows number of rows of the height functions (same)
/// @param n_cols number of columns of the height functions (same)
void initialize_ht(int** minimum_ht, int** maximum_ht, 
                   const int n_rows, const int n_cols);

/// @brief Computes the volume difference between current min and max
/// height functions
/// @param minimum_ht the current min height function
/// @param maximum_ht the current max height function
/// @param n_rows number of rows of the height functions (same)
/// @param n_cols number of columns of the height functions (same)
/// @return the sum of the elements of the difference matrix
int volume_diff(int** minimum_ht, int** maximum_ht, 
                const int n_rows, const int n_cols);

/// @brief Returns a uniformly random +1 or -1 
/// @param rn_gen the random number generator
/// @return +1 or -1 uniformly at random
short random_pm1(std::mt19937& rn_gen);

/// @brief Evolves the height function by random flips whenever possible
/// @param minimum_ht the current min height function
/// @param maximum_ht the current max height function
/// @param n_rows number of rows of the height functions (same)
/// @param n_cols number of columns of the height functions (same)
/// @param rn_gen the random number generator
void evolve_ht(int** minimum_ht, int** maximum_ht, const int n_rows, 
               const int n_cols, std::mt19937& rn_gen);

/// @brief Runs the coupling from the past main loop
/// @param minimum_ht the min height function
/// @param maximum_ht the max height function
/// @param n_rows number of rows of the height functions (same)
/// @param n_cols number of columns of the height functions (same)
/// @param rn_gen the random number generator
/// @param seeds the seeds array for reseeding at each critical point
/// @param initial the number of initial steps to run the initial loop for
/// @param verbose bool for printing info to stderr
void run_cftp(int** minimum_ht, int** maximum_ht, const int n_rows, 
              const int n_cols, std::mt19937& rn_gen, const int seeds[256],
              const int initial, const bool verbose);

/// @brief Returns a random ASM after running coupling from the past
/// @param order the size for a (square) ASM
/// @param initial (int) number of steps to try at first, should be power of 2
/// @param verbose = false (default), bool for printing info to stderr
/// @return the random sample as a matrix pointer
int** sample_asm(const int order, int initial, const bool verbose);

// global variables needed for random number generation

// holds the last random number generated
int last_rand;
// holds the offset (which bit of last_rand) is being read
int offset; // at most 32 for 32-bit code

int** sample_asm(const int order, int initial = 128, 
                 const bool verbose = false) {

    // declare variables
    int count; 
    int seeds[256]; // seeds for coupling from the past
    int random_seed; // random seed
    // note height matrix is 1 bigger in each dimension than the desired ASM
    // for now deal only with square matrices
    int n_rows = order+1, n_cols = order+1; 

    if (1 << log2_int(initial) != initial) {
        initial = (1 << log2_int(initial));
        std::cerr << "Warning, initial is not a power of two. Increasing initial to " 
                  << initial << std::endl;
    }


    // TODO: make a check for a maximum order
    if (order < 1 ) {
        std::cerr << "Invalid order " << order << std::endl;
        exit(1);
    }

    // declare the min and max height functions
    // allocate memory using malloc, as it's more compatible with cython
    // TODO(dan): smart pointers

    // int** minimum_ht = new int*[n_rows];
    // int** maximum_ht = new int*[n_rows];
    // for(int i = 0; i < n_rows; i++) {
    //     minimum_ht[i] = new int[n_cols];
    //     maximum_ht[i] = new int[n_cols];
    // }
    int** minimum_ht = (int**)malloc(n_rows * sizeof(int*));
    int** maximum_ht = (int**)malloc(n_rows * sizeof(int*));
    for (int i = 0; i < n_rows; i++) {
        minimum_ht[i] = (int*)malloc(n_cols * sizeof(int));
        maximum_ht[i] = (int*)malloc(n_cols * sizeof(int));
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
    for (count = 0; count < 256; count ++) {
        std::uniform_int_distribution<> dist(-INT_MAX-1, INT_MAX);
        seeds[count] = dist(rn_gen);
    }

    // initialize min and max height functions
    initialize_ht(minimum_ht, maximum_ht, n_rows, n_cols);

    // run coupling from the past
    run_cftp(minimum_ht, maximum_ht, n_rows, n_cols, rn_gen, 
             seeds, initial, verbose);

    // deallocate memory for minimum_ht
    for(int i = 0; i < n_rows; i++) {
        // use free, as it's more compatible with Cython
        // delete [] minimum_ht[i];
        free(minimum_ht[i]);
    }
    // use free, for Cython reasons
    // delete [] minimum_ht;
    free(minimum_ht);

    // done, now return maximum_ht
    return maximum_ht;
}

int log2_int(int x) {
    // e.g.: log2_int(17)=5, log2_int(16) = 4, log2_int(9)=4, log2_int(8)=3
    int ans = 0;
    if (x)
        x --;
    while (x > 0) {
        x >>= 1;
        ans++;
    }
    return ans;
}

bool is_extreme(int** matrix_ht, const int row, const int col) {
    return (matrix_ht[row-1][col] == matrix_ht[row][col+1] &&
            matrix_ht[row][col+1] == matrix_ht[row+1][col] &&
            matrix_ht[row+1][col] == matrix_ht[row][col-1]);
}

void initialize_ht(int** minimum_ht, int** maximum_ht, const int n_rows, 
                   const int n_cols) {
    int row, col;
    for (row = 0; row < n_rows; row++)
        for (col = 0; col < n_cols; col++)
            minimum_ht[row][col] = std::abs((int) (row - col)) + 1;
    for (row = 0; row < n_rows; row++)
        for (col = 0; col < n_cols; col++)
            // TODO: change to allow for rectangular matrices
            // for now this assumes n_rows = n_cols
            maximum_ht[row][col] = n_rows - std::abs((int) (n_rows - col - row - 1));
}

int volume_diff(int** minimum_ht, int** maximum_ht, const int n_rows, 
                const int n_cols) {
    int diff = 0;
    int row, col;
    for (row = 0; row < n_rows; row++)
        for (col = 0; col < n_cols; col++)
        diff += (maximum_ht[row][col] - minimum_ht[row][col]);
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
        offset=0;
    }
    // read off the individual bits and increase the offset for next read
    return( (last_rand&(1<<offset++)) ? 1 : -1 );
}

void evolve_ht(int** minimum_ht, int** maximum_ht, 
               const int n_rows, const int n_cols, std::mt19937& rn_gen) {
    int row, col, phase;
    short coin_flip;

    // go through the height matrix
    // look for local extremes 
    // start at 1 and end at order - 1 to stay off boundaries
    // update where possible     
    for (phase = 0; phase < 2; phase ++)
        for (row = 1; row < n_rows - 1; row ++)
            for (col = 1; col < n_cols - 1; col ++)
                if ((row + col) % 2 == phase) {
                    coin_flip = random_pm1(rn_gen); // uniform random +1 or -1
                    if (is_extreme(minimum_ht, row, col))
                        minimum_ht[row][col] = minimum_ht[row-1][col] + coin_flip;
                    if (is_extreme(maximum_ht, row, col))
                        maximum_ht[row][col] = maximum_ht[row-1][col] + coin_flip;
                }
}

void run_cftp(int** minimum_ht, int** maximum_ht, const int n_rows, 
              const int n_cols, std::mt19937& rn_gen, const int seeds[256],
              const int initial, const bool verbose = false) {
    
    int step;
    std::time_t start, end; // for elapsed time

    if (verbose)
        start = std::clock(); // start the clock

    // we now run the coupling from the past main loop
    // starting from time = -initial all the way to time 0
    // and restarting with doubling time if it hasn't converged
    int time_steps = initial;
    while (volume_diff(minimum_ht, maximum_ht, n_rows, n_cols)) {
        step = time_steps;

        /* reset min and max heights */
        initialize_ht(minimum_ht, maximum_ht, n_rows, n_cols);

        int power_of_two = -2;

        // the main coupling from the past loop, runs for a power_of_two steps
        while (step > 0) {
            if (log2_int(step) != power_of_two) {
                power_of_two = log2_int(step);
                // declare and initialize random number generator 
                // with correct seeds so we use same randomness throughout
                rn_gen = std::mt19937(seeds[power_of_two]);
                offset = 32; // needed as we generate random bits 
                             // from random 32-bit ints
            }
            evolve_ht(minimum_ht, maximum_ht, n_rows, n_cols, rn_gen);
            step--;
        }
        time_steps *= 2;
    }
    if (verbose){
        std::cerr << "Random ASM of order " << n_rows-1 << " x " << n_cols-1
                    << " generated after " 
                    << time_steps / 2 << " steps." << std::endl;
        end = std::clock();
        double total_time = (double) (end - start)/CLOCKS_PER_SEC;
        std::fprintf(stderr, "It took %.4f seconds.\n", total_time);
    }
}