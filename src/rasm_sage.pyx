# distutils: language = c++

from libc.stdlib cimport malloc, free
from libcpp cimport bool

# import C++ sampling routine
cdef extern from "rasm_lib.cpp":
    int** sample_asm(int order, int initial, bool verbose)

def ht_to_asm(ht_fn):
    """
    Converts a height function to an ASM

    Input:
    ht_fn: list[list[int]] -- the height function

    Returns:
    asm:   list[list[int]] -- the alternating sign matrix
    """

    num_rows = len(ht_fn)
    num_cols = len(ht_fn[0])
    asm = [[0 for __ in range(num_rows-1)] for _ in range(num_cols-1)]
    for row in range (1, num_rows):
        for col in range(1, num_cols):
            asm[row-1][col-1] = (ht_fn[row-1][col] + ht_fn[row][col-1] 
                                 - ht_fn[row][col] - ht_fn[row-1][col-1]) // 2
    return asm

cpdef rasm(order, initial=128, verbose=False):
    """
    Samples a random alternating sign matrix of square size given by order

    Inputs:
    order: int    -- the size of the desired ASM
    initial: int  -- the initial number of steps to start with for CFTP
    verbose: bool -- whether to print information to the console regarding 
                     time taken, etc.

    Returns: 
    list[list[int]] -- the alternating sign matrix, order x order
    """

    # declare height function, allocate memory    
    # needed (I think) in case new and delete are used on the CPP side
    # if ASM is order x order, height function is (order+1) x (order+1)
    # cdef int ** ht_fn = <int **> malloc((order+1) * sizeof(int*))
    # cdef int i
    # for i in range(order+1):
    #     ht_fn[i] = <int *> malloc((order+1) * sizeof(int));

    # declare height fn, do the sampling
    cdef int ** ht_fn = sample_asm(order, initial, verbose)

    # save answer in Python object
    height = [[0 for __ in range(order+1)] for _ in range(order+1)]
    for i in range(order+1):
        for j in range(order+1):
            height[i][j] = ht_fn[i][j]

    # deallocate memory (note ht_fn is +1 per dim bigger than the ASM)
    for i in range(order+1):
        free(ht_fn[i]);
    free(ht_fn);

    # done, return ASM
    return ht_to_asm(height)

def pprint_asm(asm, symbols=""):
    """
    Prints the ASM to screen using correct spacing either as a 0 1 -1 matrix
    (i.e. an alternatign sign matrix) or using two symbols for +1 and -1
    omitting the zeros

    Inputs:
    asm: list[list[int]] -- the ASM
    symbols: str         -- either empty or having at least 2 symbols for +1/-1

    Returns: None
    """
    num_rows = len(asm)
    num_cols = len(asm[0])

    if not symbols:
        print('\n'.join([''.join(
            [f"{asm[row][col]:1d}" if col == 0 else f"{asm[row][col]:3d}" 
            for col in range(num_cols)]) 
            for row in range(num_rows)]))
    else:
        assert len(symbols) >= 2, "Two symbols needed, one for 1, one for -1"
        sp1 = f"{symbols[0]} " # symbol for +1 and a space
        sm1 = f"{symbols[1]} " # symbol for -1 and a space
        print('\n'.join([''.join(
            ["  " if asm[row][col] == 0 else (sp1 if asm[row][col] < 0 else sm1) 
            for col in range(num_cols)]) 
            for row in range(num_rows)]))