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

double timer(){
  struct timeval tm;
  gettimeofday(&tm, NULL);
  return tm.tv_sec + tm.tv_usec / (double)1000000;
}

int cmp_int_fn(const void *a, const void *b){
  return *(int *)a - *(int *)b;
}

/**
   Runs a test comparing mergesort_mthread_uint64 vs. qsort performance on 
   random integer arrays.
*/
void run_int_test(){
  int num_iter = 5;
  int num_counts = 1;
  int num_sbase_counts = 4;
  uint64_t count_arr[1] = {1000000};
  uint64_t sbase_count_arr[4] = {1000, 10000, 100000, 1000000};
  int *arr_a =  malloc_perror(count_arr[num_counts - 1] * sizeof(int));
  int *arr_b =  malloc_perror(count_arr[num_counts - 1] * sizeof(int));
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
	  arr_a[j] = random() % count_arr[num_counts - 1];
	  arr_b[j] = random() % count_arr[num_counts - 1];
	}
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
      }
      printf("\t\tave mthread mergesort: %.6f seconds\n",
	     t_tot_m / num_iter);
      printf("\t\tave qsort:             %.6f seconds\n",
	     t_tot_q / num_iter);
    }
  }
  free(arr_a);
  free(arr_b);
  arr_a = NULL;
  arr_b = NULL;
}

int main(){
  run_int_test();
  return 0;
}
