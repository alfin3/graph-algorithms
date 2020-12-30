/**
   mergesort-mthread.c

   generic implementation without race conditions (and overhead). 
   base count parameters can be used to optimizing on
   input ranges and specific hardware settings.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "mergesort-mthread.h"
#include "utilities-alg.h"
#include "utilities-concur.h"
#include "utilities-mem.h"

typedef struct{
  size_t p, r;
  size_t mbase_count; //>1, count of merge base case
  size_t sbase_count; //>0, count of sort base case
  size_t elt_size;
  size_t num_onthread_rec;
  void *elts; //pointer to an input array
  int (*cmp)(const void *, const void *);
} mergesort_arg_t;

typedef struct{
  size_t ap, ar, bp, br, cs;
  size_t mbase_count; //>1, count of merge base case
  size_t elt_size;
  size_t num_onthread_rec;
  void *cat_elts; 
  void *elts; //pointer to an input array
  int (*cmp)(const void *, const void *);
} merge_arg_t;

const size_t NR = SIZE_MAX; //cannot be reached as array index

static void *mergesort_thread(void *arg);
static void *merge_thread(void *arg);
static void merge(merge_arg_t *ma);
static void *elt_ptr(const void *elts, size_t i, size_t elt_size);

/**
   Runs the first thread entry on the thread of the caller.
*/
void mergesort_mthread(void *elts,
		       size_t count,
		       size_t elt_size,
		       size_t sbase_count,
		       size_t mbase_count,
		       int (*cmp)(const void *, const void *)){
  mergesort_arg_t msa;
  if (count < 2) return;
  msa.p = 0;
  msa.r = count - 1;
  msa.mbase_count = mbase_count;
  msa.sbase_count = sbase_count;
  msa.elt_size = elt_size;
  msa.num_onthread_rec = 0;
  msa.elts = elts;
  msa.cmp = cmp;
  mergesort_thread(&msa);
}
  
/**
   Enters a mergesort thread that spawns mergesort threads recursively.
   The total number of threads is reduced and an additional speedup is
   provided by placing O(logn) recursive calls on a thread stack, with the
   tightness of the bound set by MERGESORT_MTHREAD_MAX_ONTHREAD_REC.
*/
static void *mergesort_thread(void *arg){
  mergesort_arg_t *msa = arg;
  mergesort_arg_t child_msas[2];
  pthread_t child_ids[2];
  merge_arg_t ma;
  size_t q;
  if (msa->r - msa->p + 1 <= msa->sbase_count){
    qsort(elt_ptr(msa->elts, msa->p, msa->elt_size),
	  msa->r - msa->p + 1,
	  msa->elt_size,
	  msa->cmp);
  }else{
    
    //sort recursion
    q = (msa->p + msa->r) / 2; //rounds down
    child_msas[0].p = msa->p;
    child_msas[0].r = q;
    child_msas[0].sbase_count = msa->sbase_count;
    child_msas[0].mbase_count = msa->mbase_count;
    child_msas[0].elt_size = msa->elt_size;
    child_msas[0].num_onthread_rec = 0;
    child_msas[0].elts = msa->elts;
    child_msas[0].cmp = msa->cmp;
    child_msas[1].p = q + 1;
    child_msas[1].r = msa->r;
    child_msas[1].sbase_count = msa->sbase_count;
    child_msas[1].mbase_count = msa->mbase_count;
    child_msas[1].elt_size = msa->elt_size;
    child_msas[1].elts = msa->elts;
    child_msas[1].cmp = msa->cmp;
    thread_create_perror(&child_ids[0], mergesort_thread, &child_msas[0]);
    if (msa->num_onthread_rec < MERGESORT_MTHREAD_MAX_ONTHREAD_REC){
      //keep putting mergesort_thread calls on the current thread stack
      child_msas[1].num_onthread_rec = msa->num_onthread_rec + 1;
      mergesort_thread(&child_msas[1]);
    }else{
      child_msas[1].num_onthread_rec = 0;
      thread_create_perror(&child_ids[1], mergesort_thread, &child_msas[1]);
      thread_join_perror(child_ids[1], NULL);
    }
    thread_join_perror(child_ids[0], NULL);

    //merge recursiom
    ma.ap = msa->p;
    ma.ar = q;
    ma.bp = q + 1;
    ma.br = msa->r;
    ma.cs = 0;
    ma.mbase_count = msa->mbase_count;
    ma.elt_size = msa->elt_size;
    ma.num_onthread_rec = msa->num_onthread_rec;
    ma.cat_elts = malloc_perror((msa->r - msa->p + 1) * msa->elt_size);
    ma.elts = msa->elts;
    ma.cmp = msa->cmp;
    merge_thread(&ma);
    //copy the merged result into the input array
    memcpy(elt_ptr(msa->elts, msa->p, msa->elt_size),
	   elt_ptr(ma.cat_elts, 0, msa->elt_size),
	   (msa->r - msa->p + 1) * msa->elt_size);
    free(ma.cat_elts);
    ma.cat_elts = NULL;
  }
  return NULL;
}

