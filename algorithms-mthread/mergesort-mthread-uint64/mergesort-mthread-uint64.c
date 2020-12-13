/**
   mergesort-mthread-uint64.c

*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "mergesort-mthread-uint64.h"
#include "utilities-mem.h"
#include "utilities-concur.h"

typedef struct{
  uint64_t p, r;
  uint64_t sbase_count; //>1, count of the sort base case
  int elt_size;
  int num_onthread_rec;
  void *elts; //pointer to an input array
  int (*cmp_elt_fn)(const void *, const void *);
} mergesort_arg_t;

#define MAX_NUM_ONTHREAD_REC (30) //max # recursive calls on a thread stack

static void *mergesort_thread(void *arg);
static void merge(void *elts,
		  uint64_t p,
		  uint64_t q ,
		  uint64_t r,
		  int elt_size,
		  int (*cmp_elt_fn)(const void *, const void *));
static void *elt_ptr(void *elts, uint64_t i, int elt_size);

/**
   Runs the first thread entry on the thread of the caller.
*/
void mergesort_mthread_uint64(void *elts,
			      uint64_t count,
			      uint64_t sbase_count,
			      int elt_size,
			      int (*cmp_elt_fn)(const void *, const void *)){
  mergesort_arg_t ma;
  if (count < 2) return;
  ma.p = 0;
  ma.r = count - 1;
  ma.sbase_count = sbase_count;
  ma.elt_size = elt_size;
  ma.num_onthread_rec = 0;
  ma.elts = elts;
  ma.cmp_elt_fn = cmp_elt_fn;
  mergesort_thread(&ma);
}
  
/**
   Enters a mergesort thread that spawns mergesort threads recursively.
   Reduces the total number of threads by a factor of 2 if allowed to
   place O(logn) recursive calls on a thread (see MAX_NUM_ONTHREAD_REC). 
   The default thread stack size should be sufficient for most inputs.
*/
static void *mergesort_thread(void *arg){
  mergesort_arg_t *ma = arg;
  mergesort_arg_t child_mas[2];
  pthread_t child_ids[2];
  uint64_t q;
  if (ma->r - ma->p + 1 <= ma->sbase_count){
    qsort(elt_ptr(ma->elts, ma->p, ma->elt_size),
	  ma->r - ma->p + 1,
	  ma->elt_size,
	  ma->cmp_elt_fn);
  }else{
    q = (ma->p + ma->r) / 2; //rounds down
    child_mas[0].p = ma->p;
    child_mas[0].r = q;
    child_mas[0].sbase_count = ma->sbase_count;
    child_mas[0].elt_size = ma->elt_size;
    child_mas[0].num_onthread_rec = 0;
    child_mas[0].elts = ma->elts;
    child_mas[0].cmp_elt_fn = ma->cmp_elt_fn;
    child_mas[1].p = q + 1;
    child_mas[1].r = ma->r;
    child_mas[1].sbase_count = ma->sbase_count;
    child_mas[1].elt_size = ma->elt_size;
    child_mas[1].elts = ma->elts;
    child_mas[1].cmp_elt_fn = ma->cmp_elt_fn;
    thread_create_perror(&child_ids[0], mergesort_thread, &child_mas[0]);
    if (ma->num_onthread_rec < MAX_NUM_ONTHREAD_REC){
      //keep putting mergesort_thread calls on the current thread stack
      child_mas[1].num_onthread_rec = ma->num_onthread_rec + 1;
      mergesort_thread(&child_mas[1]);
    }else{
      child_mas[1].num_onthread_rec = 0;
      thread_create_perror(&child_ids[1], mergesort_thread, &child_mas[1]);
      thread_join_perror(child_ids[1], NULL);
    }
    thread_join_perror(child_ids[0], NULL);
    merge(ma->elts, ma->p, q, ma->r, ma->elt_size, ma->cmp_elt_fn);
  }
  return NULL;
}

 /**
   Merge with minimized copying steps. Not yet parallel.
*/
static void merge(void *elts,
		  uint64_t p,
		  uint64_t q ,
		  uint64_t r,
		  int elt_size,
		  int (*cmp_elt_fn)(const void *, const void *)){
  void *temp = malloc_perror((r - p + 1) * elt_size);
  uint64_t first_ix = p, second_ix = q + 1, temp_ix = 0;
  while(first_ix <= q && second_ix <= r){
    if (cmp_elt_fn(elt_ptr(elts, first_ix, elt_size),
		   elt_ptr(elts, second_ix, elt_size)) < 0){
      memcpy(elt_ptr(temp, temp_ix, elt_size),
	     elt_ptr(elts, first_ix, elt_size),
	     elt_size);
      temp_ix++;
      first_ix++;
    }else{
      memcpy(elt_ptr(temp, temp_ix, elt_size),
	     elt_ptr(elts, second_ix, elt_size),
	     elt_size);
      temp_ix++;
      second_ix++;
    }
  }
  if (second_ix == r + 1){
    memcpy(elt_ptr(elts, p + temp_ix, elt_size),
	   elt_ptr(elts, first_ix, elt_size),
	   (q - first_ix + 1) * elt_size);
  }
  memcpy(elt_ptr(elts, p, elt_size),
	 elt_ptr(temp, 0, elt_size),
	 temp_ix * elt_size);
  free(temp);
  temp = NULL;
}

/**
   Computes a pointer to an element in an element array.
*/
static void *elt_ptr(void *elts, uint64_t i, int elt_size){
  return (void *)((char *)elts + i * elt_size);
}
