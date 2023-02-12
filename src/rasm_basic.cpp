#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <cstdio>
#include <cstring>
#include <random>

#define HEIGHT 2
#define ASM    3
#define ASM_F  4  // ASM written to file
#define CSUM   5  // Corner sum matrix

/// @brief Prints the options available at the command line
void print_options();

/// @brief Computes the ceiling of log base 2 of x
/// @param x an int
/// @return = (int) ceiling log2(x), where log2 is log base 2
/// e.g.: log2_int(17)=5, log2_int(16) = 4, log2_int(9)=4, log2_int(8)=3
int log2_int(int x);

/// @brief Prints the height function to stdout
/// @param matrix_ht an int matrix, the height function
/// @param n_rows number of rows of matrix_ht
/// @param n_cols number of columns of matrix_ht
void print_ht(int** matrix_ht, const int n_rows, const int n_cols);

/// @brief Prints the corner sum matrix to stdout
/// @param matrix_ht an int matrix, the height function
/// @param n_rows number of rows of matrix_ht
/// @param n_cols number of columns of matrix_ht
void print_csum(int** matrix_ht, const int n_rows, const int n_cols);

/// @brief Prints the ASM to stdout
/// @param matrix_ht an int matrix, the height function
/// @param n_rows number of rows of matrix_ht
/// @param n_cols number of columns of matrix_ht
void print_asm(int** matrix_ht, const int n_rows, const int n_cols);

/// @brief Prints the ASM to two files 
/// @param matrix_ht an int matrix, the height function
/// @param n_rows number of rows of matrix_ht
/// @param n_cols number of columns of matrix_ht
void print_asm_to_file(int** matrix_ht, const int n_rows, const int n_cols);

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
/// @param report a bool for verbose progress report
void run_cftp(int** minimum_ht, int** maximum_ht, const int n_rows, 
              const int n_cols, std::mt19937& rn_gen, const int seeds[256],
              const int initial, const bool report);

// global variables needed for random number generation

// holds the last random number generated
int last_rand;
// holds the offset (which bit of last_rand) is being read
int offset; // at most 32 for 32-bit code


