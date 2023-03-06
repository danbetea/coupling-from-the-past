#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <cstdio>
#include <cstring>
#include <random>
#include <cmath>
#include "rasm.h"

extern int offset;

int main(int argc, char **argv) {
    /*
    -----------------
    declare variables
    -----------------
    */

    int order, n_rows, n_cols; // order/size of ASM, num rows and num cols
    int count; 
    // const options for command line arguments for displaying ASM
    enum cmd_options {ASM = 2, HEIGHT = 3, CSUM = 4, ASM_F = 5};
    int output = ASM; // default option for printing to stdout
    // various options for command line
    bool min_only = false, max_only = false, use_random = true, report = false;
    int seeds[256]; // seeds for coupling from the past
    int initial = 128, random_seed; // initial no. of steps to try, random seed


    /*
    ----------------------------
    process command line options
    ----------------------------
    */


    if(argc < 2)
        print_options();

    if(!strcmp(argv[1],"-help"))
        print_options();

    // read the order
    order = std::stoi(argv[1]); // sscanf(argv[1],"%d", &order); also works

    // TODO: make a check for a maximum order
    if(order < 1) {
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
    int **minimum_ht = new int*[n_rows];
    int **maximum_ht = new int*[n_rows];
    for(int i=0; i<n_rows; ++i) {
        minimum_ht[i] = new int[n_cols];
        maximum_ht[i] = new int[n_cols];
    }

    if(argc > 2)
        for(count=2; count<argc; ++count) {
            if (!strcmp(argv[count],"-asm"))
                output = ASM;
            else if (!strcmp(argv[count],"-asm_file"))
                output = ASM_F;
            else if (!strcmp(argv[count],"-csum"))
                output = CSUM;
            else if(!strcmp(argv[count], "-height"))
                output = HEIGHT;
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
            else if(!strcmp(argv[count],"-initial")) {
                if(count == argc - 1) {
                    std::cerr << "You must specify an initial number of steps.\n";
                    exit(1);
                }
                initial = std::stoi(argv[count+1]);
                if (initial < 1 || initial > 536870912) {
                    std::cerr << "Invalid value for initial; it must be between 1 and 2^29 = 536870912 \n";
                    exit(1);
                }
                ++count;
                if(1 << log2_int(initial) != initial) {
                    initial = (1 << log2_int(initial));
                    std::cerr << "Warning, initial is not a power of two. Increasing initial to " 
                              << initial << std::endl;
                }
            }
            else if(!strcmp(argv[count],"-help"))
                print_options();
            else {
                std::cerr << "Illegal command line argument " << argv[count] << std::endl;
                print_options();
            }
        }

    // initialize min and max height functions
    initialize_ht(minimum_ht, maximum_ht, n_rows, n_cols);

    // print min or max ht function if so desired
    if (min_only) {
        if(output == ASM)
            print_asm(minimum_ht, n_rows, n_cols);
        else if(output == CSUM)
            print_csum(minimum_ht, n_rows, n_cols);
        else
            print_ht(minimum_ht, n_rows, n_cols);
        exit(0);
    }
    else if(max_only) {
        if(output == ASM)
            print_asm(maximum_ht, n_rows, n_cols);
        else if(output == CSUM)
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
    for (count = 0; count < 256; ++count) {
        std::uniform_int_distribution<> dist(-INT_MAX-1, INT_MAX);
        seeds[count] = dist(rn_gen);
    }


    /*
    -----------------------------
    do the coupling form the past
    -----------------------------
    */


    run_cftp(minimum_ht, maximum_ht, n_rows, n_cols,
             rn_gen, seeds, initial, report, true);


    /*
    ------------------------------
    done, process output and clean
    ------------------------------
    */


    if(output == ASM) 
        print_asm(maximum_ht, n_rows, n_cols);
    else if(output == ASM_F)
        print_asm_to_file(maximum_ht, n_rows, n_cols);
    else if(output == CSUM)
        print_csum(maximum_ht, n_rows, n_cols);
    else
        print_ht(maximum_ht, n_rows, n_cols);

    // std::cerr<<std::endl;

    // deallocate memory
    for(int i=0; i<n_rows; ++i) {
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
    std::cout << "   -height           output the corresponding height function\n";
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

void print_ht(int **matrix_ht, const int n_rows, const int n_cols) {
    int row, col;
    // the max entry and its number of digits (formatting purposes)
    int max_entry = (int) (std::max(n_rows, n_cols));
    int num_digits = ((int) std::floor(std::log10(max_entry)))+1;
    for (row = 0; row < n_rows; ++row) {
        for (col = 0; col < n_cols; ++col)
            std::printf("%*d ", num_digits, matrix_ht[row][col]);
        std::printf("\n");
    }
}

void print_csum(int **matrix_ht, const int n_rows, const int n_cols) {
    int row, col;
    // the max entry and its number of digits (formatting purposes)
    int max_entry = (n_rows + n_cols - matrix_ht[n_rows-1][n_cols-1]) / 2;
    int num_digits = ((int) std::floor(std::log10(max_entry))) + 1;
    for (row = 0; row < n_rows; ++row) {
        for (col = 0; col < n_cols; ++col)
            std::printf("%*d", num_digits+1, (row + col + 2 - matrix_ht[row][col])/2);
        std::printf("\n");
    }
}

void print_asm(int **matrix_ht, const int n_rows, const int n_cols) {
    int row, col;
    // start at 1, because we're reading the ASM from the 
    // + 1 bigger size height function
    for (row = 1; row < n_rows; ++row) {
        for (col = 1; col < n_cols; ++col)
            std::printf("%2d ",(matrix_ht[row-1][col] + matrix_ht[row][col-1] 
                        - matrix_ht[row][col] - matrix_ht[row-1][col-1]) / 2);
        std::printf("\n");
    }
}

void print_asm_to_file(int **matrix_ht, const int n_rows, const int n_cols) {
    int row, col;
    FILE *fptr1;
    FILE *fptr2;
    fptr1 = fopen("asm_pretty.txt", "w+");
    fptr2 = fopen("asm.txt", "w+");

    if(fptr1 == NULL || fptr2 == NULL) {
        std::cerr << "File error!";
        std::exit(1);
    }

    for (row = 1; row < n_rows; ++row) {
        for (col = 1; col < n_cols; ++col){
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

