/**
   utilities-alg.h

   Implementations of general algorithms.
*/

#ifndef UTILITIES_ALG_H
#define UTILITIES_ALG_H

#include <stdlib.h>

/**
   Performs "greater or equal" binary search on an array with count
   elements, sorted in an ascending order according to cmp. The array
   is  pointed to by elts and has at least one element.

   Given an array A, finds A[i] <= element pointed to by key <= A[i + 1]
   according to cmp, and returns i + 1.

   If at least two indices satisfy the search objective, it is unspecified
   which index in the set of indices satisfying the search objective is
   returned. Returns count, if A[count - 1] < element pointed to by key.
*/
size_t geq_bsearch(const void *key,
		   const void *elts,
		   size_t count,
		   size_t elt_size,
		   int (*cmp)(const void *, const void *));

/**
   Performs "less or equal" binary search on an array with count elements,
   sorted in an ascending order according to cmp. The array is  pointed
   to by elts and has at least one element.

   Given an array A, finds A[i] <= element pointed to by key <= A[i + 1]
   according to cmp, and returns i.

   If at least two indices satisfy the search objective, it is unspecified
   which index in the set of indices satisfying the search objective is
   returned. Returns count, if A[0] > element pointed to by key.
*/
size_t leq_bsearch(const void *key,
		   const void *elts,
		   size_t count,
		   size_t elt_size,
		   int (*cmp)(const void *, const void *));

#endif
