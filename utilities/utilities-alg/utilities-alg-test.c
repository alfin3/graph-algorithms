/**
   utilities-alg-test.c

   Tests of general algorithm utilities.

   The following command line arguments can be used to customize tests:
   utilities-alg-test
      [0, size_t width) : n for 2**n trials in geq_leq_bsearch tests
      [0, size_t width) : a
      [0, size_t width) : b s.t. 2**a <= count <= 2**b in geq_leq_bsearch tests
      [0, 1] : geq_leq_bsearch int test on/off
      [0, 1] : geq_leq_bsearch double test on/off

   usage examples: 
   ./utilities-alg-test
   ./utilities-alg-test 0 0 10
   ./utilities-alg-test 0 25 25
   ./utilities-alg-test 10 20 25 0 1

   utilities-alg-test can be run with any subset of command line arguments in
   the above-defined order. If the (i + 1)th argument is specified then the
   ith argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 with the only requirement that CHAR_BIT * sizeof(size_t) is even.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99. The tests require that:
   - clock_t is convertible to double,
   - the width of size_t is less than 2040 and even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "utilities-alg.h"
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
  "utilities-alg-test\n"
  "[0, size_t width) : n for 2**n trials in geq_leq_bsearch tests\n"
  "[0, size_t width) : a\n"
  "[0, size_t width) : b s.t. 2**a <= count <= 2**b in geq_leq_bsearch tests\n"
  "[0, 1] : geq_leq_bsearch int test on/off\n"
  "[0, 1] : geq_leq_bsearch double test on/off\n";
const int C_ARGC_ULIMIT = 6u;
const size_t C_ARGS_DEF[5] = {10u, 10u, 15u, 1u, 1u};
const size_t C_FULL_BIT = PRECISION_FROM_ULIMIT((size_t)-1);

/* tests */
const size_t C_NRAND_COUNT_ULIMIT = 100u;
const double C_HALF_PROB = 0.5;

void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int result);

/**
   Test geq_bsearch and leq_bsearch.
*/

int cmp_int(const void *a, const void *b){
  return ((*(const int *)a > *(const int *)b) -
	  (*(const int *)a < *(const int *)b));
}

int cmp_double(const void *a, const void *b){
  return ((*(const double *)a > *(const double *)b) -
	  (*(const double *)a < *(const double *)b));
}

int is_geq_leq_correct(const void *key,
		       const void *elts,
		       size_t count,
		       size_t elt_size,
		       size_t geq_ix,
		       size_t leq_ix,
		       int (*cmp)(const void *, const void *));

