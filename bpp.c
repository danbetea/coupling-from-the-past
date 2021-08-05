/***************************************************************************
 * bpp.c

 * Written by Dan Betea, based on previous work by
 * Matthew Blum and Jason Woolever (97) and Ben Wieland (07)               *
 *                                                                         *
 * bpp creates a random height function of a boxed plane partition,        *
 * with floor in an a x b box (a = # lines, b = # columns)                 *
 * and maximal part = c. An example with a=4, b=5, c=9 is given below:     *
 *                                                                         *
        8 7 7 6 5 4
        7 7 7 6 5 3
        6 5 5 5 4 1
        4 3 0 0 0 0
 *                                                                         *
 * bpp uses the Propp-Wilson algorithm in coupling the minimum and         *
 * maximum height functions until they converge.                           *
 *                                                                         *
 * Remember to compile with the r250.c random number generator             *
 * gcc bpp.c r250.c -O -o bpp                                              *
 *                                                                         *
 **************************************************************************/

#include <stdio.h>
#include <time.h>
#include "allocate.h"
#include "r250.h"
#define FALSE  0
#define TRUE   1
#define HEIGHT 2
#define ASM    3
#define ASMF   4  // ASM written to file
#define CSUM   5  // Corner sum matrix




int offset;
unsigned int last_rand;
void my_r250_init( int seed );
int my_r250_bit( void );
/* global variables + functions wrapping r250 random number
 * generator. call r250() and get 16 bits at a time, rather than
 * calling r250n(2) to get a single bit.
 * done for speed, since 1/3 of time was spent in r250 */

int log2(int x);
/* compute log base two */

void drawheight(int **matrix, int a, int b);
/* draws the height function */

void createmin(int **matrix, int a, int b, int c);
/* creates the minimum height function */

void createmax(int **matrix, int a, int b, int c);
/* creates the maximum height function */

void evolve(int **minimum, int **maximum, int a, int b, int c);
/* changes local extremes in minimum and maximum so that min and max couple */

int flippable_up(int **matrix, int row, int col, int a, int b, int c);
/* returns TRUE if a cube (+1) can be added at (row,col)  */

int flippable_down(int **matrix, int row, int col, int a, int b, int c);
/* returns TRUE if a cube (-1) can be removed at (row,col)  */

int different(int **minimum, int **maximum, int a, int b);
/* returns TRUE if minimum and maximum differ */

void printerror(void);
/* prints a listing of command line options */

int main(int argc, char **argv) {
  int **minimum, **maximum;
  // a = # rows, b = # columns, c = maximal part
  int a, b, c, count, step, powertwo, timesteps, output = HEIGHT;
  int minonly = FALSE, maxonly = FALSE;
  int seeds[256], initial = 128, randomseed, userandom = TRUE, report = FALSE;
  clock_t start, end;
  double cpu_time_used; // for time used

  if (argc < 4)
    printerror();

  if (!strcmp(argv[1],"-help"))
    printerror();

  sscanf(argv[1],"%d", &a);
  sscanf(argv[2],"%d", &b);
  sscanf(argv[3],"%d", &c);

  //if (order < 1 || order > 1000) {
  if (a < 1 || b < 1 || c < 1) {
    printf("Invalid dimensions: %d x %d with maximal part %d.\n", a, b, c);
    exit(1);
  }

  /* do some simple command line argument processing */
  if (argc > 4)
    for (count = 4; count < argc; count ++) {
      if (!strcmp(argv[count],"-report"))
	      report = TRUE;
      else if (!strcmp(argv[count],"-seed")) {
	      if (count == argc - 1) {
	        printf("Must specify seed.\n");
	        exit(1);
	      }
	      sscanf(argv[count+1],"%d",&randomseed);
	      userandom = FALSE;
	      count ++;
        }
      else if (!strcmp(argv[count],"-initial")) {
	      if (count == argc - 1) {
	        printf("Must specify initial value.\n");
	        exit(1);
	      }
	      sscanf(argv[count+1],"%d",&initial);
	      if (initial < 1 || initial > 100000000) {
	        printf("Invalid initial value %d.\n", initial);
	        exit(1);
	      }
	      count ++;
	      if (1 << log2(initial) != initial) {
	        initial = (1 << log2(initial));
	        fprintf(stderr,"Warning, initial is not a power of two.  Increasing initial to %d.\n", initial);
	      }
      }
      else if (!strcmp(argv[count],"-help"))
	    printerror();
      else {
	      printf("Illegal command line argument %s.\n", argv[count]);
	      printerror();
      }
    }


  start = clock();

  minimum = Allocate_Array(a, b);
  maximum = Allocate_Array(a, b);

  createmin(minimum, a, b, c);
  createmax(maximum, a, b, c);

  /* create random seeds */
  if (userandom)
    randomseed = time(0) * getpid();
  r250_init(randomseed);
  /* this section uses r250_init and r250.
   * elsewhere, I use my wrappers.
   * it's not legit to mix them, but they're separated here */

  fprintf(stderr,"Using random seed %d.\n", randomseed);

  for (count = 0; count < 256; count ++) {
    /* call r250 twice since each call only returns a 16 bit number and
       we need a 32 bit seed */
    seeds[count] = r250() * 65536 + r250();
  }

  /* do the Propp-Wilson algorithm for coupling */
  timesteps = initial;
  while (different(maximum, minimum, a, b)) {
    step = timesteps;

    /* reset minimum and maximum to the min and max height functions */
    createmin(minimum, a, b, c);
    createmax(maximum, a, b, c);

    powertwo = -2;
    while (step > 0) {
      if (log2(step) != powertwo) {
	      powertwo = log2(step);
	      //r250_init(seeds[powertwo]);
	      my_r250_init(seeds[powertwo]);
	      if (report)
	        fprintf(stderr, "Using maxsteps %d, volume of difference at time -%d is %d.\n",
		    timesteps, step, different(maximum, minimum, a, b));
      }
      evolve(minimum, maximum, a, b, c);
      step --;
    }
    if (report)
      fprintf(stderr,"Volume of difference at time 0 is %d\n",
	      different(maximum, minimum, a, b));
    timesteps *= 2;
  }

  /* we're done!! whoo-hoo! */
  fprintf(stderr, "Boxed plane partition generated after %d time steps.\n", timesteps / 2);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  fprintf(stderr, "It took %d seconds.\n", (int) cpu_time_used);

  drawheight(maximum, a, b);

  /* clean exit */
  exit(0);
  return 0;
}

