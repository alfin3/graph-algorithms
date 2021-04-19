/**
   utilities-mod-main.c

   Tests of utility functions in modular arithmetic.

   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "utilities-mem.h"
#include "utilities-mod.h"

/**
   Generate random numbers in a portable way for test purposes only; rand()
   in the Linux C Library uses the same generator as random(), which may not
   be the case on older rand() implementations, and on current
   implementations on different systems.
*/
#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand()) /* [0, RAND_MAX] */
#define DRAND() ((double)rand() / RAND_MAX) /* [0.0, 1.0] */

const uint64_t CT_UINT64_MAX = (uint64_t)-1;

void print_test_result(int res);

/**
   Tests pow_mod.
*/
void run_pow_mod_test(){
  int res = 1;
  uint64_t i, trials = 1000000;
  uint64_t upper_a = 16, upper_k = 17, upper_n = pow_two(32) - 2;
  uint64_t j, k;
  uint64_t a, n;
  uint64_t r, r_wo;
  printf("Run pow_mod random test\n ");
  printf("\t0 <= a <= 15, 0 <= k <= 16, 0 < n <= 2^32 - 1 --> ");
  for (i = 0; i < trials; i++){
    a = RANDOM() % upper_a;
    k = RANDOM() % upper_k;
    n = 1 + DRAND() * upper_n;
    r = pow_mod(a, k, n);
    r_wo = 1;
    for (j = 0; j < k; j++){
      r_wo *= a;
    }
    r_wo = r_wo % n;
    res *= (r == r_wo);
  }
  print_test_result(res);
  res = 1;
  printf("\ta = n - 1, 0 <= k < 2^64 - 1, where 0 = k (mod 2), "
	 "1 < n <= 2^64 - 1 --> ");
  upper_k = CT_UINT64_MAX - 1;
  upper_n = CT_UINT64_MAX - 2;
  for (i = 0; i < trials; i++){
    k = DRAND() * upper_k;
    while (k & 1){
      k = DRAND() * upper_k;
    }
    n = 2 + DRAND() * upper_n;
    a = n - 1;
    r = pow_mod(a, k, n);
    res *= (r == 1);
  }
  print_test_result(res);
  res = 1;
  printf("\tcorner cases --> ");
  upper_n = CT_UINT64_MAX;
  res *= (pow_mod(0, 0, 1) == 0);
  res *= (pow_mod(2, 0, 1) == 0);
  res *= (pow_mod(0, 0, 2) == 1);
  res *= (pow_mod(2, 0, 2) == 1);
  res *= (pow_mod(upper_n, upper_n, upper_n) == 0);
  res *= (pow_mod(upper_n - 1, upper_n, upper_n) == upper_n - 1);
  res *= (pow_mod(upper_n, upper_n - 1, upper_n) == 0);
  print_test_result(res);
}

/**
   Tests mul_mod.
*/
void run_mul_mod_test(){
  int res = 1;
  uint64_t i, trials = 1000000;
  uint64_t upper_a = pow_two(32) - 1;
  uint64_t upper_b = pow_two(32) - 1;
  uint64_t upper_n = CT_UINT64_MAX - 1;
  uint64_t a, b, n;
  uint64_t r, r_wo;
  printf("Run mul_mod random test\n");
  printf("\ta, b <= 2^32 - 1, 0 < n <= 2^64 - 1 --> ");
  for (i = 0; i < trials; i++){
    a = DRAND() * upper_a;
    b = DRAND() * upper_b;
    n = 1 + DRAND() * upper_n;
    r = mul_mod(a, b, n);
    r_wo = (a * b) % n;
    res *= (r == r_wo);
  }
  print_test_result(res);
  res = 1;
  printf("\ta, b = n - 1, 1 < n <= 2^64 - 1 --> ");
  for (i = 0; i < trials; i++){
    n = 2 + DRAND() * (upper_n - 1);
    r = mul_mod(n - 1, n - 1, n);
    res *= (r == 1);
  }
  print_test_result(res);
  res = 1;
  printf("\tcorner cases --> ");
  res *= (mul_mod(0, 0, 1) == 0);
  res *= (mul_mod(1, 0, 2) == 0);
  res *= (mul_mod(0, 1, 2) == 0);
  res *= (mul_mod(0, 2, 2) == 0);
  res *= (mul_mod(1, 1, 2) == 1);
  res *= (mul_mod(0, upper_n - 1, upper_n) == 0);
  res *= (mul_mod(upper_n - 1, 0, upper_n) == 0);
  res *= (mul_mod(upper_n - 1, 1, upper_n) == upper_n - 1);
  res *= (mul_mod(1, upper_n - 1, upper_n) == upper_n - 1);
  res *= (mul_mod(upper_n - 1, upper_n - 1, upper_n - 1) == 0);
  res *= (mul_mod(upper_n - 1, upper_n - 1, upper_n) == 1);
  res *= (mul_mod(upper_n, upper_n, upper_n) == 0);
  print_test_result(res);
}

