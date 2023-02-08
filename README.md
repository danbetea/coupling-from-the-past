# Random alternating sign matrices (and other) via coupling from the past

## Introduction

This repo generates (for now) uniformly random alternating sign matrices using the Markov Chain Monte Carlo method of coupling from the past (the Propp--Wilson algorithm).  

The main files so far in the `src` directory are and do as follows:  
  - `rasm_basic.cpp` which generates a uniformly random alternating sign matrix (ASM); it has multiple ways of outputting the resulting ASM. For similar stuff written in an old pre-99 version of C, see [Ben Wieland's page](http://nokedli.net/asm-frozen/);
  - `rasm_basic.py` is a thin wrapper of the above written in Python, for people who don't want to compile C code themselves or want to interface with an 'easier' language.   
  
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
    >```python3 rasm_basic.py 15```

## Note

From a software engineering point of view, the file `rasm_basic.cpp` is rather ugly, containing everything in one go. While I do plan some improvements to it, it will always contain everything inside. The reason is very basic: coupling from the past is a rather tricky algorithm, and so having everything in one file is there for pedagogical reasons. If C++ had a decent notebook interface, I would use a notebook instead (and perhaps I will in the future).


## References
 
- Häggström, O. *Finite Markov chains and algorithmic applications*, Cambridge University Press, 2002
- Levin, D., Peres, Y., with a contribution by Wilmer, E., *Markov chains and mixing times*, second edition, American Mathematical Society, [link to pdf](https://pages.uoregon.edu/dlevin/MARKOV/markovmixing.pdf)
- [Wikipedia page for alternating sign matrices](https://en.wikipedia.org/wiki/Alternating_sign_matrix)
- [Wikipedia page for coupling from the past](https://en.wikipedia.org/wiki/Coupling_from_the_past)