/**************************************************************************
  COMPUTE BASE 2 LOGARITHM
  e.g.: log2(17)=5, log2(16) = 4, log2(9)=4, log2(8)=3
  ************************************************************************/
int log2(int x) {
  int ans = 0;
  if (x)
    x --;
  while (x > 0) {
    x >>= 1;
    ans++;
  }
  return ans;
}

/**************************************************************************
  DRAW THE HEIGHT FUNCTION
  ************************************************************************/
void drawheight(int **matrix, int a, int b) {
  int row, col;
  for (row = 0; row < a; row ++) {
    for (col = 0; col < b; col ++)
      printf("%2d ",matrix[row][col]);
    printf("\n");
  }
}


/**************************************************************************
  CREATE THE MINIMUM AND MAXIMUM HEIGHT FUNCTIONS
  ************************************************************************/
void createmin(int **matrix, int a, int b, int c) {
  int row, col;
  for (row = 0; row < a; row ++)
    for (col = 0; col < b; col ++)
      matrix[row][col] = 0;
}



void createmax(int **matrix, int a, int b, int c) {
  int row, col;
  for (row = 0; row < a; row ++)
    for (col = 0; col < b; col ++)
      matrix[row][col] = c;
}

/**************************************************************************
  EVOLVE THE MINIMUM AND MAXIMUM HEIGHT FUNCTIONS
  ************************************************************************/
void evolve(int **minimum, int **maximum, int a, int b, int c) {
  int row, col, phase;
  int coinflip;

  /* go through the array and look for local extremes */
  for (row = 0; row < a; row ++)
    for (col = 0; col < a; col ++) {
      /* set coinflip randomly to +1 or -1 */
      coinflip = my_r250_bit() * 2 - 1;

      if (coinflip == 1){
        if (flippable_up(minimum, row, col, a, b, c))
          minimum[row][col] += coinflip;
        if (flippable_up(maximum, row, col, a, b, c))
          maximum[row][col] += coinflip;
      }
      else {
        if (flippable_down(minimum, row, col, a, b, c))
          minimum[row][col] += coinflip;
        if (flippable_down(maximum, row, col, a, b, c))
          maximum[row][col] += coinflip;
      }
    }
}

/**************************************************************************
  SEE IF MINIMUM AND MAXIMUM HEIGHT FUNCTIONS DIFFER
  ************************************************************************/
int different(int **minimum, int **maximum, int a, int b) {
  int retval = 0;
  int row, col;
  for (row = 0; row < a; row ++)
    for (col = 0; col < b; col ++)
      retval = retval + maximum[row][col] - minimum[row][col];
  return retval;
}

/**************************************************************************
  CHECK IF WE CAN ADD A BOX
  ************************************************************************/
int flippable_up(int **matrix, int row, int col, int a, int b, int c) {
  int top, left;

  if (row==0)
    top = c;
  else
    top = matrix[row-1][col];

  if (col==0)
    left = c;
  else
    left = matrix[row][col-1];

  return (top >= matrix[row][col]+1 && left >= matrix[row][col]+1);
}

/**************************************************************************
  CHECK IF WE CAN REMOVE A BOX
  ************************************************************************/
int flippable_down(int **matrix, int row, int col, int a, int b, int c) {
  int bottom, right;
  if (row==a-1)
    bottom = 0;
  else
    bottom = matrix[row+1][col];

  if (col==b-1)
    right = 0;
  else
    right = matrix[row][col+1];

  return (matrix[row][col]-1 >= right && matrix[row][col]-1 >= bottom);
}

/**************************************************************************
  LOOK FOR LOCAL EXTREMES
  ************************************************************************/
void printerror() {
  printf("Usage for bpp: squareice a b c [options]\n");
  printf("where the floor is of size a x b, the maximal part is c and\n");
  printf("where [options] are:\n");
  printf("   -seed <value>     use a specific random seed\n");
  printf("   -initial <value>  use a specific initial value\n");
  printf("   -report           give a progress report\n");
  printf("   -help             give a listing of command line arguments\n");
  exit(1);
}

void my_r250_init( int seed ) {
  r250_init(seed);
  offset=16;
}

int my_r250_bit( void ) {
  if( offset==16 ) {
    last_rand=r250();
    offset=0;
  }
  return( (last_rand&(1<<offset++)) ? 1 : 0 );
}
