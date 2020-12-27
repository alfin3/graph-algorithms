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
#include "utilities-rand-mod.h"

#define RANDOM() (random())
#define DRAND48() (drand48())

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
  int num_iter = 100000;
  int key, *elts = NULL;
  int elt_size = sizeof(int);
  int num_counts = 5;
  int num_corner_counts = 3;
  uint64_t counts[5] = {10000, 100000, 1000000, 10000000, 100000000};
  uint64_t corner_counts[3] = {1, 2, 3};
  uint64_t count, max_count = counts[4];
  uint64_t geq_ix, leq_ix;
  double tot_geq, tot_leq, tot;
  clock_t t_geq, t_leq, t;
  elts =  malloc_perror(max_count * sizeof(int));
  srandom(time(0));
  printf("Test geq_bsearch and leq_bsearch on random int arrays\n");
  for (int ci = 0; ci < num_counts; ci++){
    count = counts[ci];
    printf("\tarray count: %lu, # trials: %d\n", count, num_iter);
    for (uint64_t ei = 0; ei < count; ei++){
      elts[ei] = RANDOM() % count - RANDOM() % count; //% to repeat values
    }
    qsort(elts, count, elt_size, cmp_int_fn);
    tot_geq = 0.0;
    tot_leq = 0.0;
    tot = 0.0;
    for(int i = 0; i < num_iter; i++){
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

  //corner cases
  printf("\tcorner cases\n");
  for (int ci = 0; ci < num_corner_counts; ci++){
    count = corner_counts[ci];
    for(int i = 0; i < num_iter; i++){
      for (uint64_t ei = 0; ei < count; ei++){
	elts[ei] = RANDOM() % count - RANDOM() % count; //% to repeat values
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
  int num_iter = 100000;
  int elt_size = sizeof(double);
  int num_counts = 5;
  int num_corner_counts = 3;
  uint64_t counts[5] = {10000, 100000, 1000000, 10000000, 100000000};
  uint64_t corner_counts[3] = {1, 2, 3};
  uint64_t count, max_count = counts[4];
  uint64_t geq_ix, leq_ix;
  double key, *elts = NULL;
  double tot_geq, tot_leq, tot;
  clock_t t_geq, t_leq, t;
  elts =  malloc_perror(max_count * sizeof(double));
  srand48(time(0));
  printf("Test geq_bsearch and leq_bsearch on random double arrays\n");
  for (int ci = 0; ci < num_counts; ci++){
    count = counts[ci];
    printf("\tarray count: %lu, # trials: %d\n", count, num_iter);
    for (uint64_t ei = 0; ei < count; ei++){
      elts[ei] = DRAND48();
    }
    qsort(elts, count, elt_size, cmp_double_fn);
    tot_geq = 0.0;
    tot_leq = 0.0;
    tot = 0.0;
    for(int i = 0; i < num_iter; i++){
      key = DRAND48();
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

  //corner cases
  printf("\tcorner cases\n");
  for (int ci = 0; ci < num_corner_counts; ci++){
    count = corner_counts[ci];
    for(int i = 0; i < num_iter; i++){
      for (uint64_t ei = 0; ei < count; ei++){
	elts[ei] = DRAND48();
      }
      qsort(elts, count, elt_size, cmp_double_fn);
      key = DRAND48(); 
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
  run_geq_leq_bsearch_int_test();
  run_geq_leq_bsearch_double_test();
  return 0;
}
