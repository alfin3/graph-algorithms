/**
   mergesort-pthread.c

   Functions for optimizing and running a generic merge sort algorithm
   with parallel sorting and parallel merging.

   The design decouples merge and sort parallelisms in mergesort. The
   algorithm provides \Theta(n/log^{2}n) theoretical parallelism within the
   dynamic multithreading model.

   The implementation provides i) a set of parameters for setting the
   constant base case upper bounds for switching from parallel sorting to
   serial sorting and from parallel merging to serial merging during
   recursion, and ii) a macro for setting the constant upper bound for the
   number of recursive calls placed on the stack of a thread across sorting
   and merging operations, thereby enabling the optimization of the
   parallelism and concurrency-associated overhead across input ranges
   and hardware settings. The provided parametrization of multithreading
   is based on the common parametrization of serial mergesort, where the
   recursion depth is limited by switching to a base case non-recursive
   sort algorithm.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient resources.
   The behavior outside the specified parameter ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99. The requirement is that pthreads API is available.

   On a 4-core machine, the optimization of the base case upper bound
   parameters resulted in a speedup of approximately 2.6X in comparison
   to serial qsort (stdlib.h) on arrays of 10M random integer or double
   elements.
*/

#define _XOPEN_SOURCE 600

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mergesort-pthread.h"
#include "utilities-alg.h"
#include "utilities-mem.h"
#include "utilities-pthread.h"

struct mergesort_arg{
  size_t p, r;
  size_t sbase_count; /* >0, count of sort base case bound */
  size_t mbase_count; /* >1, count of merge base case bound */
  size_t elt_size;
  size_t num_onthread_rec;
  void *cat_elts; /* pointer to concatenation buffer for merging */
  void *elts; /* pointer to an input array */
  int (*cmp)(const void *, const void *);
};

struct merge_arg{
  size_t ap, ar, bp, br, cs;
  size_t mbase_count; /* >1, count of merge base case bound */
  size_t elt_size;
  size_t num_onthread_rec;
  void *cat_seg_elts; /* pointer to concatenation buffer segment */
  void *elts; /* pointer to an input array */
  int (*cmp)(const void *, const void *);
};

static const size_t C_SIZE_ULIMIT = (size_t)-1; /* not reached as index */

static void *mergesort_thread(void *arg);
static void *merge_thread(void *arg);
static void merge(struct merge_arg *ma);
static void *ptr(const void *block, size_t i, size_t size);

/**
   Sorts a given array pointed to by elts in ascending order according to
   cmp. The array contains count elements of elt_size bytes. The first
   thread entry is placed on the thread stack of the caller.
   elts        : pointer to the array to sort
   count       : > 0, < 2**{size_t width - 1} count of elements in the array
   elt_size    : size of each element in the array in bytes
   sbase_count : > 0 base case upper bound for parallel sorting; if the count
                 of an unsorted subarray is less or equal to sbase_count,
                 then the subarray is sorted with serial sort routine (qsort)
   mbase_count : > 1 base case upper bound for parallel merging; if the sum
                 of the counts of two sorted subarrays is less or equal to
                 mbase_count, then the two subarrays are merged with a serial
                 merge routine
   cmp         : comparison function which returns a negative integer value
                 if the element pointed to by the first argument is less than
                 the element pointed to by the second, a positive integer
                 value if the element pointed to by the first argument is
                 greater than the element pointed to by the second, and zero
                 integer value if the two elements are equal
*/
void mergesort_pthread(void *elts,
		       size_t count,
		       size_t elt_size,
		       size_t sbase_count,
		       size_t mbase_count,
		       int (*cmp)(const void *, const void *)){
  struct mergesort_arg msa;
  if (count < 1) return;
  msa.p = 0;
  msa.r = count - 1;
  msa.sbase_count = sbase_count;
  msa.mbase_count = mbase_count;
  msa.elt_size = elt_size;
  msa.num_onthread_rec = 0;
  msa.elts = elts;
  msa.cat_elts = malloc_perror(count, elt_size);
  msa.cmp = cmp;
  mergesort_thread(&msa);
  free(msa.cat_elts);
}
  
