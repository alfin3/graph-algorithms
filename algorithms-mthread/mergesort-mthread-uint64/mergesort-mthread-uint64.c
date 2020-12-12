/**
   mergesort-mthread-uint64.c

*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "mergesort-mthread-uint64.h"
#include "utilities-mem.h"
#include "utilities-concur.h"

typedef struct{
  uint64_t p, r, base_count;
  void *a;
} mergesort_arg_t;

static void *mergesort_thread(void *arg);

static int int_cmp(const void *a, const void *b){
  return *(int *)a - *(int *)b;
}

/**
   Runs the first thread entry on the thread of the caller.
*/
void mergesort_mthread_uint64(void *a, uint64_t count, uint64_t base_count){
  mergesort_arg_t ma;
  if (count < 2) return;
  ma.p = 0;
  ma.r = count - 1;
  ma.base_count = base_count;
  ma.a = a;
  mergesort_thread(&ma);
}

/**
   Merge. Not yet parallel and not yet optimized.
*/
static void merge(void *a, uint64_t p,  uint64_t q , uint64_t r){
  int *temp = malloc_perror(sizeof(int) * (r - p + 1));
  int *arr = a;
  uint64_t first_ix = p, second_ix = q + 1, temp_ix = 0;
  while(first_ix <= q && second_ix <= r){
    if (arr[first_ix] < arr[second_ix]){
      temp[temp_ix] = arr[first_ix];
      temp_ix++;
      first_ix++;
    }else{
      temp[temp_ix] = arr[second_ix];
      temp_ix++;
      second_ix++;
    }
  }
  if (first_ix == q + 1){
    while (second_ix <= r){
      temp[temp_ix] = arr[second_ix];
      temp_ix++;
      second_ix++;
    }
  }else{
    while (first_ix <= q){
      temp[temp_ix] = arr[first_ix];
      temp_ix++;
      first_ix++;
    }
  }
  memcpy(arr + p, temp, temp_ix * sizeof(int));
}
  
/**
   Enters a mergesort thread that spawns mergesort threads recursively 
   on stack (speed vs recursion depth).
*/
static void *mergesort_thread(void *arg){
  mergesort_arg_t *ma = arg;
  mergesort_arg_t child_mas[2];
  pthread_t child_ids[2];
  uint64_t q;
  if (ma->r - ma->p + 1 <= ma->base_count){
    qsort((int *)ma->a + ma->p, ma->r - ma->p + 1, sizeof(int), int_cmp);
  }else{
    q = (ma->p + ma->r) / 2; //rounds to lower
    child_mas[0].p = ma->p;
    child_mas[0].r = q;
    child_mas[0].base_count = ma->base_count;
    child_mas[0].a = ma->a;
    child_mas[1].p = q + 1;
    child_mas[1].r = ma->r;
    child_mas[1].base_count = ma->base_count;
    child_mas[1].a = ma->a;
    thread_create_perror(&child_ids[0], mergesort_thread, &child_mas[0]);
    thread_create_perror(&child_ids[1], mergesort_thread, &child_mas[1]);
    thread_join_perror(child_ids[0], NULL);
    thread_join_perror(child_ids[1], NULL);
    merge(ma->a, ma->p, q , ma->r);
  }
  return NULL;
}
