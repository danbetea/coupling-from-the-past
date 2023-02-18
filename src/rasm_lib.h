#ifndef RASM_LIB
#define RASM_LIB

// random number generator
typedef std::mt19937 RNG;

/// @brief Returns a random ASM after running coupling from the past
/// @param order the size for a (square) ASM
/// @param initial (int) number of steps to try at first, should be power of 2
/// @param verbose = false (default), bool for printing info to stderr
/// @return the random sample as a matrix pointer
int **sample_asm(const int order, int initial, const bool verbose);

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
bool is_extreme(int **matrix_ht, const int row, const int col);

/// @brief Initializes the minimum and maximum height functions
/// @param minimum_ht the min height function
/// @param maximum_ht the max height function
/// @param n_rows number of rows of the height functions (same)
/// @param n_cols number of columns of the height functions (same)
void initialize_ht(int **minimum_ht, int **maximum_ht, 
                   const int n_rows, const int n_cols);

/// @brief Computes the volume difference between current min and max
/// height functions
/// @param minimum_ht the current min height function
/// @param maximum_ht the current max height function
/// @param n_rows number of rows of the height functions (same)
/// @param n_cols number of columns of the height functions (same)
/// @return the sum of the elements of the difference matrix
int volume_diff(int **minimum_ht, int **maximum_ht, 
                const int n_rows, const int n_cols);

/// @brief Returns a uniformly random +1 or -1 
/// @param rn_gen the random number generator
/// @return +1 or -1 uniformly at random
short random_pm1(RNG& rn_gen);

/// @brief Evolves the height function by random flips whenever possible
/// @param minimum_ht the current min height function
/// @param maximum_ht the current max height function
/// @param n_rows number of rows of the height functions (same)
/// @param n_cols number of columns of the height functions (same)
/// @param rn_gen the random number generator
void evolve_ht(int **minimum_ht, int **maximum_ht, const int n_rows, 
               const int n_cols, RNG& rn_gen);

/// @brief Runs the coupling from the past main loop
/// @param minimum_ht the min height function
/// @param maximum_ht the max height function
/// @param n_rows number of rows of the height functions (same)
/// @param n_cols number of columns of the height functions (same)
/// @param rn_gen the random number generator
/// @param seeds the seeds array for reseeding at each critical point
/// @param initial the number of initial steps to run the initial loop for
/// @param report a bool for verbose progress report
void run_cftp(int **minimum_ht, int **maximum_ht, const int n_rows, 
              const int n_cols, RNG& rn_gen, const int seeds[256],
              const int initial, const bool report, const bool timing);

#endif
