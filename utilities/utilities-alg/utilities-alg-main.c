/**
   utilities-alg-main.c

   Tests of general algorithms.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "utilities-alg.h"
#include "utilities-mem.h"
#include "utilities-rand-mod.h"

void print_test_result(int result);

/**
   Test geq_bsearch and leq_bsearch.
*/

int cmp_int_fn(const void *a, const void *b){
  return *(int *)a - *(int *)b;
}

void run_geq_leq_bsearch_int_test(){
  int res = 1;
  int num_iter = 100000;
  int num_pow_twos = 4;
  int pow_twos[4] = {24, 25, 26, 27};
  int key, *elts = NULL;
  int elt_size = sizeof(int);
  uint64_t count;
  uint64_t max_count = pow_two_uint64(27);
  uint64_t geq_ix, leq_ix;
  double tot_geq = 0.0, tot_leq = 0.0,  tot = 0.0;
  clock_t t_geq, t_leq, t;
  srandom(time(0));
  elts =  malloc_perror(max_count * sizeof(int));
  printf("Test geq_bsearch and leq_bsearch on random integer arrays\n");
  for (int pow_two_ix = 0; pow_two_ix < num_pow_twos; pow_two_ix++){
    count = pow_two_uint64(pow_twos[pow_two_ix]);
    printf("\tarray count: %lu, # trials: %d\n", count, num_iter);
    for (uint64_t j = 0; j < count; j++){
      elts[j] = random() % count; //to get repeated vals
    }
    qsort(elts, count, elt_size, cmp_int_fn);
    for(int i = 0; i < num_iter; i++){
      key = random();
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
      if (geq_ix == count){
	res *= (cmp_int_fn(&key, &elts[count - 1]) > 0);
      }else if (geq_ix == 0){
	res *= (cmp_int_fn(&key, &elts[geq_ix]) <= 0);
      }else{
	res *= (cmp_int_fn(&key, &elts[geq_ix]) <= 0);
	res *= (cmp_int_fn(&key, &elts[geq_ix - 1]) >= 0);
      }
      if (leq_ix == count){
	res *= (cmp_int_fn(&key, &elts[0]) < 0);
      }else if (leq_ix == count - 1){
	res *= (cmp_int_fn(&key, &elts[leq_ix]) >= 0);
      }else{
	res *= (cmp_int_fn(&key, &elts[leq_ix]) >= 0);
	res *= (cmp_int_fn(&key, &elts[leq_ix + 1]) <= 0);
      }	
    }
    printf("\t\t\tgeq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tleq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tbsearch:     %.6f seconds\n", tot);
    printf("\t\t\tcorrectness: ");
    print_test_result(res);
  }
  free(elts);
  elts = NULL;
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
  return 0;
}
