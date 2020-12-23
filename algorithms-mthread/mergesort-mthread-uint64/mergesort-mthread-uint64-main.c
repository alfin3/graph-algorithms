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
  int num_mbase_counts = 3;
  int num_sbase_counts = 1;
  uint64_t count_arr[2] = {1000000, 10000000};
  uint64_t max_count = count_arr[1];
  uint64_t mbase_count_arr[3] = {100000, 1000000, 10000000};
  uint64_t sbase_count_arr[1] = {10000000};
  int *arr_a =  malloc_perror(max_count * sizeof(int));
  int *arr_b =  malloc_perror(max_count * sizeof(int));
  double t_tot_m, t_tot_q, t_m, t_q;
  printf("Test mergesort_mthread_uint64 performance on random integer "
	 "arrays\n");
  for (int ci = 0; ci < num_counts; ci++){
    printf("\tarray count: %lu, # trials: %d\n", count_arr[ci], num_iter);
    for (int mi = 0; mi < num_mbase_counts; mi++){
      printf("\t\tmerge base count:  %lu\n", mbase_count_arr[mi]);
      for (int si = 0; si < num_sbase_counts; si++){
	printf("\t\t\tsort base count:  %lu\n", sbase_count_arr[si]);
	t_tot_m = 0.0;
	t_tot_q = 0.0;
	for(int i = 0; i < num_iter; i++){
	  for (uint64_t j = 0; j < count_arr[ci]; j++){
	    arr_a[j] = random() % count_arr[ci]; //to get repeated vals
	  }
	  memcpy(arr_b, arr_a, count_arr[ci] * sizeof(int));
	  t_m = timer();
	  mergesort_mthread_uint64(arr_a,
				   count_arr[ci],
				   mbase_count_arr[mi],
				   sbase_count_arr[si],
				   sizeof(int),
				   cmp_int_fn);
	  t_m = timer() - t_m;
	  t_q = timer();
	  qsort(arr_b, count_arr[ci], sizeof(int), cmp_int_fn);
	  t_q = timer() - t_q;
	  t_tot_m += t_m;
	  t_tot_q += t_q;
	  for (uint64_t j = 0; j < count_arr[ci]; j++){
	    result *= (cmp_int_fn(&arr_a[j], &arr_b[j]) == 0);
	  }
	}
	printf("\t\t\tave mthread mergesort: %.6f seconds\n",
	       t_tot_m / num_iter);
	printf("\t\t\tave qsort:             %.6f seconds\n",
	       t_tot_q / num_iter);
	printf("\t\t\tcorrectness:           ");
	print_test_result(result);
      }
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
