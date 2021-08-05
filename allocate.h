/* allocate.h - contains some functions for dynamically allocating 2D arrays */
/* Ben Raphael 2/26/95 */
/* Modified by:  Pramod N. Achar 3/24/95 */
/* Bug fix by: Pramod N. Achar 3/28/95 */
#ifndef __ALLOCATE_H
#include <stdlib.h>
#include <stdio.h>
#define __ALLOCATE_H

void **Allocate_2D(int m, int n, size_t k)
     /*  Allocates an m x n array of "things" of size k.  For example,
	 to allocate a 40 x 40 array of integers called H, use 
	 H = (int **) Allocate_2D(40, 40, sizeof(int))  */
{
  int i;
  void *p, **a;

  p = (void *) calloc((size_t) (m*n), k);
  a = (void **) malloc(m*sizeof (void *));
  if ((p == NULL) || (a == NULL)) {
    fputs("Could not allocate memory for array!\n", stderr);
    exit(0);
  }
  for (i = 0; i < m; i++)
    a[i] = p + (i*n*k);
  return(a);
}

void Free_2D(void **a)
     /* Frees 2D array.  e.g., to free the integer array H, use
	Free_2D((void **) H) */
{
  void *p;

  p = (void *) a[0];
  free(p);
  free(a);
}


int **Allocate_Array(int m, int n)  /* allocates a m*n array of integers */
     /* Retained for backward compatibility */
{
  return((int **) Allocate_2D(m, n, sizeof(int)));
}

void Free_Array(int **a)  /* frees space allocated for matrix */
     /* Retained for backward compatibility */
{
  Free_2D((void **) a);
}

#endif
