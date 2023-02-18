#ifndef RASM
#define RASM

#include "rasm_lib.h"

/// @brief Prints the options available at the command line
void print_options();

/// @brief Prints the height function to stdout
/// @param matrix_ht an int matrix, the height function
/// @param n_rows number of rows of matrix_ht
/// @param n_cols number of columns of matrix_ht
void print_ht(int **matrix_ht, const int n_rows, const int n_cols);

/// @brief Prints the corner sum matrix to stdout
/// @param matrix_ht an int matrix, the height function
/// @param n_rows number of rows of matrix_ht
/// @param n_cols number of columns of matrix_ht
void print_csum(int **matrix_ht, const int n_rows, const int n_cols);

/// @brief Prints the ASM to stdout
/// @param matrix_ht an int matrix, the height function
/// @param n_rows number of rows of matrix_ht
/// @param n_cols number of columns of matrix_ht
void print_asm(int **matrix_ht, const int n_rows, const int n_cols);

/// @brief Prints the ASM to two files 
/// @param matrix_ht an int matrix, the height function
/// @param n_rows number of rows of matrix_ht
/// @param n_cols number of columns of matrix_ht
void print_asm_to_file(int **matrix_ht, const int n_rows, const int n_cols);

#endif
