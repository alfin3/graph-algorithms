/**
   mergesort-mthread-uint64-main.c

*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include "mergesort-mthread-uint64.h"
#include "utilities-mem.h"

double timer();
void print_test_result(int result);

int cmp_int_fn(const void *a, const void *b){
  return *(int *)a - *(int *)b;
}

/**
   Runs a test comparing mergesort_mthread_uint64 vs. qsort performance on 
   random integer arrays.
*/
void run_int_test(){
  int result = 1;
  int num_iter = 5;
  int num_counts = 2;
  int num_sbase_counts = 5;
  uint64_t count_arr[2] = {1000000, 10000000};
  uint64_t max_count = count_arr[1];
  uint64_t sbase_count_arr[5] = {1000, 10000, 100000, 1000000, 10000000};
  int *arr_a =  malloc_perror(max_count * sizeof(int));
  int *arr_b =  malloc_perror(max_count * sizeof(int));
  double t_tot_m, t_tot_q, t_m, t_q;
  printf("Test mergesort_mthread_uint64 performance on random integer "
	 "arrays\n");
  for (int c_ix = 0; c_ix < num_counts; c_ix++){
    printf("\tarray count: %lu, # trials: %d\n", count_arr[c_ix], num_iter);
    for (int sbc_ix = 0; sbc_ix < num_sbase_counts; sbc_ix++){
      printf("\t\tmergesort base count:  %lu\n", sbase_count_arr[sbc_ix]);
      t_tot_m = 0.0;
      t_tot_q = 0.0;
      for(int i = 0; i < num_iter; i++){
	for (uint64_t j = 0; j < count_arr[c_ix]; j++){
	  arr_a[j] = random() % count_arr[c_ix]; //to get repeated vals
	}
	memcpy(arr_b, arr_a, count_arr[c_ix] * sizeof(int));
	t_m = timer();
	mergesort_mthread_uint64(arr_a,
				 count_arr[c_ix],
				 sbase_count_arr[sbc_ix],
				 sizeof(int),
				 cmp_int_fn);
	t_m = timer() - t_m;
	t_q = timer();
	qsort(arr_b, count_arr[c_ix], sizeof(int), cmp_int_fn);
	t_q = timer() - t_q;
	t_tot_m += t_m;
	t_tot_q += t_q;
	for (uint64_t j = 0; j < count_arr[c_ix]; j++){
	  result *= (cmp_int_fn(&arr_a[j], &arr_b[j]) == 0);
	}
      }
      printf("\t\tave mthread mergesort: %.6f seconds\n",
	     t_tot_m / num_iter);
      printf("\t\tave qsort:             %.6f seconds\n",
	     t_tot_q / num_iter);
      printf("\t\tcorrectness:           ");
      print_test_result(result);
    }
  }
  free(arr_a);
  free(arr_b);
  arr_a = NULL;
  arr_b = NULL;
}

/**
   Times execution.
*/
double timer(){
  struct timeval tm;
  gettimeofday(&tm, NULL);
  return tm.tv_sec + tm.tv_usec / (double)1000000;
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
  run_int_test();
  return 0;
}