/**
   Tests sum_mod.
*/
void run_sum_mod_test(){
  int res = 1;
  uint64_t i, trials = 1000000;
  uint64_t upper_a = pow_two(63) - 1;
  uint64_t upper_b = pow_two(63) - 1;
  uint64_t upper_n = CT_UINT64_MAX - 1;
  uint64_t a, b, n;
  uint64_t r, r_wo;
  printf("Run sum_mod random test\n");
  printf("\ta, b <= 2^63 - 1 (mod n), 0 < n <= 2^64 - 1 --> ");
  for (i = 0; i < trials; i++){
    a = DRAND() * upper_a;
    b = DRAND() * upper_b;
    n = 1 + DRAND() * upper_n;
    r = sum_mod(a, b, n);
    r_wo = (a + b) % n;
    res *= (r == r_wo);
  }
  print_test_result(res);
  res = 1;
  printf("\ta = 2^64 - 2, 0 < b <= 2^64 - 1, n = 2^64 - 1 --> ");
  for (i = 0; i < trials; i++){
    b = 1 + DRAND() * upper_n;
    r = sum_mod(upper_n, b, upper_n + 1);
    res *= (r == b - 1);
  }
  print_test_result(res);
  res = 1;
  printf("\tcorner cases --> ");
  res *= (sum_mod(0, 0, 1) == 0);
  res *= (sum_mod(1, 0, 2) == 1);
  res *= (sum_mod(0, 1, 2) == 1);
  res *= (sum_mod(1, 1, 2) == 0);
  res *= (sum_mod(upper_n - 1, upper_n - 1, upper_n) == upper_n - 2);
  print_test_result(res);
}