/**
   Merges two subproblems in a parallel manner without race conditions.
*/
static void *merge_thread(void *arg){
  merge_arg_t *ma = arg;
  merge_arg_t child_mas[2];
  pthread_t child_ids[2];
  size_t aq, bq, ix;
  if ((ma->ap == NR && ma->ar == NR) ||
      (ma->bp == NR && ma->br == NR) ||
      (ma->ar - ma->ap) + (ma->br - ma->bp) + 2 <= ma->mbase_count){
    merge(ma);
    return NULL;
  }
  
  //recursion parameters with <= 3/4 problem size for the larger subproblem
  if (ma->ar - ma->ap > ma->br - ma->bp){
    aq = (ma->ap + ma->ar) / 2;
    child_mas[0].ap = ma->ap;
    child_mas[0].ar = aq;
    child_mas[0].cs = ma->cs;
    child_mas[1].ap = aq + 1;
    child_mas[1].ar = ma->ar;
    ix = leq_bsearch(elt_ptr(ma->elts, aq, ma->elt_size),
		     elt_ptr(ma->elts, ma->bp, ma->elt_size),
		     ma->br - ma->bp + 1, //at least 1 element
		     ma->elt_size,
		     ma->cmp);
    if (ix == ma->br - ma->bp + 1){
      child_mas[0].bp = NR;
      child_mas[0].br = NR;
      child_mas[1].bp = ma->bp;
      child_mas[1].br = ma->br;
      child_mas[1].cs = ma->cs + (aq - ma->ap + 1);
    }else if (ix == ma->br - ma->bp){
      child_mas[0].bp = ma->bp;
      child_mas[0].br = ma->br;
      child_mas[1].bp = NR;
      child_mas[1].br = NR;
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
    ix = leq_bsearch(elt_ptr(ma->elts, bq, ma->elt_size),
		     elt_ptr(ma->elts, ma->ap, ma->elt_size),
		     ma->ar - ma->ap + 1, //at least 1 element
		     ma->elt_size,
		     ma->cmp);
    if (ix == ma->ar - ma->ap + 1){
      child_mas[0].ap = NR;
      child_mas[0].ar = NR;
      child_mas[1].ap = ma->ap;
      child_mas[1].ar = ma->ar;
      child_mas[1].cs = ma->cs + (bq - ma->bp + 1);
    }else if (ix == ma->ar - ma->ap){
      child_mas[0].ap = ma->ap;
      child_mas[0].ar = ma->ar;
      child_mas[1].ap = NR;
      child_mas[1].ar = NR;
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
  child_mas[0].cat_elts = ma->cat_elts;
  child_mas[0].elts = ma->elts;
  child_mas[0].cmp = ma->cmp;
  child_mas[1].mbase_count = ma->mbase_count;
  child_mas[1].elt_size = ma->elt_size;
  child_mas[1].num_onthread_rec = ma->num_onthread_rec;
  child_mas[1].cat_elts = ma->cat_elts;
  child_mas[1].elts = ma->elts;
  child_mas[1].cmp = ma->cmp;

  //recursion
  thread_create_perror(&child_ids[0], merge_thread, &child_mas[0]);
  if (ma->num_onthread_rec < MERGESORT_MTHREAD_MAX_ONTHREAD_REC){
    //keep putting merge_thread calls on the current thread stack
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
   Merge two contiguous or non-contiguous sorted regions of an element array 
   onto a concatenation array. This is the base case for parallel merge.
*/
static void merge(merge_arg_t *ma){
  size_t first_ix, second_ix, cat_ix;
  size_t elt_size = ma->elt_size;
  if (ma->ap == NR && ma->ar == NR){
    //a is empty
    memcpy(elt_ptr(ma->cat_elts, ma->cs, elt_size),
	   elt_ptr(ma->elts, ma->bp, elt_size),
	   (ma->br - ma->bp + 1) * elt_size);
  }else if (ma->bp == NR && ma->br == NR){
    //b is empty
    memcpy(elt_ptr(ma->cat_elts, ma->cs, elt_size),
	   elt_ptr(ma->elts, ma->ap, elt_size),
	   (ma->ar - ma->ap + 1) * elt_size);
  }else{
    //a and b are each not empty
    first_ix = ma->ap;
    second_ix = ma->bp;
    cat_ix = ma->cs;
    while(first_ix <= ma->ar && second_ix <= ma->br){
      if (ma->cmp(elt_ptr(ma->elts, first_ix, elt_size),
		  elt_ptr(ma->elts, second_ix, elt_size)) < 0){
	memcpy(elt_ptr(ma->cat_elts, cat_ix, elt_size),
	       elt_ptr(ma->elts, first_ix, elt_size),
	       elt_size);
	cat_ix++;
	first_ix++;
      }else{
	memcpy(elt_ptr(ma->cat_elts, cat_ix, elt_size),
	       elt_ptr(ma->elts, second_ix, elt_size),
	       elt_size);
	cat_ix++;
	second_ix++;
      }
    }
    if (second_ix == ma->br + 1){
      memcpy(elt_ptr(ma->cat_elts, cat_ix, elt_size),
	     elt_ptr(ma->elts, first_ix, elt_size),
	     (ma->ar - first_ix + 1) * elt_size);
    }else{
      memcpy(elt_ptr(ma->cat_elts, cat_ix, elt_size),
	     elt_ptr(ma->elts, second_ix, elt_size),
	     (ma->br - second_ix + 1) * elt_size);
    }
  }
}

/**
   Computes a pointer to an element in an element array.
*/
static void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
}
