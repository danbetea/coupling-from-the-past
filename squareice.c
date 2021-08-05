/***************************************************************************
 * squareice.c


 * version 1.7                                                             *
 * Written by Matthew Blum and Jason Woolever 2-28-97                      *
 * last changed 01-14-07 by Ben Wieland                                    *
 * that changed the output, the mapping from seeds to asms.                *
 *                                                                         *
 * squareice creates a random height function of a square ice.  It can     *
 * also output a random alternating sign matrix.                           *
 *                                                                         *
 * squareice uses the Propp-Wilson algorithm in coupling the minimum and   *
 * maximum height functions until they converge.                           *
 *                                                                         *
 * Remember to compile with the r250.c random number generator             *
 * gcc squareice.c r250.c -O -o squareice                                  *
 *                                                                         *
 *                                                                         *
 * modified by Dan Betea, December 2016                                    *
 * (added routines to send random asm and the corresponding corner sum     *
 * matrix to an external file, for easier plotting)                        *
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

void drawheight(int **matrix, int order);
/* draws the height function */

void drawasm(int **matrix, int order);
/* draws the corresponding alternating sign matrix for the height function on the screen */

void drawasm_to_file(int **matrix, int order);
/* draws the corresponding ASM for the height function, in output.txt */

void drawcsum(int **matrix, int order);
/* draws the corresponding corner sum matrix */

void createmin(int **matrix, int order);
/* creates the minimum height function */

void createmax(int **matrix, int order);
/* creates the maximum height function */

void evolve(int **minimum, int **maximum, int order);
/* changes local extremes in minimum and maximum so that min and max couple */

int extreme(int **matrix, int row, int col);
/* returns TRUE if (row,col) is a local extreme in matrix */

int different(int **minimum, int **maximum, int order);
/* returns TRUE if minimum and maximum differ */

void printerror(void);
/* prints a listing of command line options */

