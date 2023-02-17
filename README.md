# Random alternating sign matrices (and other) via monotone coupling from the past

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

### Basic usage

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

### Usage with SageMath or Cython

If you have [SageMath](https://www.sagemath.org) installed, or just [Cython](https://cython.readthedocs.io/en/latest/#), you can try the following examples from within the REPL to generate a 10 x 10 random ASM once you've started the REPL from within the ```src``` directory (example below is for sage):

- ```sage: %runfile rasm_sage.pyx```
- ```sage: a = rasm(10, initial=int(2 ** 7), verbose=True)```
- ```sage: pprint_asm(a)```
- ```sage: pprint_asm(a, symbols="+-")```
- for example:

   ```sage
   sage: %runfile rasm_sage.pyx
   Compiling ./rasm_sage.pyx...
   sage: a = rasm(10, initial=int(2 ** 7), verbose=True)
   Random ASM of order 10 x 10 generated after 128 steps.
   Elapsed time: 0.0002 seconds.
   sage: pprint_asm(a, symbols="+x")
                    +   
            +           
        +               
          + x +         
      + x   + x +       
    + x           +     
        +   x +     x + 
      +                 
            +   x   +   
                +  
   sage: pprint_asm(a)
   0  0  0  0  0  0  0  0  1  0
   0  0  0  0  1  0  0  0  0  0
   0  0  1  0  0  0  0  0  0  0
   0  0  0  1 -1  1  0  0  0  0
   0  1 -1  0  1 -1  1  0  0  0
   1 -1  0  0  0  0  0  1  0  0
   0  0  1  0 -1  1  0  0 -1  1
   0  1  0  0  0  0  0  0  0  0
   0  0  0  0  1  0 -1  0  1  0
   0  0  0  0  0  0  1  0  0  0
   ```

## Timing

Below are some basic running times (for the main algorithm only, not the wall time) on a 2015 Macbook Pro Retina 13" with 8GB of RAM. Note the optimizations using the `-initial` flag.

```bash
 % ./rasm_basic 100 -asm_file
Using random seed 1050614381.
Random ASM of order 100 x 100 generated after 16384 steps.
Elapsed time: 3.7235 seconds.

 % ./rasm_basic 100 -asm_file -initial 16384
Random ASM of order 100 x 100 generated after 16384 steps.
Elapsed time: 1.8513 seconds.

 % ./rasm_basic 100 -asm_file -initial 16384
Using random seed -13878880.
Random ASM of order 100 x 100 generated after 32768 steps.
Elapsed time: 5.3178 seconds.

 % ./rasm_basic 100 -asm_file -initial 32768
Using random seed 462488753.
Random ASM of order 100 x 100 generated after 32768 steps.
Elapsed time: 3.4711 seconds.
```

Finally, here is a comparison with the Python implementation in sage. The speed-up seems on the order of 200-300x. Again note the optimizations using the `initial` argument.

```sage
sage: %timeit AlternatingSignMatrices(30).random_element()
7.35 s ± 1.22 s per loop (mean ± std. dev. of 7 runs, 1 loop each)
sage: %runfile rasm_sage.pyx
Compiling ./rasm_sage.pyx...
sage: %timeit rasm(30, initial=int(2 ** 9))
28.1 ms ± 3.63 ms per loop (mean ± std. dev. of 7 runs, 10 loops each)
sage: %timeit rasm(30, initial=int(2 ** 10))
21.8 ms ± 1.6 ms per loop (mean ± std. dev. of 7 runs, 100 loops each)
sage: %timeit rasm(30, initial=int(2 ** 11))
22.6 ms ± 2.9 ms per loop (mean ± std. dev. of 7 runs, 10 loops each)
sage: %timeit rasm(30, initial=int(2 ** 12))
39.9 ms ± 207 µs per loop (mean ± std. dev. of 7 runs, 10 loops each)
sage: 7350 / 21.8
337.155963302752
```

## Note

From a software engineering point of view, the file `rasm_basic.cpp` is rather ugly, containing everything in one file. While I do plan some improvements to it, it will always contain everything inside. The reason is very basic: coupling from the past is a rather tricky algorithm, and so having everything in one file is there for pedagogical reasons. If C++ had a decent notebook interface, I would use a notebook instead (and perhaps I will in the future).

## References

Below are a few references, at various levels of technicality, for the description above.

- Häggström, O. *Finite Markov chains and algorithmic applications*, Cambridge University Press, 2002
- Levin, D., Peres, Y., with a contribution by Wilmer, E., *Markov chains and mixing times*, second edition, American Mathematical Society, [link to pdf](https://pages.uoregon.edu/dlevin/MARKOV/markovmixing.pdf)
- [Wikipedia page for alternating sign matrices](https://en.wikipedia.org/wiki/Alternating_sign_matrix)
- [Wikipedia page for coupling from the past](https://en.wikipedia.org/wiki/Coupling_from_the_past)
- Wilson, D., *Perfectly random sampling with Markov chains*, [author's webpage](http://www.dbwilson.com/exact/)
