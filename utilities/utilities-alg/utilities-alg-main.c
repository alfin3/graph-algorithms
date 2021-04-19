/**
   utilities-alg-main.c

   Tests of general algorithms.

   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "utilities-alg.h"
#include "utilities-mem.h"

/**
   Generate random numbers in a portable way for test purposes only; rand()
   in the Linux C Library uses the same generator as random(), which may not
   be the case on older rand() implementations, and on current
   implementations on different systems.
*/
#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand()) /* [0, RAND_MAX] */
#define DRAND() ((double)rand() / RAND_MAX) /* [0.0, 1.0] */

void *elt_ptr(const void *elts, size_t i, size_t elt_size);
void print_test_result(int result);

/**
   Test geq_bsearch and leq_bsearch.
*/

int cmp_int_fn(const void *a, const void *b){
  return *(int *)a - *(int *)b;
}

int cmp_double_fn(const void *a, const void *b){
  if (*(double *)a > *(double *)b){
    return 1;
  }else if  (*(double *)a < *(double *)b){
    return -1;
  }else{
    return 0;
  }
}

int test_geq_leq_indices(const void *key,
			 const void *elts,
			 uint64_t count,
			 uint64_t elt_size,
			 uint64_t geq_ix,
			 uint64_t leq_ix,
			 int (*cmp)(const void *, const void *));

void run_geq_leq_bsearch_int_test(){
  int res = 1;
  int i, num_iter = 100000;
  int key, *elts = NULL;
  int elt_size = sizeof(int);
  int ci, num_counts = 5, num_corner_counts = 3;
  uint64_t counts[5] = {10000, 100000, 1000000, 10000000, 100000000};
  uint64_t corner_counts[3] = {1, 2, 3};
  uint64_t ei, count; 
  uint64_t max_count = counts[4];
  uint64_t geq_ix, leq_ix;
  double tot_geq, tot_leq, tot;
  clock_t t_geq, t_leq, t;
  elts =  malloc_perror(max_count, sizeof(int));
  printf("Test geq_bsearch and leq_bsearch on random int arrays\n");
  for (ci = 0; ci < num_counts; ci++){
    count = counts[ci];
    printf("\tarray count: %lu, # trials: %d\n", count, num_iter);
    for (ei = 0; ei < count; ei++){
      elts[ei] = RANDOM() % count - RANDOM() % count; /* % to repeat */
    }
    qsort(elts, count, elt_size, cmp_int_fn);
    tot_geq = 0.0;
    tot_leq = 0.0;
    tot = 0.0;
    for(i = 0; i < num_iter; i++){
      key = RANDOM() % count - RANDOM() % count;
      t_geq = clock();
      geq_ix = geq_bsearch(&key, elts, count, elt_size, cmp_int_fn);
      t_geq = clock() - t_geq;
      tot_geq += (double)t_geq / CLOCKS_PER_SEC;
      t_leq = clock();
      leq_ix = leq_bsearch(&key, elts, count, elt_size, cmp_int_fn);
      t_leq = clock() - t_leq;
      tot_leq += (double)t_leq / CLOCKS_PER_SEC;
      t = clock();
      bsearch(&key, elts, count, elt_size, cmp_int_fn);
      t = clock() - t;
      tot += (double)t / CLOCKS_PER_SEC;
      res *= test_geq_leq_indices(&key,
				  elts,
				  count,
				  elt_size,
				  geq_ix,
				  leq_ix,
				  cmp_int_fn);
    }
    printf("\t\t\tgeq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tleq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tbsearch:     %.6f seconds\n", tot);
    printf("\t\t\tcorrectness: ");
    print_test_result(res);
  }

  /* corner cases */
  printf("\tcorner cases\n");
  for (ci = 0; ci < num_corner_counts; ci++){
    count = corner_counts[ci];
    for(i = 0; i < num_iter; i++){
      for (ei = 0; ei < count; ei++){
	elts[ei] = RANDOM() % count - RANDOM() % count; /* % to repeat */
      }
      qsort(elts, count, elt_size, cmp_int_fn);
      key = RANDOM() % count - RANDOM() % count;
      geq_ix = geq_bsearch(&key, elts, count, elt_size, cmp_int_fn);
      leq_ix = leq_bsearch(&key, elts, count, elt_size, cmp_int_fn);
      res *= test_geq_leq_indices(&key,
				  elts,
				  count,
				  elt_size,
				  geq_ix,
				  leq_ix,
				  cmp_int_fn);
    }
  }
  printf("\t\t\tcorrectness: ");
  print_test_result(res);
  free(elts);
  elts = NULL;
}

