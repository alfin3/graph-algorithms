/**
   utilities-alg.c

   Implementations of general algorithms.
*/

#include <stdio.h>
#include <stdlib.h>
#include "utilities-mem.h"

static void *elt_ptr(const void *elts, size_t i, size_t elt_size);

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
		   int (*cmp)(const void *, const void *)){
  size_t cur = (count - 1) / 2;
  size_t  prev_low = 0, prev_high = count - 1;
  if (cmp(key, elt_ptr(elts, prev_low, elt_size)) <= 0) return 0;
  if (cmp(key, elt_ptr(elts, prev_high, elt_size)) > 0) return count;
  while (1){
    /* a. A[0] < key element <= A[count - 1], A's count > 1 */
    if (cmp(key, elt_ptr(elts, cur, elt_size)) < 0){
      prev_high = cur;
      /* b. must decrease each time, since cur is not prev_low */
      cur = (cur + prev_low) / 2;
    }else if (cmp(key, elt_ptr(elts, cur + 1, elt_size)) > 0){
      prev_low = cur;
      /* c. must increase each time, since cur + 1 is not prev_high */
      cur = (cur + prev_high) / 2; /* < count - 1 */
    }else{
      /* must enter due to a, b, and c, at the latest after step size is 1 */
      break;
    }
  }
  return cur + 1;
}

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
		   int (*cmp)(const void *, const void *)){
  size_t cur;
  if (cmp(key, elt_ptr(elts, 0, elt_size)) < 0){
    /* key element < A[0], thus no element in A <= key element */
    return count;
  }
  cur = geq_bsearch(key, elts, count, elt_size, cmp);
  if (cur == 0){
    /* key element == A[0] */
    return 0;
  }else{
    return cur - 1;
  }
}

/**
   Computes a pointer to the ith element in an array pointed to by elts.
*/
static void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
}