int main(int argc, char **argv) {


    /*
    -----------------
    declare variables
    -----------------
    */

    int order, n_rows, n_cols; // order/size of ASM, num rows and num cols
    int count; 
    int output = HEIGHT; // default option for printing to stdout
    // various options for command line
    bool min_only = false, max_only = false, use_random = true, report = false;
    int seeds[256]; // seeds for coupling from the past
    int initial = 128, random_seed; // initial no. of steps to try, random seed


    /*
    ----------------------------
    process command line options
    ----------------------------
    */


    if (argc < 2)
        print_options();

    if (!strcmp(argv[1],"-help"))
        print_options();

    // read the order
    order = std::stoi(argv[1]); // sscanf(argv[1],"%d", &order); also works

    // TODO: make a check for a maximum order
    if (order < 1 ) {
        std::cerr << "Invalid order " << order << std::endl;
        exit(1);
    }

    // deal only with square matrices for now
    // since working with height functions, they are 
    // +1 more in dimensions than the actual ASM
    n_rows = order + 1;
    n_cols = order + 1; 

    // declare the min and max height functions
    // allocate memory
    int** minimum_ht = new int*[n_rows];
    int** maximum_ht = new int*[n_rows];
    for(int i = 0; i < n_rows; i++) {
        minimum_ht[i] = new int[n_cols];
        maximum_ht[i] = new int[n_cols];
    }

    if (argc > 2)
        for (count = 2; count < argc; count++) {
            if (!strcmp(argv[count],"-asm"))
                output = ASM;
            else if (!strcmp(argv[count],"-asm_file"))
                output = ASM_F;
            else if (!strcmp(argv[count],"-csum"))
                output = CSUM;
            else if (!strcmp(argv[count],"-report"))
                report = true;
            else if (!strcmp(argv[count],"-min_only"))
                min_only = true;
            else if (!strcmp(argv[count],"-max_only"))
                max_only = true;
            else if (!strcmp(argv[count],"-seed")) {
                if (count == argc - 1) {
                    std::cerr << "You must specify a seed.\n";
                    exit(1);
                }
                // sscanf(argv[count+1],"%d",&random_seed); also works
                random_seed = std::stoi(argv[count+1]); 
                use_random = false;
                count++;
            }
            else if (!strcmp(argv[count],"-initial")) {
                if (count == argc - 1) {
                    std::cerr << "You must specify an initial number of steps.\n";
                    exit(1);
                }
                initial = std::stoi(argv[count+1]);
                if (initial < 1 || initial > 536870912) {
                    std::cerr << "Invalid value for initial; it must be between 1 and 2^29 = 536870912 \n";
                    exit(1);
                }
                count++;
                if (1 << log2_int(initial) != initial) {
                    initial = (1 << log2_int(initial));
                    std::cerr << "Warning, initial is not a power of two. Increasing initial to " 
                              << initial << std::endl;
                }
            }
            else if (!strcmp(argv[count],"-help"))
                print_options();
            else {
                std::cerr << "Illegal command line argument " << argv[count] << std::endl;
                print_options();
            }
        }

    // print min or max ht function if so desired
    if (min_only) {
        if (output == ASM)
            print_asm(minimum_ht, n_rows, n_cols);
        else if (output == CSUM)
            print_csum(minimum_ht, n_rows, n_cols);
        else
            print_ht(minimum_ht, n_rows, n_cols);
        exit(0);
    }
    else if (max_only) {
        if (output == ASM)
            print_asm(maximum_ht, n_rows, n_cols);
        else if (output == CSUM)
            print_csum(maximum_ht, n_rows, n_cols);
        else
            print_ht(maximum_ht, n_rows, n_cols);
        exit(0);
    }


    /*
    -------------------------------------
    initialize seeds and height functions
    -------------------------------------
    */
    

    // create a random seed to be used just below
    if (use_random) {
        // std::cerr << "\nGenerating random seed to start everything..." 
        //           << std::endl;
        std::random_device rd; // use to seed the rng 
        std::mt19937 rng0(rd()); // rng
        std::uniform_int_distribution<> dist0(-INT_MAX-1, INT_MAX);
        random_seed = dist0(rng0);
    }

    std::cerr << "Using random seed " << random_seed << ".\n";

    // initialize the random number generator used throughout
    // note: as soon as the main loop starts running, it will be reinitialized
    // this initialization is then only used to generate 256 random seeds used
    // in the coupling from the past construction down the line
    std::mt19937 rn_gen(random_seed);
    offset = 32; // technically not needed here

    // get 256 seeds, to be used by the random number generator in the
    // coupling from the past main loop
    for (count = 0; count < 256; count++) {
        std::uniform_int_distribution<> dist(-INT_MAX-1, INT_MAX);
        seeds[count] = dist(rn_gen);
    }

    // initialize min and max height functions
    initialize_ht(minimum_ht, maximum_ht, n_rows, n_cols);


    /*
    -----------------------------
    do the coupling form the past
    -----------------------------
    */


    run_cftp(minimum_ht, maximum_ht, n_rows, n_cols, 
             rn_gen, seeds, initial, report);


    /*
    ------------------------------
    done, process output and clean
    ------------------------------
    */


    if (output == ASM) 
        print_asm(maximum_ht, n_rows, n_cols);
    else if (output == ASM_F)
        print_asm_to_file(maximum_ht, n_rows, n_cols);
    else if (output == CSUM)
        print_csum(maximum_ht, n_rows, n_cols);
    else
        print_ht(maximum_ht, n_rows, n_cols);

    // std::cerr<<std::endl;

    // deallocate memory
    for(int i = 0; i < n_rows; i++) {
        delete [] minimum_ht[i];
        delete [] maximum_ht[i];
    }
    delete [] minimum_ht;
    delete [] maximum_ht;

    return 0;
}

