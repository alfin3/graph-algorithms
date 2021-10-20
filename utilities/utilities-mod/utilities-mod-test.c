/**
   utilities-mod-test.c

   Tests of integer overflow-safe utility functions in modular arithmetic.

   The following command line arguments can be used to customize tests:
   utilities-mod-test
      [0, size_t width) : n for 2**n # trials in tests
      [0, size_t width) : a
      [0, size_t width) : b s.t. 2**a <= size - 1 <= 2**b in mem mod tests
      [0, 1] : pow_mod, mul_mod, and sum_mod tests on/off
      [0, 1] : mem_mod test on/off
      [0, 1] : mul_ext, represent_uint, and pow_two_perror tests on/off

   usage examples: 
   ./utilities-mod-test 20
   ./utilities-mod-test 20 0 15
   ./utilities-mod-test 20 25 25 0 1 1
   ./utilities-mod-test 20 30 30 0 0 1 

   utilities-mod-test can be run with any subset of command line arguments in
   the above-defined order. If the (i + 1)th argument is specified then the
   ith argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 with the only requirement that the width of size_t is even.

   TODO: add portable size_t printing
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
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
  "utilities-mod-test \n"
  "[0, size_t width) : n for 2**n # trials in tests \n"
  "[0, size_t width) : a \n"
  "[0, size_t width) : b s.t. 2**a <= size - 1 <= 2**b in mem mod tests \n"
  "[0, 1] : pow_mod, mul_mod, and sum_mod tests on/off \n"
  "[0, 1] : mem_mod test on/off \n"
  "[0, 1] : mul_ext, represent_uint, and pow_two_perror tests on/off \n";
const int C_ARGC_MAX = 7;
const size_t C_ARGS_DEF[6] = {15u, 10u, 15u, 1u, 1u, 1u};

/* tests */
const unsigned char C_UCHAR_MAX = (unsigned char)-1;
const size_t C_SIZE_MAX = (size_t)-1; /* >= 3 */
const size_t C_SIZE_HALF = (((size_t)1 <<
			     (UINT_WIDTH_FROM_MAX((size_t)-1) / 2u)) - 1u);
const size_t C_BYTE_BIT = CHAR_BIT;
const size_t C_FULL_BIT = UINT_WIDTH_FROM_MAX((size_t)-1); /* >= 2 */
const size_t C_HALF_BIT = UINT_WIDTH_FROM_MAX((size_t)-1) / 2u; /* >= 1 */
const size_t C_BASE_MAX = (((size_t)1 << (CHAR_BIT / 2u))
			   + 1u); /* >= 2, <= C_SIZE_MAX */

void print_test_result(int res);

