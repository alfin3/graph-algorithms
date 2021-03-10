/**
   mergesort-mthread-main.c

   Optimization and correctness tests of a generic merge sort algorithm with
   parallel sorting and parallel merging. 

   Requirements for running tests: 
   - UINT64_MAX must be defined

*/

#define _POSIX_C_SOURCE 200112L

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include "mergesort-mthread.h"
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

double timer();
void print_uint64_elts(const uint64_t *a, size_t count);
void print_test_result(int res);

int cmp_int_fn(const void *a, const void *b){
  if (*(int *)a > *(int *)b){
    return 1;
  }else if  (*(int *)a < *(int *)b){
    return -1;
  }else{
    return 0;
  }
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

/**
   Runs a mergesort_mthread corner cases test on random integer arrays.
*/
void run_int_corner_test(){
  int res = 1;
  int ci, num_counts = 7;
  int si, num_sbase_counts = 3;
  int mi, num_mbase_counts = 3;
  int i, num_iter = 100;
  uint64_t j, count_arr[7] = {1, 2, 3, 4, 16, 64, 100};
  uint64_t max_count = count_arr[6];
  uint64_t sbase_count_arr[3] = {1, 2, 3};
  uint64_t mbase_count_arr[3] = {2, 3, 4};
  int *arr_a =  malloc_perror(max_count * sizeof(int));
  int *arr_b =  malloc_perror(max_count * sizeof(int));
  printf("Test mergesort_mthread on random integer corner case arrays\n");
  printf("\t# trials:          %d\n", num_iter);
  printf("\tarray counts:      ");
  print_uint64_elts(count_arr, num_counts);
  printf("\tsort base counts:  ");
  print_uint64_elts(sbase_count_arr, num_sbase_counts);
  printf("\tmerge base counts: ");
  print_uint64_elts(mbase_count_arr, num_mbase_counts);
  for (ci = 0; ci < num_counts; ci++){
    for (si = 0; si < num_sbase_counts; si++){
      for (mi = 0; mi < num_mbase_counts; mi++){
	for(i = 0; i < num_iter; i++){
	  for (j = 0; j < count_arr[ci]; j++){
	    arr_a[j] = (RANDOM() % count_arr[ci] -
			RANDOM() % count_arr[ci]); /* % to repeat vals */
	  }
	  memcpy(arr_b, arr_a, count_arr[ci] * sizeof(int));
	  mergesort_mthread(arr_a,
			    count_arr[ci],
			    sizeof(int),
			    sbase_count_arr[si],
			    mbase_count_arr[mi],
			    cmp_int_fn);
	  qsort(arr_b, count_arr[ci], sizeof(int), cmp_int_fn);
	  for (j = 0; j < count_arr[ci]; j++){
	    res *= (cmp_int_fn(&arr_a[j], &arr_b[j]) == 0);
	  }
	}
      }
    }
  }
  printf("\tcorrectness:       ");
  print_test_result(res);
  free(arr_a);
  free(arr_b);
  arr_a = NULL;
  arr_b = NULL;
}

/**
   Runs a test comparing mergesort_mthread vs. qsort performance on random 
   integer arrays across sort and merge base count bounds.
*/
void run_int_opt_test(){
  int res = 1;
  int ci, num_counts = 1;
  int si, num_sbase_counts = 4;
  int mi, num_mbase_counts = 5;
  int i, num_iter = 5;
  uint64_t j, count_arr[1] = {10000000};
  uint64_t max_count = count_arr[0];
  uint64_t sbase_count_arr[4] = {10000, 100000, 1000000, 10000000};
  uint64_t mbase_count_arr[5] = {1000000,
				 2000000,
				 3000000,
				 4000000,
				 10000000};
  int *arr_a =  malloc_perror(max_count * sizeof(int));
  int *arr_b =  malloc_perror(max_count * sizeof(int));
  double tot_m, tot_q, t_m, t_q;
  printf("Test mergesort_mthread performance on random integer arrays\n");
  for (ci = 0; ci < num_counts; ci++){
    printf("\t# trials: %d, array count: %lu\n", num_iter, count_arr[ci]);
    for (si = 0; si < num_sbase_counts; si++){
      printf("\t\tsort base count: %lu\n", sbase_count_arr[si]);
      for (mi = 0; mi < num_mbase_counts; mi++){
	printf("\t\t\tmerge base count: %lu\n", mbase_count_arr[mi]);
	tot_m = 0.0;
	tot_q = 0.0;
	for(i = 0; i < num_iter; i++){
	  for (j = 0; j < count_arr[ci]; j++){
	    arr_a[j] = (RANDOM() % count_arr[ci] -
			RANDOM() % count_arr[ci]); /* % to repeat vals */
	  }
	  memcpy(arr_b, arr_a, count_arr[ci] * sizeof(int));
	  t_m = timer();
	  mergesort_mthread(arr_a,
			    count_arr[ci],
			    sizeof(int),
			    sbase_count_arr[si],
			    mbase_count_arr[mi],
			    cmp_int_fn);
	  t_m = timer() - t_m;
	  t_q = timer();
	  qsort(arr_b, count_arr[ci], sizeof(int), cmp_int_fn);
	  t_q = timer() - t_q;
	  tot_m += t_m;
	  tot_q += t_q;
	  for (j = 0; j < count_arr[ci]; j++){
	    res *= (cmp_int_fn(&arr_a[j], &arr_b[j]) == 0);
	  }
	}
	printf("\t\t\tave mthread mergesort: %.6f seconds\n",
	       tot_m / num_iter);
	printf("\t\t\tave qsort:             %.6f seconds\n",
	       tot_q / num_iter);
	printf("\t\t\tcorrectness:           ");
	print_test_result(res);
      }
    }
  }
}

/**
   Runs a mergesort_mthread corner cases test on random double arrays.
*/
void run_double_corner_test(){
  int res = 1;
  int ci, num_counts = 7;
  int si, num_sbase_counts = 3;
  int mi, num_mbase_counts = 3;
  int i, num_iter = 100;
  uint64_t j, count_arr[7] = {1, 2, 3, 4, 16, 64, 100};
  uint64_t max_count = count_arr[6];
  uint64_t sbase_count_arr[3] = {1, 2, 3};
  uint64_t mbase_count_arr[3] = {2, 3, 4};
  double *arr_a =  malloc_perror(max_count * sizeof(double));
  double *arr_b =  malloc_perror(max_count * sizeof(double));
  printf("Test mergesort_mthread on random double corner case arrays\n");
  printf("\t# trials:          %d\n", num_iter);
  printf("\tarray counts:      ");
  print_uint64_elts(count_arr, num_counts);
  printf("\tsort base counts:  ");
  print_uint64_elts(sbase_count_arr, num_sbase_counts);
  printf("\tmerge base counts: ");
  print_uint64_elts(mbase_count_arr, num_mbase_counts);
  for (ci = 0; ci < num_counts; ci++){
    for (si = 0; si < num_sbase_counts; si++){
      for (mi = 0; mi < num_mbase_counts; mi++){
	for(i = 0; i < num_iter; i++){
	  for (j = 0; j < count_arr[ci]; j++){
	    arr_a[j] = DRAND() - DRAND();
	  }
	  memcpy(arr_b, arr_a, count_arr[ci] * sizeof(double));
	  mergesort_mthread(arr_a,
			    count_arr[ci],
			    sizeof(double),
			    sbase_count_arr[si],
			    mbase_count_arr[mi],
			    cmp_double_fn);
	  qsort(arr_b, count_arr[ci], sizeof(double), cmp_double_fn);
	  for (j = 0; j < count_arr[ci]; j++){
	    /* == 0 because of the same bit patterns */
	    res *= (cmp_double_fn(&arr_a[j], &arr_b[j]) == 0);
	  }
	}
      }
    }
  }
  printf("\tcorrectness:       ");
  print_test_result(res);
  free(arr_a);
  free(arr_b);
  arr_a = NULL;
  arr_b = NULL;
}

/**
   Runs a test comparing mergesort_mthread vs. qsort performance on random 
   double arrays across sort and merge base count bounds.
*/
void run_double_opt_test(){
  int res = 1;
  int ci, num_counts = 1;
  int si, num_sbase_counts = 4;
  int mi, num_mbase_counts = 5;
  int i, num_iter = 5;
  uint64_t j, count_arr[1] = {10000000};
  uint64_t max_count = count_arr[0];
  uint64_t sbase_count_arr[4] = {10000, 100000, 1000000, 10000000};
  uint64_t mbase_count_arr[5] = {1000000,
				 2000000,
				 3000000,
				 4000000,
				 10000000};
  double *arr_a =  malloc_perror(max_count * sizeof(double));
  double *arr_b =  malloc_perror(max_count * sizeof(double));
  double tot_m, tot_q, t_m, t_q;
  printf("Test mergesort_mthread performance on random double arrays\n");
  for (ci = 0; ci < num_counts; ci++){
    printf("\t# trials: %d, array count: %lu\n", num_iter, count_arr[ci]);
    for (si = 0; si < num_sbase_counts; si++){
      printf("\t\tsort base count: %lu\n", sbase_count_arr[si]);
      for (mi = 0; mi < num_mbase_counts; mi++){
	printf("\t\t\tmerge base count: %lu\n", mbase_count_arr[mi]);
	tot_m = 0.0;
	tot_q = 0.0;
	for(i = 0; i < num_iter; i++){
	  for (j = 0; j < count_arr[ci]; j++){
	    arr_a[j] = DRAND() - DRAND();
	  }
	  memcpy(arr_b, arr_a, count_arr[ci] * sizeof(double));
	  t_m = timer();
	  mergesort_mthread(arr_a,
			    count_arr[ci],
			    sizeof(double),
			    sbase_count_arr[si],
			    mbase_count_arr[mi],
			    cmp_double_fn);
	  t_m = timer() - t_m;
	  t_q = timer();
	  qsort(arr_b, count_arr[ci], sizeof(double), cmp_double_fn);
	  t_q = timer() - t_q;
	  tot_m += t_m;
	  tot_q += t_q;
	  for (j = 0; j < count_arr[ci]; j++){
	    /* == 0 because of the same bit patterns */
	    res *= (cmp_double_fn(&arr_a[j], &arr_b[j]) == 0);
	  }
	}
	printf("\t\t\tave mthread mergesort: %.6f seconds\n",
	       tot_m / num_iter);
	printf("\t\t\tave qsort:             %.6f seconds\n",
	       tot_q / num_iter);
	printf("\t\t\tcorrectness:           ");
	print_test_result(res);
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
   Print helper functions.
*/

void print_uint64_elts(const uint64_t *a, size_t count){
  size_t i;
  for (i = 0; i < count; i++){
    printf("%lu ", a[i]);
  }
  printf("\n");
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  RGENS_SEED();
  run_int_corner_test();
  run_int_opt_test();
  run_double_corner_test();
  run_double_opt_test();
  return 0;
}
