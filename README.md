# coupling_from_the_past
Generates uniformly random alternating sign matrices and boxed plane partitions using coupling from the past (the Propp--Wilson algorithm).  

The main files are and do:  
  --- squareice.c creates a uniformly random alternating sign matrix (ASM) using the Propp--Wilson (coupling from the past) Markov Chain Monte Carlo (MCMC) algorithm. Most of the code is not mine, see attributions at the beginning of the file. There are multiple ways of outputting the ASM. A somewhat smaller version of the same code exists on Ben Wieland's page at http://nokedli.net/asm-frozen/  
  --- random_asm.py is a thin wrapper of the above written in python, for people who don't want to compile C code themselves or want to interface with an 'easier' language.   
  --- bpp.c creates a uniformly random boxed plane partition (tiling of a hexagon) using the same Propp--Wilson MCMC algorithm.  
