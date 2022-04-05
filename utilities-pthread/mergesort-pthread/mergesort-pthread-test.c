/**
   mergesort-pthread-test.c

   Optimization and correctness tests of a generic merge sort algorithm with
   parallel sorting and parallel merging.

   The following command line arguments can be used to customize tests:
   mergesort-pthread-test
      [0, size_t width - 1) : a
      [0, size_t width - 1) : b s.t. 2**a <= count <= 2**b
      [0, size_t width) : c
      [0, size_t width) : d s.t. 2**c <= sort base case bound <= 2**d
      [1, size_t width) : e
      [1, size_t width) : f s.t. 2**e <= merge base case bound <= 2**f
      [0, 1] : int corner test on/off
      [0, 1] : int performance test on/off
      [0, 1] : double corner test on/off
      [0, 1] : double performance test on/off

   usage examples: 
   ./mergesort-pthread-test
   ./mergesort-pthread-test 17 17
   ./mergesort-pthread-test 20 20 15 20 15 20
   ./mergesort-pthread-test 20 20 15 20 15 20 0 1 0 1

   mergesort-pthread-test can be run with any subset of command line
   arguments in the above-defined order. If the (i + 1)th argument is
   specified then the ith argument must be specified for i >= 0. Default
   values are used for the unspecified arguments according to the
   C_ARGS_DEF array.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99. The requirements are: i) the width of size_t is less than 2040
   and is even, and ii) pthreads API is available.
   
   TODO: add portable printing of size_t
*/

#define _XOPEN_SOURCE 600

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include "mergesort-pthread.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-lim.h"

/**
   Generate random numbers in a portable way for test purposes only; rand()
   in the Linux C Library uses the same generator as random(), which may not
   be the case on older rand() implementations, and on current
   implementations on different systems.
*/
#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand()) /* [0, RAND_MAX] */
#define DRAND() ((double)rand() / RAND_MAX) /* [0.0, 1.0] */

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "mergesort-pthread-test\n"
  "[0, size_t width - 1) : a\n"
  "[0, size_t width - 1) : b s.t. 2**a <= count <= 2**b\n"
  "[0, size_t width) : c\n"
  "[0, size_t width) : d s.t. 2**c <= sort base case bound <= 2**d\n"
  "[1, size_t width) : e\n"
  "[1, size_t width) : f s.t. 2**e <= merge base case bound <= 2**f\n"
  "[0, 1] : int corner test on/off\n"
  "[0, 1] : int performance test on/off\n"
  "[0, 1] : double corner test on/off\n"
  "[0, 1] : double performance test on/off\n";
const int C_ARGC_ULIMIT = 11;
const size_t C_ARGS_DEF[10] = {15u, 15u, 10u, 15u, 10u, 15u, 1u, 1u, 1u, 1u};
const size_t C_FULL_BIT = PRECISION_FROM_ULIMIT((size_t)-1);

/* corner cases */
const size_t C_CORNER_TRIALS = 10u;
const size_t C_CORNER_COUNT_ULIMIT = 17u;
const size_t C_CORNER_SBASE_START = 1u;
const size_t C_CORNER_SBASE_END = 17u;
const size_t C_CORNER_MBASE_START = 2u;
const size_t C_CORNER_MBASE_END = 20u;
const double C_HALF_PROB = 0.5;

/* performance tests */
const size_t C_TRIALS = 5u;

double timer();
void print_test_result(int res);

int cmp_int(const void *a, const void *b){
  return ((*(const int *)a > *(const int *)b) -
	  (*(const int *)a < *(const int *)b));
}

int cmp_double(const void *a, const void *b){
  return ((*(const double *)a > *(const double *)b) -
	  (*(const double *)a < *(const double *)b));
}