/**
   Tests pow_mod.
*/
void run_pow_mod_test(int log_trials){
  int res = 1;
  size_t i, trials;
  size_t k_max, n_max;
  size_t base_sq_max = C_SIZE_MAX;
  size_t j, k;
  size_t a, n;
  size_t r, r_wo;
  trials = pow_two_perror(log_trials);
  k_max = 1;
  n_max = C_SIZE_HALF - 1; /* >= 0 */
  while (base_sq_max / C_BASE_MAX >= C_BASE_MAX){
    base_sq_max /= C_BASE_MAX;
    k_max++;
  }
  printf("Run pow_mod random test\n ");
  for (i = 0; i < trials; i++){
    r_wo = 1;
    a = DRAND() * C_BASE_MAX;
    k = DRAND() * k_max;
    n = 1 + DRAND() * n_max; /* >= 1*/
    r = pow_mod(a, k, n);
    for (j = 0; j < k; j++){
      r_wo *= a;
    }
    r_wo = r_wo % n;
    res *= (r == r_wo);
  }
  printf("\t0 <= a <= %lu, 0 <= k <= %lu, 0 < n <= 2**%lu - 1 --> ",
	 TOLU(C_BASE_MAX), TOLU(k_max), TOLU(C_HALF_BIT));
  print_test_result(res);
  res = 1;
  k_max = C_SIZE_MAX - 1;
  n_max = C_SIZE_MAX - 2;
  for (i = 0; i < trials; i++){
    k = DRAND() * k_max;
    while (k & 1){
      k = DRAND() * k_max;
    }
    n = 2 + DRAND() * n_max;
    a = n - 1;
    r = pow_mod(a, k, n);
    res *= (r == 1);
  }
  printf("\ta = n - 1, 0 <= k < 2**%lu - 1, 1 < n <= 2**%lu - 1, "
	 "where 0 = k (mod 2) --> ",
	 TOLU(C_FULL_BIT), TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  res *= (pow_mod(0, 0, 1) == 0);
  res *= (pow_mod(2, 0, 1) == 0);
  res *= (pow_mod(0, 0, 2) == 1);
  res *= (pow_mod(2, 0, 2) == 1);
  res *= (pow_mod(C_SIZE_MAX, C_SIZE_MAX, C_SIZE_MAX) == 0);
  res *= (pow_mod(C_SIZE_MAX - 1, C_SIZE_MAX, C_SIZE_MAX) == C_SIZE_MAX - 1);
  res *= (pow_mod(C_SIZE_MAX, C_SIZE_MAX - 1, C_SIZE_MAX) == 0);
  printf("\tcorner cases --> ");
  print_test_result(res);
}

/**
   Tests mul_mod.
*/
void run_mul_mod_test(int log_trials){
  int res = 1;
  size_t i, trials;
  size_t a_max, b_max, n_max;
  size_t a, b, n;
  size_t r, r_wo;
  trials = pow_two_perror(log_trials);
  a_max = C_SIZE_HALF;
  b_max = C_SIZE_HALF;
  n_max = C_SIZE_MAX - 1;
  printf("Run mul_mod random test\n");
  for (i = 0; i < trials; i++){
    a = DRAND() * a_max;
    b = DRAND() * b_max;
    n = 1 + DRAND() * n_max;
    r = mul_mod(a, b, n);
    r_wo = (a * b) % n;
    res *= (r == r_wo);
  }
  printf("\ta, b <= 2**%lu - 1, 0 < n <= 2**%lu - 1 --> ",
	 TOLU(C_HALF_BIT), TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  for (i = 0; i < trials; i++){
    n = 2 + DRAND() * (n_max - 1);
    r = mul_mod(n - 1, n - 1, n);
    res *= (r == 1);
  }
  printf("\ta, b = n - 1, 1 < n <= 2**%lu - 1 --> ", TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  res *= (mul_mod(0, 0, 1) == 0);
  res *= (mul_mod(1, 0, 2) == 0);
  res *= (mul_mod(0, 1, 2) == 0);
  res *= (mul_mod(0, 2, 2) == 0);
  res *= (mul_mod(1, 1, 2) == 1);
  res *= (mul_mod(0, C_SIZE_MAX - 1, C_SIZE_MAX) == 0);
  res *= (mul_mod(C_SIZE_MAX - 1, 0, C_SIZE_MAX) == 0);
  res *= (mul_mod(C_SIZE_MAX - 1, 1, C_SIZE_MAX) == C_SIZE_MAX - 1);
  res *= (mul_mod(1, C_SIZE_MAX - 1, C_SIZE_MAX) == C_SIZE_MAX - 1);
  res *= (mul_mod(C_SIZE_MAX - 1, C_SIZE_MAX - 1, C_SIZE_MAX - 1) == 0);
  res *= (mul_mod(C_SIZE_MAX - 1, C_SIZE_MAX - 1, C_SIZE_MAX) == 1);
  res *= (mul_mod(C_SIZE_MAX, C_SIZE_MAX, C_SIZE_MAX) == 0);
  printf("\tcorner cases --> ");
  print_test_result(res);
}

/**
   Tests sum_mod.
*/
void run_sum_mod_test(int log_trials){
  int res = 1;
  size_t i, trials;
  size_t a_max, b_max, n_max;
  size_t a, b, n;
  size_t r, r_wo;
  trials = pow_two_perror(log_trials);
  a_max = pow_two_perror(C_FULL_BIT - 1) - 1;
  b_max = pow_two_perror(C_FULL_BIT - 1) - 1;
  n_max = C_SIZE_MAX - 1;
  printf("Run sum_mod random test\n");
  for (i = 0; i < trials; i++){
    a = DRAND() * a_max;
    b = DRAND() * b_max;
    n = 1 + DRAND() * n_max;
    r = sum_mod(a, b, n);
    r_wo = (a + b) % n;
    res *= (r == r_wo);
  }
  printf("\ta, b <= 2**%lu - 1, 0 < n <= 2**%lu - 1 --> ",
	 TOLU(C_FULL_BIT - 1), TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  for (i = 0; i < trials; i++){
    b = 1 + DRAND() * n_max;
    r = sum_mod(n_max, b, n_max + 1);
    res *= (r == b - 1);
  }
  printf("\ta = 2**%lu - 2, 0 < b <= 2**%lu - 1, n = 2**%lu - 1 --> ",
	 TOLU(C_FULL_BIT), TOLU(C_FULL_BIT), TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  res *= (sum_mod(0, 0, 1) == 0);
  res *= (sum_mod(1, 0, 2) == 1);
  res *= (sum_mod(0, 1, 2) == 1);
  res *= (sum_mod(1, 1, 2) == 0);
  res *= (sum_mod(C_SIZE_MAX - 1, C_SIZE_MAX - 1, C_SIZE_MAX) ==
	  C_SIZE_MAX - 2);
  printf("\tcorner cases --> ");
  print_test_result(res);
}

/**
   Tests mem_mod.
*/
void run_mem_mod_test(int log_size_start,
		      int log_size_end){		      
  unsigned char *block = NULL;
  int res = 1;
  int i;
  size_t n, mod_n;
  size_t size;
  clock_t t;
  block = calloc_perror(1, add_sz_perror(pow_two_perror(log_size_end), 1));
  printf("Run mem_mod on large memory blocks\n");
  n = 1 + DRAND() * (C_SIZE_MAX - 1);
  for (i = log_size_start; i <= log_size_end; i++){
    size = pow_two_perror(i) + 1;
    block[size - 1] = 1;
    t = clock();
    mod_n = mem_mod(block, size, n);
    t = clock() - t;
    res = (mod_n == pow_mod(mul_mod(pow_two_perror(C_BYTE_BIT - 1), 2, n),
			    size - 1,
			    n));
    printf("\tblock size:  %lu bytes \n", TOLU(size));
    printf("\truntime:     %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    printf("\tcorrectness: ");
    print_test_result(res);
    res = 1;
    block[size - 1] = 0;
  }
  free(block);
  block = NULL;
}

/**
   Tests mul_ext.
*/
void run_mul_ext_test(int log_trials){
  int res = 1;
  size_t i, trials;
  size_t a, b, n;
  size_t h, l;
  size_t *hl = NULL;
  trials = pow_two_perror(log_trials);
  hl = malloc_perror(2, sizeof(size_t));
  printf("Run mul_ext random test\n");
  for (i = 0; i < trials; i++){
    a = DRAND() * C_SIZE_HALF;
    b = DRAND() * C_SIZE_HALF;
    mul_ext(a, b, &h, &l);
    res *= (h == 0);
    res *= (l == a * b);
  }
  printf("\t0 <= a, b <= 2**%lu - 1  --> ", TOLU(C_HALF_BIT));
  print_test_result(res);
  res = 1;
  for (i = 0; i < trials; i++){
    a = 1 + DRAND() * (C_SIZE_MAX - 1);
    b = 1 + DRAND() * (C_SIZE_MAX - 1);
    n = 1 + DRAND() * (C_SIZE_MAX - 1);
    mul_ext(a, b, &h, &l);
    hl[0] = l;
    hl[1] = h;
    res *= (sum_mod(hl[0] % n,
		    mul_mod(sum_mod(C_SIZE_MAX, 1, n), hl[1] % n, n),
		    n) == mul_mod(a, b, n));
  }
  printf("\t0 < a, b <= 2**%lu - 1 --> ", TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  mul_ext(0, 0, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(1, 0, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(0, 1, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(1, 1, &h, &l);
  res *= (h == 0 && l == 1);
  mul_ext(pow_two_perror(C_HALF_BIT),
	  pow_two_perror(C_HALF_BIT),
	  &h,
	  &l);
  res *= (h == 1 && l == 0);
  mul_ext(pow_two_perror(C_FULL_BIT - 1),
	  pow_two_perror(C_FULL_BIT - 1),
	  &h,
	  &l);
  res *= (h == pow_two_perror(C_FULL_BIT - 2) && l == 0);
  mul_ext(C_SIZE_MAX, C_SIZE_MAX, &h, &l);
  res *= (h == C_SIZE_MAX - 1 && l == 1);
  printf("\tcorner cases --> ");
  print_test_result(res);
  free(hl);
  hl = NULL;
}

/**
   Tests represent_uint.
*/
void run_represent_uint_test(int log_trials){
  int res = 1;
  int i, trials;
  size_t n, k, u;
  size_t j;
  trials = pow_two_perror(log_trials);
  printf("Run represent_uint odds test --> ");
  for (i = 0; i < trials; i++){
    n = RANDOM();
    while (!(n & 1)){
      n = RANDOM();
    }
    represent_uint(n, &k, &u);
    res *= (k == 0 && u == n);
  }
  print_test_result(res);
  res = 1;
  printf("Run represent_uint odds * 2**k test --> ");
  for (i = 0; i < trials; i++){
    for (j = 0; j <= C_FULL_BIT - C_BYTE_BIT; j++){
      n = RANDOM() % C_UCHAR_MAX;
      if (!(n & 1)) n++; /* <= C_UCHAR_MAX as size_t */
      represent_uint(pow_two_perror(j) * n, &k, &u);
      res *= (k == j && u == n);
    }
  }
  print_test_result(res);
  res = 1;
  printf("Run represent_uint corner cases test --> ");
  represent_uint(0, &k, &u);
  res *= (k == C_FULL_BIT && u == 0);
  represent_uint(1, &k, &u);
  res *= (k == 0 && u == 1);
  print_test_result(res);
}

/**
   Tests pow_two.
*/
void run_pow_two_test(){
  int res = 1;
  int i, trials = C_FULL_BIT;
  size_t prod = 1;
  for (i = 0; i < trials; i++){
    res *= (prod == pow_two_perror(i));
    prod *= 2;
  }
  printf("Run pow_two test --> ");
  print_test_result(res);
}

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
  if (argc > C_ARGC_MAX){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_MAX - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_MAX - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] > C_FULL_BIT - 1 ||
      args[1] > C_FULL_BIT - 1 ||
      args[2] > C_FULL_BIT - 1 ||
      args[1] > args[2] ||
      args[3] > 1 ||
      args[4] > 1 ||
      args[5] > 1){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[3]){
    run_pow_mod_test(args[0]);
    run_mul_mod_test(args[0]);
    run_sum_mod_test(args[0]);
  }
  if (args[4]) run_mem_mod_test(args[1], args[2]);
  if (args[5]){
    run_mul_ext_test(args[0]);
    run_represent_uint_test(args[0]);
    run_pow_two_test();
  }
  free(args);
  args = NULL;
  return 0;
}