void run_geq_leq_bsearch_int_test(size_t log_trials,
				  size_t log_count_start,
				  size_t log_count_end){
  int res = 1;
  int key;
  int *elts = NULL, *nrand_elts = NULL;
  size_t i, j, count;
  size_t k, trials;
  size_t elt_size = sizeof(int);
  size_t geq_ix, leq_ix;
  double tot_geq, tot_leq, tot;
  clock_t t_geq, t_leq, t;
  trials = pow_two_perror(log_trials);
  elts = malloc_perror(pow_two_perror(log_count_end), elt_size);
  nrand_elts = malloc_perror(C_NRAND_COUNT_ULIMIT, elt_size);
  printf("Test geq_bsearch and leq_bsearch on random int arrays\n");
  for (i = log_count_start; i <= log_count_end; i++){
    count = pow_two_perror(i);
    for (j = 0; j < count; j++){
      elts[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * RANDOM();
    }
    qsort(elts, count, elt_size, cmp_int);
    tot_geq = 0.0;
    tot_leq = 0.0;
    tot = 0.0;
    for(k = 0; k < trials; k++){
      key = (DRAND() < C_HALF_PROB ? -1 : 1) * RANDOM();
      t_geq = clock();
      geq_ix = geq_bsearch(&key, elts, count, elt_size, cmp_int);
      t_geq = clock() - t_geq;
      tot_geq += (double)t_geq / CLOCKS_PER_SEC;
      t_leq = clock();
      leq_ix = leq_bsearch(&key, elts, count, elt_size, cmp_int);
      t_leq = clock() - t_leq;
      tot_leq += (double)t_leq / CLOCKS_PER_SEC;
      t = clock();
      bsearch(&key, elts, count, elt_size, cmp_int);
      t = clock() - t;
      tot += (double)t / CLOCKS_PER_SEC;
      res *= is_geq_leq_correct(&key,
				elts,
				count,
				elt_size,
				geq_ix,
				leq_ix,
				cmp_int);
    }
    printf("\tarray count: %lu, # trials: %lu\n", TOLU(count), TOLU(trials));
    printf("\t\t\tgeq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tleq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tbsearch:     %.6f seconds\n", tot);
    printf("\t\t\tcorrectness: ");
    print_test_result(res);
  }
  printf("\tnon-random array and corner cases\n");
  res = 1;
  for (j = 0; j < C_NRAND_COUNT_ULIMIT; j++){
    if (j == 0){
      nrand_elts[j] = 1;
    }else{
      nrand_elts[j] = nrand_elts[j - 1] + 2; /* odd elements */
    }
  }
  for (count = 1; count <= C_NRAND_COUNT_ULIMIT; count++){
    for (j = 0; j <= count; j++){
      key = 2 * j; /* even keys */
      geq_ix = geq_bsearch(&key,
			   nrand_elts,
			   count,
			   elt_size,
			   cmp_int);
      leq_ix = leq_bsearch(&key,
			   nrand_elts,
			   count,
			   elt_size,
			   cmp_int);
      if (j == 0){
	res *= (geq_ix == 0 && leq_ix == count);
      }else if (j == count){
	res *= (geq_ix == count && leq_ix == count - 1);
      }else{
	res *= (geq_ix == j && leq_ix == j - 1);
      }
    }
  }
  printf("\t\t\tcorrectness: ");
  print_test_result(res);
  free(elts);
  free(nrand_elts);
  elts = NULL;
  nrand_elts = NULL;
}

void run_geq_leq_bsearch_double_test(size_t log_trials,
				     size_t log_count_start,
				     size_t log_count_end){
  int res = 1;
  size_t i, j, count;
  size_t k, trials;
  size_t elt_size = sizeof(double);
  size_t geq_ix, leq_ix;
  double key;
  double *elts = NULL, *nrand_elts = NULL;
  double tot_geq, tot_leq, tot;
  clock_t t_geq, t_leq, t;
  trials = pow_two_perror(log_trials);
  elts = malloc_perror(pow_two_perror(log_count_end), elt_size);
  nrand_elts = malloc_perror(C_NRAND_COUNT_ULIMIT, elt_size);
  printf("Test geq_bsearch and leq_bsearch on random double arrays\n");
  for (i = log_count_start; i <= log_count_end; i++){
    count = pow_two_perror(i);
    for (j = 0; j < count; j++){
      elts[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * DRAND();
    }
    qsort(elts, count, elt_size, cmp_double);
    tot_geq = 0.0;
    tot_leq = 0.0;
    tot = 0.0;
    for(k = 0; k < trials; k++){
      key = (DRAND() < C_HALF_PROB ? -1 : 1) * DRAND();
      t_geq = clock();
      geq_ix = geq_bsearch(&key, elts, count, elt_size, cmp_double);
      t_geq = clock() - t_geq;
      tot_geq += (double)t_geq / CLOCKS_PER_SEC;
      t_leq = clock();
      leq_ix = leq_bsearch(&key, elts, count, elt_size, cmp_double);
      t_leq = clock() - t_leq;
      tot_leq += (double)t_leq / CLOCKS_PER_SEC;
      t = clock();
      bsearch(&key, elts, count, elt_size, cmp_double);
      t = clock() - t;
      tot += (double)t / CLOCKS_PER_SEC;
      res *= is_geq_leq_correct(&key,
				elts,
				count,
				elt_size,
				geq_ix,
				leq_ix,
				cmp_double);
    }
    printf("\tarray count: %lu, # trials: %lu\n", TOLU(count), TOLU(trials));
    printf("\t\t\tgeq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tleq_bsearch: %.6f seconds\n", tot_geq);
    printf("\t\t\tbsearch:     %.6f seconds\n", tot);
    printf("\t\t\tcorrectness: ");
    print_test_result(res);
  }
  printf("\tnon-random array and corner cases\n");
  res = 1;
  for (j = 0; j < C_NRAND_COUNT_ULIMIT; j++){
    if (j == 0){
      nrand_elts[j] = 1;
    }else{
      nrand_elts[j] = nrand_elts[j - 1] + 2;
    }
  }
  for (count = 1; count <= C_NRAND_COUNT_ULIMIT; count++){
    for (j = 0; j <= count; j++){
      key = 2 * j;
      geq_ix = geq_bsearch(&key,
			   nrand_elts,
			   count,
			   elt_size,
			   cmp_double);
      leq_ix = leq_bsearch(&key,
			   nrand_elts,
			   count,
			   elt_size,
			   cmp_double);
      if (j == 0){
	res *= (geq_ix == 0 && leq_ix == count);
      }else if (j == count){
	res *= (geq_ix == count && leq_ix == count - 1);
      }else{
	res *= (geq_ix == j && leq_ix == j - 1);
      }
    }
  }
  printf("\t\t\tcorrectness: ");
  print_test_result(res);
  free(elts);
  free(nrand_elts);
  elts = NULL;
  nrand_elts = NULL;
}

int is_geq_leq_correct(const void *key,
		       const void *elts,
		       size_t count,
		       size_t elt_size,
		       size_t geq_ix,
		       size_t leq_ix,
		       int (*cmp)(const void *, const void *)){
  int res = 1;
  if (geq_ix == count){
    res *= (cmp(key, ptr(elts, count - 1, elt_size)) > 0);
  }else if (geq_ix == 0){
    res *= (cmp(key, ptr(elts, geq_ix, elt_size)) <= 0);
  }else{
    res *= (cmp(key, ptr(elts, geq_ix, elt_size)) <= 0);
    res *= (cmp(key, ptr(elts, geq_ix - 1, elt_size)) >= 0);
  }
  if (leq_ix == count){
    res *= (cmp(key, ptr(elts, 0, elt_size)) < 0);
  }else if (leq_ix == count - 1){
    res *= (cmp(key, ptr(elts, leq_ix, elt_size)) >= 0);
  }else{
    res *= (cmp(key, ptr(elts, leq_ix, elt_size)) >= 0);
    res *= (cmp(key, ptr(elts, leq_ix + 1, elt_size)) <= 0);
  }
  return res;
}									 

/**
   Computes a pointer to the ith element in the block of elements.
*/
void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
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
  if (args[0] > C_FULL_BIT - 1 ||
      args[1] > C_FULL_BIT - 1 ||
      args[2] > C_FULL_BIT - 1 ||
      args[1] > args[2] ||
      args[3] > 1 ||
      args[4] > 1){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[3]) run_geq_leq_bsearch_int_test(args[0], args[1], args[2]);
  if (args[4]) run_geq_leq_bsearch_double_test(args[0], args[1], args[2]);
  free(args);
  args = NULL;
  return 0;
}