/**
   Runs a mergesort_pthread corner cases test on random integer arrays.
*/
void run_int_corner_test(){
  int res = 1;
  int *arr_a = NULL, *arr_b = NULL;
  size_t count, sb, mb;
  size_t i, j;
  size_t elt_size =  sizeof(int);
  arr_a =  malloc_perror(C_CORNER_COUNT_ULIMIT, elt_size);
  arr_b =  malloc_perror(C_CORNER_COUNT_ULIMIT, elt_size);
  printf("Test mergesort_pthread on corner cases on random "
	 "integer arrays\n");
  for (count = 1; count <= C_CORNER_COUNT_ULIMIT; count++){
    for (sb = C_CORNER_SBASE_START; sb <= C_CORNER_SBASE_END; sb++){
      for (mb = C_CORNER_MBASE_START; mb <= C_CORNER_MBASE_END; mb++){
	for(i = 0; i < C_CORNER_TRIALS; i++){
	  for (j = 0; j < count; j++){
	    arr_a[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * RANDOM();
	  }
	  memcpy(arr_b, arr_a, count * elt_size);
	  mergesort_pthread(arr_a, count, elt_size, sb, mb, cmp_int);
	  qsort(arr_b, count, elt_size, cmp_int);
	  for (j = 0; j < count; j++){
	    res *= (arr_a[j] == arr_b[j]);
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
   Runs a test comparing mergesort_pthread vs. qsort performance on random 
   integer arrays across sort and merge base count bounds.
*/
void run_int_opt_test(size_t log_count_start,
		      size_t log_count_end,
		      size_t log_sbase_start,
		      size_t log_sbase_end,
		      size_t log_mbase_start,
		      size_t log_mbase_end){
  int res = 1;
  int *arr_a = NULL, *arr_b = NULL;
  size_t ci, si, mi;
  size_t count, sbase, mbase;
  size_t i, j;
  size_t elt_size = sizeof(int);
  double tot_m, tot_q, t_m, t_q;
  arr_a =  malloc_perror(pow_two_perror(log_count_end), elt_size);
  arr_b =  malloc_perror(pow_two_perror(log_count_end), elt_size);
  printf("Test mergesort_pthread performance on random integer arrays\n");
  for (ci = log_count_start; ci <= log_count_end; ci++){
    count = pow_two_perror(ci); /* > 0 */
    printf("\t# trials: %lu, array count: %lu\n",
	   TOLU(C_TRIALS), TOLU(count));
    for (si = log_sbase_start; si <= log_sbase_end; si++){
      sbase = pow_two_perror(si);
      printf("\t\tsort base count: %lu\n", TOLU(sbase));
      for (mi = log_mbase_start; mi <= log_mbase_end; mi++){
	mbase = pow_two_perror(mi);
	printf("\t\t\tmerge base count: %lu\n", TOLU(mbase));
	tot_m = 0.0;
	tot_q = 0.0;
	for(i = 0; i < C_TRIALS; i++){
	  for (j = 0; j < count; j++){
	    arr_a[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * RANDOM();
	  }
	  memcpy(arr_b, arr_a, count * elt_size);
	  t_m = timer();
	  mergesort_pthread(arr_a, count, elt_size, sbase, mbase, cmp_int);
	  t_m = timer() - t_m;
	  t_q = timer();
	  qsort(arr_b, count, elt_size, cmp_int);
	  t_q = timer() - t_q;
	  tot_m += t_m;
	  tot_q += t_q;
	  for (j = 0; j < count; j++){
	    res *= (arr_a[j] == arr_b[j]);
	  }
	}
	printf("\t\t\tave pthread mergesort: %.6f seconds\n",
	       tot_m / C_TRIALS);
	printf("\t\t\tave qsort:             %.6f seconds\n",
	       tot_q / C_TRIALS);
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
   Runs a mergesort_pthread corner cases test on random double arrays.
*/
void run_double_corner_test(){
  int res = 1;
  size_t count, sb, mb;
  size_t i, j;
  size_t elt_size =  sizeof(double);
  double *arr_a = NULL, *arr_b = NULL;
  arr_a =  malloc_perror(C_CORNER_COUNT_ULIMIT, elt_size);
  arr_b =  malloc_perror(C_CORNER_COUNT_ULIMIT, elt_size);
  printf("Test mergesort_pthread on corner cases on random "
	 "double arrays\n");
  for (count = 1; count <= C_CORNER_COUNT_ULIMIT; count++){
    for (sb = C_CORNER_SBASE_START; sb <= C_CORNER_SBASE_END; sb++){
      for (mb = C_CORNER_MBASE_START; mb <= C_CORNER_MBASE_END; mb++){
	for(i = 0; i < C_CORNER_TRIALS; i++){
	  for (j = 0; j < count; j++){
	    arr_a[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * DRAND();
	  }
	  memcpy(arr_b, arr_a, count * elt_size);
	  mergesort_pthread(arr_a, count, elt_size, sb, mb, cmp_double);
	  qsort(arr_b, count, elt_size, cmp_double);
	  for (j = 0; j < count; j++){
	    res *= (arr_a[j] == arr_b[j]);
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
   Runs a test comparing mergesort_pthread vs. qsort performance on random 
   double arrays across sort and merge base count bounds.
*/
void run_double_opt_test(size_t log_count_start,
			 size_t log_count_end,
			 size_t log_sbase_start,
			 size_t log_sbase_end,
			 size_t log_mbase_start,
			 size_t log_mbase_end){
  int res = 1;
  size_t ci, si, mi;
  size_t count, sbase, mbase;
  size_t i, j;
  size_t elt_size = sizeof(double);
  double *arr_a = NULL, *arr_b = NULL;
  double tot_m, tot_q, t_m, t_q;
  arr_a =  malloc_perror(pow_two_perror(log_count_end), elt_size);
  arr_b =  malloc_perror(pow_two_perror(log_count_end), elt_size);
  printf("Test mergesort_pthread performance on random double arrays\n");
  for (ci = log_count_start; ci <= log_count_end; ci++){
    count = pow_two_perror(ci); /* > 0 */
    printf("\t# trials: %lu, array count: %lu\n",
	   TOLU(C_TRIALS), TOLU(count));
    for (si = log_sbase_start; si <= log_sbase_end; si++){
      sbase = pow_two_perror(si);
      printf("\t\tsort base count: %lu\n", TOLU(sbase));
      for (mi = log_mbase_start; mi <= log_mbase_end; mi++){
	mbase = pow_two_perror(mi);
	printf("\t\t\tmerge base count: %lu\n", TOLU(mbase));
	tot_m = 0.0;
	tot_q = 0.0;
	for(i = 0; i < C_TRIALS; i++){
	  for (j = 0; j < count; j++){
	    arr_a[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * DRAND();
	  }
	  memcpy(arr_b, arr_a, count * elt_size);
	  t_m = timer();
	  mergesort_pthread(arr_a, count, elt_size, sbase, mbase, cmp_double);
	  t_m = timer() - t_m;
	  t_q = timer();
	  qsort(arr_b, count, elt_size, cmp_double);
	  t_q = timer() - t_q;
	  tot_m += t_m;
	  tot_q += t_q;
	  for (j = 0; j < count; j++){
	    res *= (arr_a[j] == arr_b[j]);
	  }
	}
	printf("\t\t\tave pthread mergesort: %.6f seconds\n",
	       tot_m / C_TRIALS);
	printf("\t\t\tave qsort:             %.6f seconds\n",
	       tot_q / C_TRIALS);
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
   Print result.
*/
void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(int argc, char *argv[]){
  int i;
  size_t *args = NULL;
  RGENS_SEED();
  if (argc > C_ARGC_ULIMIT){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_ULIMIT - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_ULIMIT - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] > C_FULL_BIT - 2 ||
      args[1] > C_FULL_BIT - 2 ||
      args[2] > C_FULL_BIT - 1 ||
      args[3] > C_FULL_BIT - 1 ||
      args[4] > C_FULL_BIT - 1 ||
      args[5] > C_FULL_BIT - 1 ||
      args[4] < 1 ||
      args[5] < 1 ||
      args[0] > args[1] ||
      args[2] > args[3] ||
      args[4] > args[5] ||
      args[6] > 1 ||
      args[7] > 1 ||
      args[8] > 1 ||
      args[9] > 1){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[6]) run_int_corner_test();
  if (args[7]) run_int_opt_test(args[0],
				args[1],
				args[2],
				args[3],
				args[4],
				args[5]);
  if (args[8]) run_double_corner_test();
  if (args[9]) run_double_opt_test(args[0],
				   args[1],
				   args[2],
				   args[3],
				   args[4],
				   args[5]);
  free(args);
  args = NULL;
  return 0;
}