/**
   Tests mem_mod. A little-endian machine is assumed for test purposes.
*/
void run_mem_mod_test(){
  int res = 1;
  int j;
  unsigned char *block = NULL;
  uint64_t i, trials = 1000000;
  uint64_t upper = CT_UINT64_MAX - 1;
  uint64_t num, n, mod_n;
  uint64_t size;
  clock_t t;
  size = sizeof(uint64_t);
  printf("Run mem_mod in a random test, size = %lu bytes  --> ", size);
  for (i = 0; i < trials; i++){
    num = DRAND() * upper;
    n = 1 + DRAND() * upper;
    res *= (num % n == mem_mod(&num, size, n));
  }
  print_test_result(res);
  res = 1;
  printf("Run mem_mod on large memory blocks \n");
  n = 1 + DRAND() * upper;
  for (j = 10; j <= 20; j += 10){
    size = pow_two(j); /* KB, MB */
    printf("\tmemory block size: %lu bytes \n", size);
    block = calloc_perror(1, size);
    block[size - 1] = (unsigned char)pow_two(7);
    t = clock();
    mod_n = mem_mod(block, size, n);
    t = clock() - t;
    printf("\truntime: %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    res = (mod_n == pow_mod(2, pow_two(j) * 8 - 1, n));
    printf("\tcorrectness: block = %lu (mod %lu)  --> ", mod_n, n);
    print_test_result(res);
    res = 1;
    free(block);
    block = NULL;
  } 
}

/**
   Tests fast_mem_mod. A little-endian machine is assumed for test purposes.
*/
void run_fast_mem_mod_test(){
  int res = 1;
  int j;
  unsigned char *block = NULL;
  uint64_t i, trials = 10000;
  uint64_t upper = CT_UINT64_MAX - 1;
  uint64_t num, n, mod_n;
  uint64_t k, size;
  clock_t t;
  size = sizeof(uint64_t);
  printf("Run fast_mem_mod in a random test, size = %lu bytes  --> ", size);
  for (i = 0; i < trials; i++){
    num = DRAND() * upper;
    n = 1 + DRAND() * upper;
    res *= (num % n == fast_mem_mod(&num, size, n));
  }
  print_test_result(res);
  res = 1;
  printf("Run fast_mem_mod on large memory blocks, n <= 2^32 - 1 \n");
  upper = pow_two(32) - 2;
  n = 1 + DRAND() * upper;
  for (j = 10; j <= 30; j += 10){
    size = pow_two(j); /* KB, MB, GB */
    printf("\tmemory block size: %lu bytes \n", size);
    block = calloc_perror(1, size);
    block[size - 1] = (unsigned char)pow_two(7);
    t = clock();
    mod_n = fast_mem_mod(block, size, n);
    t = clock() - t;
    printf("\truntime: %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    res = (mod_n == pow_mod(2, pow_two(j) * 8 - 1, n));
    printf("\tcorrectness: block = %lu (mod %lu)  --> ", mod_n, n);
    print_test_result(res);
    res = 1;
    free(block);
    block = NULL;
  }
  printf("Run fast_mem_mod on large memory blocks, "
	 "2^32 - 1 < n <= 2^64 - 1 \n");
  upper = CT_UINT64_MAX;
  n = pow_two(32) + DRAND() * (upper - pow_two(32));
  for (j = 10; j <= 30; j += 10){
    size = pow_two(j); /* KB, MB, GB */
    printf("\tmemory block size: %lu bytes \n", size);
    block = calloc_perror(1, size);
    block[size - 1] = (unsigned char)pow_two(7);
    t = clock();
    mod_n = fast_mem_mod(block, size, n);
    t = clock() - t;
    printf("\truntime: %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    res = (mod_n == pow_mod(2, pow_two(j) * 8 - 1, n));
    printf("\tcorrectness: block = %lu (mod %lu)  --> ", mod_n, n);
    print_test_result(res);
    res = 1;
    free(block);
    block = NULL;
  } 
  printf("Run fast_mem_mod and mem_mod comparison on random blocks "
	 "of random size --> ");
  fflush(stdout);
  upper = CT_UINT64_MAX - 1;
  for (i = 0; i < trials; i++){
    size = 1 + DRAND() * (pow_two(10) - 1);
    block = calloc_perror(1, size);
    n = 1 + DRAND() * upper;
    for (k = 0; k < size; k++){
      block[k] = DRAND() * (pow_two(8) - 1);
    }  
    res *= (fast_mem_mod(block, size, n) == mem_mod(block, size, n));
    free(block);
    block = NULL;
  }
  print_test_result(res);
}

/**
   Tests mul_mod_pow_two. The test assumes wrapping around of unsigned
   integers.
*/
void run_mul_mod_pow_two_test(){
  int res = 1;
  uint64_t i, trials = 1000000;
  uint64_t upper = pow_two(32) - 1;
  uint64_t a, b, h, l, ret;
  printf("Run mul_mod_pow_two random test\n");
  printf("\t0 <= a, b <= 2^32 - 1  --> ");
  for (i = 0; i < trials; i++){
    a = DRAND() * upper;
    b = DRAND() * upper;
    ret = mul_mod_pow_two(a, b);
    res *= (ret == a * b);
  }
  print_test_result(res);
  res = 1;
  printf("\t0 < a, b <= 2^64 - 1 --> ");
  upper = CT_UINT64_MAX - 1;
  for (i = 0; i < trials; i++){
    a = 1 + DRAND() * upper;
    b = 1 + DRAND() * upper;
    mul_ext(a, b, &h, &l);
    ret = mul_mod_pow_two(a, b);
    res *= (ret == l && ret == a * b);
  }
  print_test_result(res);
  res = 1;
  printf("\tcorner cases --> ");
  res *= (mul_mod_pow_two(0, 0) == 0);
  res *= (mul_mod_pow_two(1, 0) == 0);
  res *= (mul_mod_pow_two(0, 1) == 0);
  res *= (mul_mod_pow_two(1, 1) == 1);
  res *= (mul_mod_pow_two(pow_two(32), pow_two(32)) == 0);
  res *= (mul_mod_pow_two(pow_two(63), pow_two(63)) == 0);
  res *= (mul_mod_pow_two(CT_UINT64_MAX, CT_UINT64_MAX) == 1);
  print_test_result(res);
}

/**
   Tests mul_ext.
*/
void run_mul_ext_test(){
  int res = 1;
  uint64_t i, trials = 1000000;
  uint64_t upper = pow_two(32) - 1;
  uint64_t a, b, n, h, l;
  uint64_t *hl = malloc_perror(2, sizeof(uint64_t));
  printf("Run mul_ext random test\n");
  printf("\t0 <= a, b <= 2^32 - 1  --> ");
  for (i = 0; i < trials; i++){
    a = DRAND() * upper;
    b = DRAND() * upper;
    mul_ext(a, b, &h, &l);
    res *= (h == 0);
    res *= (l == a * b);
  }
  print_test_result(res);
  res = 1;
  printf("\t0 < a, b <= 2^64 - 1 --> ");
  upper = CT_UINT64_MAX - 1;
  for (i = 0; i < trials; i++){
    a = 1 + DRAND() * upper;
    b = 1 + DRAND() * upper;
    n = 1 + DRAND() * upper;
    mul_ext(a, b, &h, &l);
    hl[0] = l;
    hl[1] = h;
    res *= (mem_mod(hl, 2 * sizeof(uint64_t), n) == mul_mod(a, b, n));
  }
  print_test_result(res);
  res = 1;
  printf("\tcorner cases --> ");
  mul_ext(0, 0, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(1, 0, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(0, 1, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(1, 1, &h, &l);
  res *= (h == 0 && l == 1);
  mul_ext(pow_two(32), pow_two(32), &h, &l);
  res *= (h == 1 && l == 0);
  mul_ext(pow_two(63), pow_two(63), &h, &l);
  res *= (h == pow_two(62) && l == 0);
  mul_ext(CT_UINT64_MAX, CT_UINT64_MAX, &h, &l);
  res *= (h == CT_UINT64_MAX - 1 && l == 1);
  print_test_result(res);
}

/**
   Tests represent_uint.
*/
void run_represent_uint_test(){
  int res = 1;
  int i, count = 15;
  uint64_t n, k, u;
  uint64_t j, upper_k = 16;
  uint64_t primes[15] = {2, 3, 5, 7, 11, 
                        13, 17, 19, 23, 29, 
                        103991, 103993, 103997, 104003, 104009};
  uint64_t odds[15] = {9, 15, 21, 25, 27,
		      33, 35, 39, 45, 49,
		      103999, 104001, 104005, 104023, 104025};
  printf("Run represent_uint primes test --> ");
  for (i = 0; i < count; i++){
    represent_uint(primes[i], &k, &u);
    if (primes[i] == 2){
      res *= (k == 1 && u == 1);
    }else{
      res *= (k == 0 && u == primes[i]);
    }
  }
  print_test_result(res);
  res = 1;
  printf("Run represent_uint odds test --> ");
  for (i = 0; i < count; i++){
    represent_uint(odds[i], &k, &u);
    res *= (k == 0 && u == odds[i]);
  }
  print_test_result(res);
  res = 1;
  printf("Run represent_uint odds * 2^k test --> ");
  for (i = 0; i < count; i++){
    for (j = 0; j < upper_k; j++){
      n = pow_two(j) * odds[i];
      represent_uint(n, &k, &u);
      res *= (k == j && u == odds[i]);
    }
  }
  print_test_result(res);
  res = 1;
  printf("Run represent_uint corner cases test --> ");
  represent_uint(0, &k, &u);
  res *= (k == 64 && u == 0);
  represent_uint(1, &k, &u);
  res *= (k == 0 && u == 1);
  print_test_result(res);
}

/**
   Tests pow_two.
*/
void run_pow_two_test(){
  int res = 1;
  int i, trials = 64;
  uint64_t prod = 1;
  for (i = 0; i < trials; i++){
    res *= (prod == pow_two(i));
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

int main(){
  RGENS_SEED();
  run_pow_mod_test();
  run_mul_mod_test();
  run_sum_mod_test();
  run_mem_mod_test();
  run_fast_mem_mod_test();
  run_mul_mod_pow_two_test();
  run_mul_ext_test();
  run_represent_uint_test();
  run_pow_two_test();
  return 0;
}