void print_options() {
    std::cout << std::endl;
    std::cout << "Usage for this program (don't type the '$'): \n";
    std::cout << std::endl;
    std::cout << "   $ ./rasm_basic order [options]\n";
    std::cout << std::endl;
    std::cout << "where order is an integer > 0 and [options] are:\n";
    std::cout << std::endl;
    std::cout << "   -asm              output the alternating sign matrix\n";
    std::cout << "   -asm_file         output the alternating sign matrix to files asm.txt and asm_pretty.txt\n";
    std::cout << "   -csum             output the corresponding corner sum matrix\n";
    std::cout << "   -seed <value>     use a specific random seed\n";
    std::cout << "   -initial <value>  use a specific initial value\n";
    std::cout << "   -report           give a progress report\n";
    std::cout << "   -min_only         only output the minimum square ice\n";
    std::cout << "   -max_only         only output the maximum square ice\n";
    std::cout << "   -help             give a listing of command line arguments\n";
    std::cout << std::endl;
    std::cout << "Example: \n\n";
    std::cout << "   $ ./rasm_basic 301 -asm_file -initial 262144\n\n";
    std::cout << "is optimized for fast generation of 300x300 ASMs dumped to a file. \n\n";

    std::exit(1);
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

void print_ht(int** matrix_ht, const int n_rows, const int n_cols) {
    int row, col;
    for (row = 0; row < n_rows; row++) {
        for (col = 0; col < n_cols; col++)
            std::printf("%2d ",matrix_ht[row][col]);
        std::printf("\n");
    }
}

void print_csum(int** matrix_ht, const int n_rows, const int n_cols) {
    int row, col;
    for (row = 0; row < n_rows; row++) {
        for (col = 0; col < n_cols; col++)
            std::printf("%2d ",(row + col + 2 - matrix_ht[row][col])/2);
        std::printf("\n");
    }
}

void print_asm(int** matrix_ht, const int n_rows, const int n_cols) {
    int row, col;
    // start at 1, because we're reading the ASM from the 
    // + 1 bigger size height function
    for (row = 1; row < n_rows; row++) {
        for (col = 1; col < n_cols; col++)
            std::printf("%2d ",(matrix_ht[row-1][col] + matrix_ht[row][col-1] 
                        - matrix_ht[row][col] - matrix_ht[row-1][col-1]) / 2);
        std::printf("\n");
    }
}

void print_asm_to_file(int** matrix_ht, const int n_rows, const int n_cols) {
    int row, col;
    FILE *fptr1;
    FILE *fptr2;
    fptr1 = fopen("asm_pretty.txt", "w+");
    fptr2 = fopen("asm.txt", "w+");

    if(fptr1 == NULL || fptr2 == NULL) {
        std::cerr << "File error!";
        std::exit(1);
    }

    for (row = 1; row < n_rows; row++) {
        for (col = 1; col < n_cols; col++){
            int entry = (int) (matrix_ht[row-1][col] + matrix_ht[row][col-1] 
                       - matrix_ht[row][col] - matrix_ht[row-1][col-1]) / 2;
            if (entry == 0) {
                std::fprintf(fptr1, "%s", "  ");
                std::fprintf(fptr2, (col == 1) ? "%d" : "%3d", entry);
            } 
            else if (entry == -1) {
                std::fprintf(fptr1, "%s", "- ");
                std::fprintf(fptr2, (col == 1) ? "%d" : "%3d", entry);
            } 
            else {
                std::fprintf(fptr1, "%s", "+ ");
                std::fprintf(fptr2, (col == 1) ? "%d" : "%3d", entry);
            }
        }
        std::fprintf(fptr1, "\n");
        std::fprintf(fptr2, "\n");
    }
    std::fclose(fptr1);
    std::fclose(fptr2);
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
    for (phase = 0; phase < 2; phase++)
        for (row = 1; row < n_rows - 1; row++)
            for (col = 1; col < n_cols - 1; col++)
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
              const int initial, const bool report) {
    
    int step;
    std::time_t start, end; // for elapsed time

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

                if (report)
                    std::cerr << "Using max number of steps " << time_steps 
                        << " and difference in volume at time " 
                        << step << " is " 
                        << volume_diff(maximum_ht, minimum_ht, n_rows, n_cols) 
                        << std::endl;
            }
            evolve_ht(minimum_ht, maximum_ht, n_rows, n_cols, rn_gen);
            step--;
        }

        if (report)
            std::cerr << "Volume of difference at time 0 is " 
                      << volume_diff(maximum_ht, minimum_ht, n_rows, n_cols)
                      << std::endl;

        time_steps *= 2;
    }
    std::cerr << "Random ASM of order " << n_rows-1 << " x " << n_cols-1
                << " generated after " 
                << time_steps / 2 << " steps." << std::endl;
    end = std::clock();
    double total_time = (double) (end - start)/CLOCKS_PER_SEC;
    std::fprintf(stderr, "It took %.4f seconds.\n", total_time);
}