/**
   Enters a mergesort thread that spawns mergesort threads recursively.
   The total number of threads is reduced and an additional speedup is
   provided by placing O(logn) recursive calls on a thread stack, with the
   tightness of the bound set by MERGESORT_PTHREAD_MAX_ONTHREAD_REC.
*/
static void *mergesort_thread(void *arg){
  size_t q;
  struct mergesort_arg *msa = arg;
  struct mergesort_arg child_msas[2];
  struct merge_arg ma;
  pthread_t child_ids[2];
  if (msa->r - msa->p + 1 <= msa->sbase_count){
    qsort(ptr(msa->elts, msa->p, msa->elt_size),
	  msa->r - msa->p + 1,
	  msa->elt_size,
	  msa->cmp);
  }else{
    
    /* sort recursion */
    q = (msa->p + msa->r) / 2; /* rounds down */
    child_msas[0].p = msa->p;
    child_msas[0].r = q;
    child_msas[0].sbase_count = msa->sbase_count;
    child_msas[0].mbase_count = msa->mbase_count;
    child_msas[0].elt_size = msa->elt_size;
    child_msas[0].num_onthread_rec = 0;
    child_msas[0].cat_elts = msa->cat_elts;
    child_msas[0].elts = msa->elts;
    child_msas[0].cmp = msa->cmp;
    child_msas[1].p = q + 1;
    child_msas[1].r = msa->r;
    child_msas[1].sbase_count = msa->sbase_count;
    child_msas[1].mbase_count = msa->mbase_count;
    child_msas[1].elt_size = msa->elt_size;
    child_msas[1].cat_elts = msa->cat_elts;
    child_msas[1].elts = msa->elts;
    child_msas[1].cmp = msa->cmp;
    thread_create_perror(&child_ids[0], mergesort_thread, &child_msas[0]);
    if (msa->num_onthread_rec < MERGESORT_PTHREAD_MAX_ONTHREAD_REC){
      /* keep putting mergesort_thread calls on the current thread stack */
      child_msas[1].num_onthread_rec = msa->num_onthread_rec + 1;
      mergesort_thread(&child_msas[1]);
    }else{
      child_msas[1].num_onthread_rec = 0;
      thread_create_perror(&child_ids[1], mergesort_thread, &child_msas[1]);
      thread_join_perror(child_ids[1], NULL);
    }
    thread_join_perror(child_ids[0], NULL);

    /* merge recursion */
    ma.ap = msa->p;
    ma.ar = q;
    ma.bp = q + 1;
    ma.br = msa->r;
    ma.cs = 0;
    ma.mbase_count = msa->mbase_count;
    ma.elt_size = msa->elt_size;
    ma.num_onthread_rec = msa->num_onthread_rec;
    ma.cat_seg_elts = ptr(msa->cat_elts, msa->p, msa->elt_size);
    ma.elts = msa->elts;
    ma.cmp = msa->cmp;
    merge_thread(&ma);
    /* copy the merged result into the input array */
    memcpy(ptr(msa->elts, msa->p, msa->elt_size),
	   ptr(ma.cat_seg_elts, 0, msa->elt_size),
	   (msa->r - msa->p + 1) * msa->elt_size);
  }
  return NULL;
}

