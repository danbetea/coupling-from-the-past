"""
A thin wrapper around fast coupling from the past, implemented in C++ in the file rasm_basic.cpp

-h for help

Example usage:
    :: generate a 15 by 15 uniform random alternating sign matrix ::
    python random_asm.py 15

(C) Dan Betea 2017--2023

License: MIT License
"""

import subprocess, os, sys, getopt

def rand_asm(n):
    cmd_exec = ["./rasm_basic", str(n),"-asm"]
    try:
        result = subprocess.Popen(cmd_exec, stdout=subprocess.PIPE, stderr=open('/dev/null','w'))
        # convert to a list of strings
        result = result.stdout.read().decode("utf-8").split('\n')
        # get the matrix
        matrix = [[int(c) for c in row.split(" ") if c not in ['', ' ']] for row in result[:-1]]
    except OSError:
        sys.stderr.write("Running program for the first time. Compiling C++ code...\n")
        # note: using call, as it ends for the completion of the compilation
        cmd_compile = ['g++', '-Ofast', 'rasm_basic.cpp', '-o', 'rasm_basic']
        result_compile = subprocess.call(cmd_compile, stderr=open('/dev/null','w'))
        sys.stderr.write("End of compilation.\n")
        # return a bytes object
        result = subprocess.Popen(cmd_exec, stdout=subprocess.PIPE, stderr=open('/dev/null','w'))
        # convert to a list of strings
        result = result.stdout.read().decode("utf-8").split('\n')
        # get the matrix
        matrix = [[int(c) for c in row.split(" ") if c not in ['', ' ']] for row in result[:-1]]
    
    return matrix

def main(argv=None):
    if argv is None:
        argv = sys.argv
    opts, args = getopt.getopt(argv[1:], "h", ["help"])
    for o, a in opts:
        if o in ("-h", "--help"):
            print(__doc__)
            sys.exit(0)
    # process arguments
    n = int(args[0])
    asm = rand_asm(n)
    print('\n'.join([''.join(
        [f"{asm[row][col]:1d}" if col == 0 else f"{asm[row][col]:3d}" 
        for col in range(n)]) 
        for row in range(n)]))

if __name__ == "__main__":
    sys.exit(main())