void run_geq_leq_bsearch_double_test(){
  int res = 1;
  int i, num_iter = 100000;
  int elt_size = sizeof(double);
  int ci, num_counts = 5, num_corner_counts = 3;
  uint64_t counts[5] = {10000, 100000, 1000000, 10000000, 100000000};
  uint64_t corner_counts[3] = {1, 2, 3};
  uint64_t ei, count; 
  uint64_t max_count = counts[4];
  uint64_t geq_ix, leq_ix;
  double key, *elts = NULL;
  double tot_geq, tot_leq, tot;
  clock_t t_geq, t_leq, t;
  elts =  malloc_perror(max_count, sizeof(double));
  printf("Test geq_bsearch and leq_bsearch on random double arrays\n");
  for (ci = 0; ci < num_counts; ci++){
    count = counts[ci];
    printf("\tarray count: %lu, # trials: %d\n", count, num_iter);
    for (ei = 0; ei < count; ei++){
      elts[ei] = DRAND();
    }
    qsort(elts, count, elt_size, cmp_double_fn);
    tot_geq = 0.0;
    tot_leq = 0.0;
    tot = 0.0;
    for(i = 0; i < num_iter; i++){
      key = DRAND();
      t_geq = clock();
      geq_ix = geq_bsearch(&key, elts, count, elt_size, cmp_double_fn);
      t_geq = clock() - t_geq;
      tot_geq += (double)t_geq / CLOCKS_PER_SEC;
      t_leq = clock();
      leq_ix = leq_bsearch(&key, elts, count, elt_size, cmp_double_fn);
      t_leq = clock() - t_leq;
      tot_leq += (double)t_leq / CLOCKS_PER_SEC;
      t = clock();
      bsearch(&key, elts, count, elt_size, cmp_double_fn);
      t = clock() - t;
      tot += (double)t / CLOCKS_PER_SEC;
      res *= test_geq_leq_indices(&key,
				  elts,
				  count,
				  elt_size,
				  geq_ix,
				  leq_ix,
				  cmp_double_fn);
    }
    printf("\t\t\tgeq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tleq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tbsearch:     %.6f seconds\n", tot);
    printf("\t\t\tcorrectness: ");
    print_test_result(res);
  }

  /* corner cases */
  printf("\tcorner cases\n");
  for (ci = 0; ci < num_corner_counts; ci++){
    count = corner_counts[ci];
    for(i = 0; i < num_iter; i++){
      for (ei = 0; ei < count; ei++){
	elts[ei] = DRAND();
      }
      qsort(elts, count, elt_size, cmp_double_fn);
      key = DRAND(); 
      geq_ix = geq_bsearch(&key, elts, count, elt_size, cmp_double_fn);
      leq_ix = leq_bsearch(&key, elts, count, elt_size, cmp_double_fn);
      res *= test_geq_leq_indices(&key,
				  elts,
				  count,
				  elt_size,
				  geq_ix,
				  leq_ix,
				  cmp_double_fn);
    }
  }
  printf("\t\t\tcorrectness: ");
  print_test_result(res);
  free(elts);
  elts = NULL;
}

int test_geq_leq_indices(const void *key,
			 const void *elts,
			 uint64_t count,
			 uint64_t elt_size,
			 uint64_t geq_ix,
			 uint64_t leq_ix,
			 int (*cmp)(const void *, const void *)){
  int res = 1;
  if (geq_ix == count){
    res *= (cmp(key, elt_ptr(elts, count - 1, elt_size)) > 0);
  }else if (geq_ix == 0){
    res *= (cmp(key, elt_ptr(elts, geq_ix, elt_size)) <= 0);
  }else{
    res *= (cmp(key, elt_ptr(elts, geq_ix, elt_size)) <= 0);
    res *= (cmp(key, elt_ptr(elts, geq_ix - 1, elt_size)) >= 0);
  }
  if (leq_ix == count){
    res *= (cmp(key, elt_ptr(elts, 0, elt_size)) < 0);
  }else if (leq_ix == count - 1){
    res *= (cmp(key, elt_ptr(elts, leq_ix, elt_size)) >= 0);
  }else{
    res *= (cmp(key, elt_ptr(elts, leq_ix, elt_size)) >= 0);
    res *= (cmp(key, elt_ptr(elts, leq_ix + 1, elt_size)) <= 0);
  }
  return res;
}									 

/**
   Computes a pointer to the ith element in an array pointed to by elts.
*/
void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
}

/**
   Prints test result.
*/
void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  RGENS_SEED();
  run_geq_leq_bsearch_int_test();
  run_geq_leq_bsearch_double_test();
  return 0;
}
