# Random alternating sign matrices (and other) via coupling from the past

## Idea

The ideas behind this repo are:
 - to have a *basic fast modern* implementation of the [coupling from the past exact random sampling algorithm](https://en.wikipedia.org/wiki/Coupling_from_the_past) of Propp and Wilson;
 - to have (so far an ugly but) a pedagogical example of said algorithm, the reason being that the algorithm and its implementations can become rather subtle rather quickly (see the book by Häggström in the references), and that very little in terms of implementation is available in the public domain.

## Introduction

This repo generates (for now) uniformly random [alternating sign matrices](https://en.wikipedia.org/wiki/Alternating_sign_matrix) using the Markov Chain Monte Carlo method of coupling from the past (the Propp--Wilson algorithm).  

The main files so far in the `src` directory are and do as follows:  
  - `rasm_basic.cpp` which generates a uniformly random alternating sign matrix (ASM); it has multiple ways of outputting the resulting ASM;
  - `rasm_basic.py` is a thin wrapper of the above written in Python, for people who don't want to compile C++ code themselves or want to interface with an 'easier' language;
  - `rasm_lib.cpp` is a library meant to be bound with Cython (e.g. there is no main function);
  - `rasm_sage.pyx` is a Cython binding doing the random sampling; it can be used from sage.
  
For more on coupling from the past and alternating sign matrices, see the references below. 
  
## Usage

For basic usage:

 - compile with:
```g++ -Ofast rasm_basic.cpp -o rasm_basic```
 - use as:
    > ```./rasm_basic -help```
    >
    > ```./rasm_basic 10 -asm```
    >
    > ```./rasm_basic 100 -asm_file```
    >
    > ```./rasm_basic 1000 -asm_file -initial 4194304``` (optimized for sampling a size 1000 ASM, takes about 8-10 hours)
    >
    > ```python3 rasm_basic.py 15```

If you have [sagemath](https://www.sagemath.org) installed, or just [Cython](https://cython.readthedocs.io/en/latest/#), you can try this from within the REPL to generate a 10 x 10 random ASM once you've started the REPL from within the ```src``` directory (example below is for sage):

 - ```sage: %runfile rasm_sage.pyx```
 - ```sage: a = rasm(10, initial=2^7, verbose=True)```
 - ```sage: pprint_asm(a, symbols="")```
 - ```sage: pprint_asm(a, symbols="+-")```
 - for example:
   ```
   > sage: %runfile rasm_sage.pyx
   > Compiling ./rasm_sage.pyx...
   > sage: a = rasm(10, initial=2^7, verbose=True)
   > Random ASM of order 10 x 10 generated after 128 steps.
   > Elapsed time: 0.0002 seconds.
   > sage: pprint_asm(a, symbols="+x")
   >                  +   
   >          +           
   >      +               
   >        + x +         
   >    + x   + x +       
   >  + x           +     
   >      +   x +     x + 
   >    +                 
   >          +   x   +   
   >              +  
   > sage: pprint_asm(a)
   > 0  0  0  0  0  0  0  0  1  0
   > 0  0  0  0  1  0  0  0  0  0
   > 0  0  1  0  0  0  0  0  0  0
   > 0  0  0  1 -1  1  0  0  0  0
   > 0  1 -1  0  1 -1  1  0  0  0
   > 1 -1  0  0  0  0  0  1  0  0
   > 0  0  1  0 -1  1  0  0 -1  1
   > 0  1  0  0  0  0  0  0  0  0
   > 0  0  0  0  1  0 -1  0  1  0
   > 0  0  0  0  0  0  1  0  0  0
   ```

## Note

From a software engineering point of view, the file `rasm_basic.cpp` is rather ugly, containing everything in one file. While I do plan some improvements to it, it will always contain everything inside. The reason is very basic: coupling from the past is a rather tricky algorithm, and so having everything in one file is there for pedagogical reasons. If C++ had a decent notebook interface, I would use a notebook instead (and perhaps I will in the future).

## References
 
- Häggström, O. *Finite Markov chains and algorithmic applications*, Cambridge University Press, 2002
- Levin, D., Peres, Y., with a contribution by Wilmer, E., *Markov chains and mixing times*, second edition, American Mathematical Society, [link to pdf](https://pages.uoregon.edu/dlevin/MARKOV/markovmixing.pdf)
- [Wikipedia page for alternating sign matrices](https://en.wikipedia.org/wiki/Alternating_sign_matrix)
- [Wikipedia page for coupling from the past](https://en.wikipedia.org/wiki/Coupling_from_the_past)
- Wilson, D., *Perfectly random sampling with Markov chains*, [author's webpage](http://www.dbwilson.com/exact/)