int main(int argc, char **argv) {
  int **minimum, **maximum;
  int order, count, step, powertwo, timesteps, output = HEIGHT;
  int minonly = FALSE, maxonly = FALSE;
  int seeds[256], initial = 128, randomseed, userandom = TRUE, report = FALSE;
  clock_t start, end;
  double cpu_time_used; // for time used

  if (argc < 2)
    printerror();

  if (!strcmp(argv[1],"-help"))
    printerror();

  sscanf(argv[1],"%d", &order);
  //if (order < 1 || order > 1000) {
  if (order < 1 ) {
    printf("Invalid order %d.\n", order);
    exit(1);
  }

  /* do some simple command line argument processing */
  if (argc > 2)
    for (count = 2; count < argc; count ++) {
      if (!strcmp(argv[count],"-asm"))
	      output = ASM;
      else if (!strcmp(argv[count],"-asmfile"))
        output = ASMF;
      else if (!strcmp(argv[count],"-csum"))
        output = CSUM;
      else if (!strcmp(argv[count],"-report"))
	      report = TRUE;
      else if (!strcmp(argv[count],"-minonly"))
	      minonly = TRUE;
      else if (!strcmp(argv[count],"-maxonly"))
	      maxonly = TRUE;
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

  minimum = Allocate_Array(order, order);
  maximum = Allocate_Array(order, order);

  createmin(minimum, order);
  createmax(maximum, order);

  if (minonly) {
    if (output == ASM)
      drawasm(minimum, order);
    else if (output == CSUM)
        drawcsum(minimum, order);
    else
      drawheight(minimum, order);
    exit(0);
  }
  else if (maxonly) {
    if (output == ASM)
      drawasm(maximum, order);
    else if (output == CSUM)
      drawcsum(maximum, order);
    else
      drawheight(maximum, order);
    exit(0);
  }

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
  while (different(maximum, minimum, order)) {
    step = timesteps;

    /* reset minimum and maximum to the min and max height functions */
    createmin(minimum, order);
    createmax(maximum, order);

    powertwo = -2;
    while (step > 0) {
      if (log2(step) != powertwo) {
	      powertwo = log2(step);
	      //r250_init(seeds[powertwo]);
	      my_r250_init(seeds[powertwo]);
	      if (report)
	        fprintf(stderr, "Using maxsteps %d, volume of difference at time -%d is %d.\n",
		    timesteps, step, different(maximum, minimum, order));
      }
      evolve(minimum, maximum, order);
      step --;
    }
    if (report)
      fprintf(stderr,"Volume of difference at time 0 is %d\n",
	      different(maximum,minimum,order));
    timesteps *= 2;
  }

  /* we're done!! whoo-hoo! */
  fprintf(stderr, "Random ice generated after %d time steps.\n", timesteps / 2);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  fprintf(stderr, "It took %d seconds.\n", (int) cpu_time_used);
  if (output == ASM)
    drawasm(maximum, order);
  else if (output == ASMF)
    drawasm_to_file(maximum, order);
  else if (output == CSUM)
    drawcsum(maximum, order);
  else
    drawheight(maximum, order);

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
void drawheight(int **matrix, int order) {
  int row, col;
  for (row = 0; row < order; row ++) {
    for (col = 0; col < order; col ++)
      printf("%2d ",matrix[row][col]);
    printf("\n");
  }
}

/**************************************************************************
  DRAW THE CORNER SUM MATRIX  ************************************************************************/
void drawcsum(int **matrix, int order) {
  int row, col;
  for (row = 0; row < order; row ++) {
    for (col = 0; col < order; col ++)
      printf("%2d ",(row + col + 2 - matrix[row][col])/2);
    printf("\n");
  }
}

/**************************************************************************
  DRAW THE ALTERNATING SIGN MATRIX
  ************************************************************************/
void drawasm(int **matrix, int order) {
  int row, col;
  for (row = 1; row < order; row ++) {
    for (col = 1; col < order; col ++)
      printf("%2d ",(matrix[row-1][col] + matrix[row][col-1] -
		     matrix[row][col] - matrix[row-1][col-1]) / 2);
    printf("\n");
  }
}

/**************************************************************************
  DRAW THE ALTERNATING SIGN MATRIX TO FILE asm.txt
  ************************************************************************/
void drawasm_to_file(int **matrix, int order) {
  int row, col;

  FILE *fptr1;
  FILE *fptr2;

  fptr1 = fopen("asm_pretty.txt", "w+");
  fptr2 = fopen("asm.txt", "w+");

  if(fptr1 == NULL)
  {
     printf("Error!");
     exit(1);
  }

  if(fptr2 == NULL)
  {
     printf("Error!");
     exit(1);
  }

  for (row = 1; row < order; row ++) {
    for (col = 1; col < order; col ++){
      int entry = (int) (matrix[row-1][col] + matrix[row][col-1] - matrix[row][col] - matrix[row-1][col-1]) / 2;
      if (entry == 0) {
        fprintf(fptr1, "%s", "  ");
        fprintf(fptr2, "%2d", entry);
      }
      else if (entry == -1) {
        fprintf(fptr1, "%s", "- ");
        fprintf(fptr2, "%2d", entry);
      }
      else {
        fprintf(fptr1, "%s", "+ ");
        fprintf(fptr2, "%2d", entry);
      }
    }
    fprintf(fptr1, "\n");
    fprintf(fptr2, "\n");
  }
  fclose(fptr1);
  fclose(fptr2);
}

/**************************************************************************
  CREATE THE MINIMUM AND MAXIMUM HEIGHT FUNCTIONS
  ************************************************************************/
void createmin(int **matrix, int order) {
  int row, col;
  for (row = 0; row < order; row ++)
    for (col = 0; col < order; col ++)
      matrix[row][col] = abs(row-col) + 1;
}



void createmax(int **matrix, int order) {
  int row, col;
  for (row = 0; row < order; row ++)
    for (col = 0; col < order; col ++)
      matrix[row][col] = order - abs(order - col - row - 1);
}

/**************************************************************************
  EVOLVE THE MINIMUM AND MAXIMUM HEIGHT FUNCTIONS
  ************************************************************************/
void evolve(int **minimum, int **maximum, int order) {
  int row, col, phase;
  int coinflip;

  /* go through the array and look for local extremes */
  /* start at 1 and end at order - 1 to stay off boundaries */
  for (phase = 0; phase < 2; phase ++)
    for (row = 1; row < order - 1; row ++)
      for (col = 1; col < order - 1; col ++) {
	      if ((row + col) % 2 == phase) {
	        /* set coinflip randomly to +1 or -1 */
	        coinflip = my_r250_bit() * 2 - 1;
	        //coinflip = r250n(2) * 2 - 1;

	        if (extreme(minimum,row,col))
	          minimum[row][col] = minimum[row-1][col] + coinflip;
	        if (extreme(maximum,row,col))
	          maximum[row][col] = maximum[row-1][col] + coinflip;
	      }
      }
}

/**************************************************************************
  SEE IF MINIMUM AND MAXIMUM HEIGHT FUNCTIONS DIFFER
  ************************************************************************/
int different(int **minimum, int **maximum, int order) {
  int retval = 0;
  int row, col;
  for (row = 0; row < order; row ++)
    for (col = 0; col < order; col ++)
      retval = retval + minimum[row][col] - maximum[row][col];
  return retval;
}

/**************************************************************************
  LOOK FOR LOCAL EXTREMES
  ************************************************************************/
int extreme(int **matrix, int row, int col) {
  return (matrix[row-1][col] == matrix[row][col+1] &&
	  matrix[row][col+1] == matrix[row+1][col] &&
	  matrix[row+1][col] == matrix[row][col-1]);
}

/**************************************************************************
  LOOK FOR LOCAL EXTREMES
  ************************************************************************/
void printerror() {
  printf("Usage for squareice: squareice order [options]\n");
  printf("where [options] are:\n");
  printf("   -asm              output the alternating sign matrix\n");
  printf("   -asmfile          output the alternating sign matrix to asm.txt\n");
  printf("   -csum             output the corresponding corner sum matrix\n");
  printf("   -seed <value>     use a specific random seed\n");
  printf("   -initial <value>  use a specific initial value\n");
  printf("   -report           give a progress report\n");
  printf("   -minonly          only output the minimum square ice\n");
  printf("   -maxonly          only output the maximum square ice\n");
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