/**
   Merges two sorted subarrays in a parallel manner.
*/
static void *merge_thread(void *arg){
  size_t aq, bq, ix;
  struct merge_arg *ma = arg;
  struct merge_arg child_mas[2];
  pthread_t child_ids[2];
  if ((ma->ap == C_SIZE_ULIMIT && ma->ar == C_SIZE_ULIMIT) ||
      (ma->bp == C_SIZE_ULIMIT && ma->br == C_SIZE_ULIMIT) ||
      (ma->ar - ma->ap) + (ma->br - ma->bp) + 2 <= ma->mbase_count){
    merge(ma);
    return NULL;
  }
  
  /* rec. parameters with <= 3/4 problem size for the larger subproblem */
  if (ma->ar - ma->ap > ma->br - ma->bp){
    aq = (ma->ap + ma->ar) / 2;
    child_mas[0].ap = ma->ap;
    child_mas[0].ar = aq;
    child_mas[0].cs = ma->cs;
    child_mas[1].ap = aq + 1;
    child_mas[1].ar = ma->ar;
    ix = leq_bsearch(ptr(ma->elts, aq, ma->elt_size),
		     ptr(ma->elts, ma->bp, ma->elt_size),
		     ma->br - ma->bp + 1, /* at least 1 element */
		     ma->elt_size,
		     ma->cmp);
    if (ix == ma->br - ma->bp + 1){
      child_mas[0].bp = C_SIZE_ULIMIT;
      child_mas[0].br = C_SIZE_ULIMIT;
      child_mas[1].bp = ma->bp;
      child_mas[1].br = ma->br;
      child_mas[1].cs = ma->cs + (aq - ma->ap + 1);
    }else if (ix == ma->br - ma->bp){
      child_mas[0].bp = ma->bp;
      child_mas[0].br = ma->br;
      child_mas[1].bp = C_SIZE_ULIMIT;
      child_mas[1].br = C_SIZE_ULIMIT;
      child_mas[1].cs = ma->cs + (aq - ma->ap) + (ma->br - ma->bp) + 2;
    }else{
      child_mas[0].bp = ma->bp;
      child_mas[0].br = ma->bp + ix;
      child_mas[1].bp = ma->bp + ix + 1;
      child_mas[1].br = ma->br;
      child_mas[1].cs = ma->cs + (aq - ma->ap) + ix + 2;
    }
  }else{
    bq = (ma->bp + ma->br) / 2;
    child_mas[0].bp = ma->bp;
    child_mas[0].br = bq;
    child_mas[0].cs = ma->cs;
    child_mas[1].bp = bq + 1;
    child_mas[1].br = ma->br;
    ix = leq_bsearch(ptr(ma->elts, bq, ma->elt_size),
		     ptr(ma->elts, ma->ap, ma->elt_size),
		     ma->ar - ma->ap + 1, /* at least 1 element */
		     ma->elt_size,
		     ma->cmp);
    if (ix == ma->ar - ma->ap + 1){
      child_mas[0].ap = C_SIZE_ULIMIT;
      child_mas[0].ar = C_SIZE_ULIMIT;
      child_mas[1].ap = ma->ap;
      child_mas[1].ar = ma->ar;
      child_mas[1].cs = ma->cs + (bq - ma->bp + 1);
    }else if (ix == ma->ar - ma->ap){
      child_mas[0].ap = ma->ap;
      child_mas[0].ar = ma->ar;
      child_mas[1].ap = C_SIZE_ULIMIT;
      child_mas[1].ar = C_SIZE_ULIMIT;
      child_mas[1].cs = ma->cs + (ma->ar - ma->ap) + (bq - ma->bp) + 2;
    }else{
      child_mas[0].ap = ma->ap;
      child_mas[0].ar = ma->ap + ix;
      child_mas[1].ap = ma->ap + ix + 1;
      child_mas[1].ar = ma->ar;
      child_mas[1].cs = ma->cs + ix + (bq - ma->bp) + 2;
    }
  }
  child_mas[0].mbase_count = ma->mbase_count;
  child_mas[0].elt_size = ma->elt_size;
  child_mas[0].num_onthread_rec = ma->num_onthread_rec;
  child_mas[0].cat_seg_elts = ma->cat_seg_elts;
  child_mas[0].elts = ma->elts;
  child_mas[0].cmp = ma->cmp;
  child_mas[1].mbase_count = ma->mbase_count;
  child_mas[1].elt_size = ma->elt_size;
  child_mas[1].num_onthread_rec = ma->num_onthread_rec;
  child_mas[1].cat_seg_elts = ma->cat_seg_elts;
  child_mas[1].elts = ma->elts;
  child_mas[1].cmp = ma->cmp;

  /* recursion */
  thread_create_perror(&child_ids[0], merge_thread, &child_mas[0]);
  if (ma->num_onthread_rec < MERGESORT_PTHREAD_MAX_ONTHREAD_REC){
    /* keep putting merge_thread calls on the current thread stack */
    child_mas[1].num_onthread_rec = ma->num_onthread_rec + 1;
    merge_thread(&child_mas[1]);
  }else{
    child_mas[1].num_onthread_rec = 0;
    thread_create_perror(&child_ids[1], merge_thread, &child_mas[1]);
    thread_join_perror(child_ids[1], NULL);
  }
  thread_join_perror(child_ids[0], NULL);
  return NULL;
}

/**
   Merges two sorted subarrays onto a concatenation array as the base case
   of parallel merge.
*/
static void merge(struct merge_arg *ma){
  size_t first_ix, second_ix, cat_ix;
  size_t elt_size = ma->elt_size;
  if (ma->ap == C_SIZE_ULIMIT && ma->ar == C_SIZE_ULIMIT){
    /* a is empty */
    memcpy(ptr(ma->cat_seg_elts, ma->cs, elt_size),
	   ptr(ma->elts, ma->bp, elt_size),
	   (ma->br - ma->bp + 1) * elt_size);
  }else if (ma->bp == C_SIZE_ULIMIT && ma->br == C_SIZE_ULIMIT){
    /* b is empty */
    memcpy(ptr(ma->cat_seg_elts, ma->cs, elt_size),
	   ptr(ma->elts, ma->ap, elt_size),
	   (ma->ar - ma->ap + 1) * elt_size);
  }else{
    /* a and b are each not empty */
    first_ix = ma->ap;
    second_ix = ma->bp;
    cat_ix = ma->cs;
    while(first_ix <= ma->ar && second_ix <= ma->br){
      if (ma->cmp(ptr(ma->elts, first_ix, elt_size),
		  ptr(ma->elts, second_ix, elt_size)) < 0){
	memcpy(ptr(ma->cat_seg_elts, cat_ix, elt_size),
	       ptr(ma->elts, first_ix, elt_size),
	       elt_size);
	cat_ix++;
	first_ix++;
      }else{
	memcpy(ptr(ma->cat_seg_elts, cat_ix, elt_size),
	       ptr(ma->elts, second_ix, elt_size),
	       elt_size);
	cat_ix++;
	second_ix++;
      }
    }
    if (second_ix == ma->br + 1){
      memcpy(ptr(ma->cat_seg_elts, cat_ix, elt_size),
	     ptr(ma->elts, first_ix, elt_size),
	     (ma->ar - first_ix + 1) * elt_size);
    }else{
      memcpy(ptr(ma->cat_seg_elts, cat_ix, elt_size),
	     ptr(ma->elts, second_ix, elt_size),
	     (ma->br - second_ix + 1) * elt_size);
    }
  }
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}
