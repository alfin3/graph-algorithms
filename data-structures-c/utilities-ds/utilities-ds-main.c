/**
   utilities-ds-main.c

   Examples of utility functions across the areas of randomness,
   modular arithmetic, and binary representation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include "utilities-ds.h"

void print_test_result(int result);

/** Randomness */

/**
   Tests random_uint64.
*/
void run_random_uint64_test(){
  int num_trials = 10000000;
  int id_count[2] = {0, 0};
  int result;
  uint64_t rand_num;
  uint64_t threshold = pow_two_uint64(63);
  printf("Run random_uint64 test --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_uint64();
    if (rand_num < threshold){id_count[0]++;}
    if (rand_num >= threshold){id_count[1]++;}
  }
  result = (abs(id_count[0] - id_count[1]) < num_trials/1000);
  print_test_result(result);
}

/**
   Tests random_uint32.
*/
void run_random_uint32_test(){
  int num_trials = 10000000;
  int id_count[2] = {0, 0};
  int result;
  uint32_t rand_num;
  uint32_t threshold = (uint32_t)pow_two_uint64(31);
  printf("Run random_uint32 test --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_uint32();
    if (rand_num < threshold){id_count[0]++;}
    if (rand_num >= threshold){id_count[1]++;}
  }
  result = (abs(id_count[0] - id_count[1]) < num_trials/1000);
  print_test_result(result);
}

/**
   Test random_range_uint64.
*/
static void eq_split_uint64_test(uint64_t upper,
				 uint64_t threshold,
				 int num_trials,
				 int precision);

void run_random_range_uint64_test(){
  int num_trials = 10000000;
  int precision = 1000;
  int arr_len = 9;
  int upper_pow_two[9] = {1, 7, 15, 23, 31, 39, 47, 55, 63};
  uint64_t upper;
  uint64_t threshold;
  for (int i = 0; i < arr_len; i++){
    upper = pow_two_uint64(upper_pow_two[i]) - 1;
    threshold = pow_two_uint64(upper_pow_two[i] - 1) - 1;
    printf("Run random_range_uint64 test, n = %lu --> ", upper);
    eq_split_uint64_test(upper, threshold, num_trials, precision);
  }
}

static void eq_split_uint64_test(uint64_t upper,
				 uint64_t threshold,
				 int num_trials,
				 int precision){
  int id_count[2] = {0, 0};
  int result;
  uint64_t rand_num;
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_range_uint64(upper);
    if (rand_num <= threshold){id_count[0]++;};
    if (rand_num > threshold && rand_num <= upper){id_count[1]++;}
  }
  result = (abs(id_count[0] - id_count[1]) < num_trials / precision &&
	    id_count[0] + id_count[1] == num_trials);
  print_test_result(result);
}

/**
   Test random_range_uint32.
*/
static void eq_split_uint32_test(uint32_t upper,
				 uint32_t threshold,
				 int num_trials,
				 int precision);

void run_random_range_uint32_test(){
  int num_trials = 10000000;
  int precision = 1000;
  int arr_len = 5;
  int upper_pow_two[5] = {1, 7, 15, 23, 31};
  uint32_t upper;
  uint32_t threshold;
  for (int i = 0; i < arr_len; i++){
    upper = (uint32_t)(pow_two_uint64(upper_pow_two[i]) - 1);
    threshold = (uint32_t)(pow_two_uint64(upper_pow_two[i] - 1) - 1);
    printf("Run random_range_uint32 test, n = %u --> ", upper);
    eq_split_uint32_test(upper, threshold, num_trials, precision);
  }
}

static void eq_split_uint32_test(uint32_t upper,
				 uint32_t threshold,
				 int num_trials,
				 int precision){
  int id_count[2] = {0, 0};
  int result;
  uint32_t rand_num;
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_range_uint32(upper);
    if (rand_num <= threshold){id_count[0]++;};
    if (rand_num > threshold && rand_num <= upper){id_count[1]++;}
  }
  result = (abs(id_count[0] - id_count[1]) < num_trials / precision &&
	    id_count[0] + id_count[1] == num_trials);
  print_test_result(result);
}

/**
   Test bern_uint64.
*/
static void bern_uint64_test_helper(uint64_t threshold,
				    uint64_t low,
			            uint64_t high,
			            int num_trials,
			            double p,
			            double precision);
  
void run_bern_uint64_test(){
  int num_trials = 10000000;
  double precision = 0.0005;
  double p[] = {0.5, 0.25, 0.125, 0.0625};
  int pow_two_high_start[] = {2, 3, 4, 5};
  int pow_two_diff[] = {1, 2, 3, 4};
  int arr_len = 4;
  uint64_t threshold;
  uint64_t low = 0;
  uint64_t high;
  for (int i = 0; i < arr_len; i++){
    printf("Run bern_uint64 p = %lf test\n ", p[i]);
    for (int j = pow_two_high_start[i]; j < 64; j += 8){
      threshold = pow_two_uint64(j - pow_two_diff[i]);
      high = pow_two_uint64(j);
      bern_uint64_test_helper(threshold, low, high,
			      num_trials, p[i], precision);
    }
  }
}

static void bern_uint64_test_helper(uint64_t threshold,
				    uint64_t low,
			            uint64_t high,
			            int num_trials,
			            double p,
			            double precision){
  int id_count = 0;
  int result;
  double gen_p;
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    if(bern_uint64(threshold, low, high)){id_count++;}
  }
  printf("\t[%lu, %lu], true: %d, false: %d --> ",
	 low, high, id_count, num_trials - id_count);
  gen_p = (double)id_count / (double)num_trials;
  result = (gen_p < p + precision && gen_p > p - precision);
  print_test_result(result);
}

/**
   Test bern_uint32.
*/

static void bern_uint32_test_helper(uint32_t threshold,
				    uint32_t low,
			            uint32_t high,
			            int num_trials,
			            double p,
			            double precision);

void run_bern_uint32_test(){
  int num_trials = 10000000;
  double precision = 0.0005;
  double p[] = {0.5, 0.25, 0.125, 0.0625};
  int pow_two_high_start[] = {2, 3, 4, 5};
  int pow_two_diff[] = {1, 2, 3, 4};
  int arr_len = 4;
  uint32_t threshold;
  uint32_t low = 0;
  uint32_t high;
  for (int i = 0; i < arr_len; i++){
    printf("Run bern_uint32 p = %lf test\n ", p[i]);
    for (int j = pow_two_high_start[i]; j < 32; j += 4){
      threshold = (uint32_t)pow_two_uint64(j - pow_two_diff[i]);
      high = (uint32_t)pow_two_uint64(j);
      bern_uint32_test_helper(threshold, low, high,
			      num_trials, p[i], precision);
    }
  }
}

static void bern_uint32_test_helper(uint32_t threshold,
				    uint32_t low,
			            uint32_t high,
			            int num_trials,
			            double p,
			            double precision){
  int id_count = 0;
  int result;
  double gen_p;
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    if(bern_uint32(threshold, low, high)){id_count++;}
  }
  printf("\t[%u, %u], true: %d, false: %d --> ",
	 low, high, id_count, num_trials - id_count);
  gen_p = (double)id_count / (double)num_trials;
  result = (gen_p < p + precision && gen_p > p - precision);
  print_test_result(result);
}

/** Modular arithmetic */

/**
   Tests pow_mod_uint64.
*/
void run_pow_mod_uint64_test(){
  int num_trials = 100000;
  int result = 1;
  uint64_t upper_a = 15;
  uint64_t upper_k = 16;
  uint64_t upper_n = pow_two_uint64(32) - 2;
  uint64_t rand_a;
  uint64_t rand_k;
  uint64_t rand_n;
  uint64_t r;
  uint64_t r_wo_pow;
  printf("Run pow_mod_uint64 random test\n ");
  printf("\t0 <= a <= 15, 0 <= k <= 16, 0 < n <= 2^32 - 1 --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint64(upper_a);
    rand_n = 1 + random_range_uint64(upper_n);
    rand_k = random_range_uint64(upper_k);
    r = pow_mod_uint64(rand_a, rand_k, rand_n);
    r_wo_pow = 1;
    for (uint64_t i = 0; i < rand_k; i++){
      r_wo_pow *= rand_a;
    }
    r_wo_pow = r_wo_pow % rand_n;
    result *= (r == r_wo_pow);
  }
  print_test_result(result);
  printf("\ta = x^2, where 0 <= x <= 2^32 - 1, 0 <= k <= 2^64 - 1, "
	 "0 < n <= 2^32 - 1 --> ");
  upper_a = pow_two_uint64(32) - 1;
  upper_k = (pow_two_uint64(63) - 1) + pow_two_uint64(63);
  upper_n = pow_two_uint64(32) - 2;
  srandom(time(0));
  result = 1;
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint64(upper_a);
    rand_k = random_range_uint64(upper_k);
    rand_n = 1 + random_range_uint64(upper_n);
    r = pow_mod_uint64(rand_a * rand_a, rand_k, rand_n);
    r_wo_pow = (uint64_t)pow_mod_uint32((uint32_t)rand_a,
					rand_k,
					(uint32_t)rand_n);
    result *= (r == (r_wo_pow * r_wo_pow) % rand_n);
  }
  print_test_result(result);
  printf("\ta = n - 1, 0 <= k <= 2^64 - 1, where 0 = k (mod 2), "
	 "1 < n <= 2^64 - 1 --> ");
  upper_k = (pow_two_uint64(63) - 1) + pow_two_uint64(63);
  upper_n = (pow_two_uint64(63) - 1) + (pow_two_uint64(63) - 2);
  srandom(time(0));
  result = 1;
  for (int i = 0; i < num_trials; i++){
    rand_k = random_range_uint64(upper_k);
    while (rand_k & 1){
      rand_k = random_range_uint64(upper_k);
    }
    rand_n = 2 + random_range_uint64(upper_n);
    rand_a = rand_n - 1;
    r = pow_mod_uint64(rand_a, rand_k, rand_n);
    result *= (r == 1);
  }
  print_test_result(result);
  printf("\tcorner cases --> ");
  result = 1;
  upper_n = (pow_two_uint64(63) - 1) + pow_two_uint64(63);
  result *= (pow_mod_uint64(0, 0, 1) == 0);
  result *= (pow_mod_uint64(2, 0, 1) == 0);
  result *= (pow_mod_uint64(0, 0, 2) == 1);
  result *= (pow_mod_uint64(2, 0, 2) == 1);
  result *= (pow_mod_uint64(upper_n, upper_n, upper_n) == 0);
  result *= (pow_mod_uint64(upper_n - 1, upper_n, upper_n) == upper_n - 1);
  result *= (pow_mod_uint64(upper_n, upper_n - 1, upper_n) == 0);
  print_test_result(result);
}

/**
   Tests pow_mod_uint32.
*/
void run_pow_mod_uint32_test(){
  int num_trials = 1000000;
  int result = 1;
  uint32_t upper_a = 15;
  uint32_t upper_n = (uint32_t)(pow_two_uint64(32) - 1);
  uint32_t upper_k = 16;
  uint32_t rand_a;
  uint32_t rand_n;
  uint64_t rand_k; 
  uint64_t r;
  uint64_t r_wo_pow;
  printf("Run pow_mod_uint32 random test \n");
  printf("\t0 <= a <= 15, 0 <= k <= 16, 0 < n <= 2^32 - 1 --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint32(upper_a);
    rand_n = random_range_uint32(upper_n);
    rand_k = (uint64_t)random_range_uint32(upper_k);
    r = (uint64_t)pow_mod_uint32(rand_a, rand_k, rand_n);
    r_wo_pow = 1;
    for (uint64_t i = 0; i < rand_k; i++){
      r_wo_pow *= (uint64_t)rand_a;
    }
    r_wo_pow = r_wo_pow % (uint64_t)rand_n;
    result *= (r == r_wo_pow);
  }
  print_test_result(result);
  printf("\tcorner cases --> ");
  result = 1;
  result *= (pow_mod_uint32(0, 0, 1) == 0);
  result *= (pow_mod_uint32(2, 0, 1) == 0);
  result *= (pow_mod_uint32(0, 0, 2) == 1);
  result *= (pow_mod_uint32(2, 0, 2) == 1);
  result *= (pow_mod_uint32(4294967295, 4294967295, 4294967295) == 0);
  result *= (pow_mod_uint32(4294967294, 4294967295, 4294967295) == 4294967294);
  print_test_result(result);
}

/**
   Tests mul_mod_uint64.
*/
void run_mul_mod_uint64_test(){
  int num_trials = 1000000;
  int result = 1;
  uint64_t upper_a = pow_two_uint64(32) - 1;
  uint64_t upper_b = pow_two_uint64(32) - 1;
  uint64_t upper_n = (pow_two_uint64(63) - 1) + (pow_two_uint64(63) - 1);
  uint64_t rand_a;
  uint64_t rand_b;
  uint64_t rand_n;
  uint64_t r;
  uint64_t r_wo_mul_mod;
  printf("Run mul_mod_uint64 random test\n");
  printf("\ta, b <= 2^32 - 1, 0 < n <= 2^64 - 1 --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint64(upper_a);
    rand_b = random_range_uint64(upper_b);
    rand_n = 1 + random_range_uint64(upper_n); 
    r = mul_mod_uint64(rand_a, rand_b, rand_n);
    r_wo_mul_mod = (rand_a * rand_b) % rand_n;
    result *= (r == r_wo_mul_mod);
  }
  print_test_result(result);
  printf("\ta, b = n - 1, 1 < n <= 2^64 - 1 --> ");
  result = 1;
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_n = 2 + random_range_uint64(upper_n - 1);
    r = mul_mod_uint64(rand_n - 1, rand_n - 1, rand_n);
    result *= (r == 1);
  }
  print_test_result(result);
  printf("\tcorner cases --> ");
  result = 1;
  result *= (mul_mod_uint64(0, 0, 1) == 0);
  result *= (mul_mod_uint64(1, 0, 2) == 0);
  result *= (mul_mod_uint64(0, 1, 2) == 0);
  result *= (mul_mod_uint64(0, 2, 2) == 0);
  result *= (mul_mod_uint64(1, 1, 2) == 1);
  result *= (mul_mod_uint64(0, upper_n, upper_n + 1) == 0);
  result *= (mul_mod_uint64(upper_n, 0, upper_n + 1) == 0);
  result *= (mul_mod_uint64(upper_n, 1, upper_n + 1) == upper_n);
  result *= (mul_mod_uint64(1, upper_n, upper_n + 1) == upper_n);
  result *= (mul_mod_uint64(upper_n, upper_n, upper_n) == 0);
  result *= (mul_mod_uint64(upper_n, upper_n, upper_n + 1) == 1);
  print_test_result(result);
}

/**
   Tests sum_mod_uint64.
*/
void run_sum_mod_uint64_test(){
  int num_trials = 1000000;
  int result = 1;
  uint64_t upper_a = pow_two_uint64(63) - 1;
  uint64_t upper_b = pow_two_uint64(63) - 1;
  uint64_t upper_n = (pow_two_uint64(63) - 1) + (pow_two_uint64(63) - 1);
  uint64_t rand_a;
  uint64_t rand_b;
  uint64_t rand_n;
  uint64_t r;
  uint64_t r_wo_sum_mod;
  printf("Run sum_mod_uint64 random test\n");
  printf("\ta, b <= 2^63 - 1 (mod n), 0 < n <= 2^64 - 1 --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint64(upper_a);
    rand_b = random_range_uint64(upper_b);
    rand_n = 1 + random_range_uint64(upper_n);
    rand_a = rand_a % rand_n;
    rand_b = rand_b % rand_n; 
    r = sum_mod_uint64(rand_a, rand_b, rand_n);
    r_wo_sum_mod = (rand_a + rand_b) % rand_n;
    result *= (r == r_wo_sum_mod);
  }
  print_test_result(result);
  printf("\ta = 2^64 - 2, 0 < b <= 2^64 - 1, n = 2^64 - 1 --> ");
  result = 1;
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_b = 1 + random_range_uint64(upper_n); //[1, 2^64 - 1]
    r = sum_mod_uint64(upper_n, rand_b, upper_n + 1);
    result *= (r == rand_b - 1);
  }
  print_test_result(result);
  printf("\tcorner cases --> ");
  result = 1;
  result *= (sum_mod_uint64(0, 0, 1) == 0);
  result *= (sum_mod_uint64(1, 0, 2) == 1);
  result *= (sum_mod_uint64(0, 1, 2) == 1);
  result *= (sum_mod_uint64(1, 1, 2) == 0);
  result *= (sum_mod_uint64(upper_n, upper_n, upper_n + 1) == upper_n - 1);
  print_test_result(result);
}

/**
   Tests mem_mod_uint64. A little-endian machine is assumed for test purposes.
*/
void run_mem_mod_uint64_test(){
  int num_trials = 10000;
  int result = 1;
  uint8_t *mem_block;
  uint64_t upper = (pow_two_uint64(63) - 1) + pow_two_uint64(63);
  uint64_t rand_num;
  uint64_t rand_n;
  uint64_t mod_n;
  uint64_t size;
  clock_t t;
  size = 8;
  printf("Run mem_mod_uint64 in a random test, size = %lu bytes  --> ", size);
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_range_uint64(upper);
    rand_n = random_range_uint64(upper);
    result *= (rand_num % rand_n == mem_mod_uint64(&rand_num, size, rand_n));
  }
  print_test_result(result);
  printf("Run mem_mod_uint64 on large memory blocks \n");
  srandom(time(0));
  result = 1;
  rand_n = random_range_uint64(upper);
  for (int i = 10; i <= 20; i += 10){
    size = pow_two_uint64(i); //KB, MB
    printf("\tmemory block size: %lu bytes \n", size);
    mem_block = calloc(size, 1);
    assert(mem_block != NULL);
    mem_block[size - 1] = (uint8_t)pow_two_uint64(7);
    t = clock();
    mod_n = mem_mod_uint64(mem_block, size, rand_n);
    t = clock() - t;
    printf("\truntime: %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    result = (mod_n == pow_mod_uint64(2, pow_two_uint64(i) * 8 - 1, rand_n));
    printf("\tcorrectness: block bits = %lu (mod %lu)  --> ", mod_n, rand_n);
    print_test_result(result);
    free(mem_block);
  } 
}

/**
   Tests fast_mem_mod_uint64. A little-endian machine is assumed for 
   test purposes.
*/
void run_fast_mem_mod_uint64_test(){
  int num_trials = 10000;
  int result = 1;
  uint8_t *mem_block;
  uint64_t upper = (pow_two_uint64(63) - 1) + pow_two_uint64(63);
  uint64_t upper_byte_val = (uint64_t)(pow_two_uint64(8) - 1);
  uint64_t rand_num;
  uint64_t rand_n;
  uint64_t mod_n;
  uint64_t size;
  clock_t t;
  size = 8;
  printf("Run fast_mem_mod_uint64 in a random test, size = %lu bytes  --> ",
	 size);
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_range_uint64(upper);
    rand_n = random_range_uint64(upper);
    result *= (rand_num % rand_n ==
	       fast_mem_mod_uint64(&rand_num, size, rand_n));
  }
  print_test_result(result);
  printf("Run fast_mem_mod_uint64 on large memory blocks \n");
  srandom(time(0));
  result = 1;
  rand_n = random_range_uint64(upper);
  for (int i = 10; i <= 20; i += 10){
    size = pow_two_uint64(i); //KB, MB
    printf("\tmemory block size: %lu bytes \n", size);
    mem_block = calloc(size, 1);
    assert(mem_block != NULL);
    mem_block[size - 1] = (uint8_t)pow_two_uint64(7);
    t = clock();
    mod_n = fast_mem_mod_uint64(mem_block, size, rand_n);
    t = clock() - t;
    printf("\truntime: %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    result = (mod_n == pow_mod_uint64(2, pow_two_uint64(i) * 8 - 1, rand_n));
    printf("\tcorrectness: block bits = %lu (mod %lu)  --> ", mod_n, rand_n);
    print_test_result(result);
    free(mem_block);
  } 
  printf("Run fast_mem_mod_uint64 and mem_mod_uint64 comparison "
	 "on random blocks of random size --> ");
  fflush(stdout);
  srandom(time(0));
  result = 1;
  for (int i = 0; i < num_trials; i++){
    size = random_range_uint64(pow_two_uint64(10));
    mem_block = calloc(size, 1);
    assert(mem_block != NULL);
    rand_n = random_range_uint64(upper);
    for (uint64_t i = 0; i < size; i++){
      mem_block[i] = (uint8_t)random_range_uint64(upper_byte_val);
    }  
    result *= (fast_mem_mod_uint64(mem_block, size, rand_n) ==
	       mem_mod_uint64(mem_block, size, rand_n));
    free(mem_block);
  }
  print_test_result(result);
}

/**
   Tests mem_mod_uint32. A little-endian machine is assumed for test purposes.
*/
void run_mem_mod_uint32_test(){
  int num_trials = 1000000;
  int result = 1;
  uint8_t *mem_block;
  uint32_t upper = (uint32_t)(pow_two_uint64(32) - 1);
  uint32_t rand_num;
  uint32_t rand_n;
  uint32_t mod_n;
  uint64_t size;
  clock_t t;
  size = 4;
  printf("Run mem_mod_uint32 in a random test, size = %lu bytes  --> ", size);
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_range_uint32(upper);
    rand_n = random_range_uint32(upper);
    result *= (rand_num % rand_n == mem_mod_uint32(&rand_num, size, rand_n));
  }
  print_test_result(result);
  printf("Run mem_mod_uint32 on large memory blocks \n");
  srandom(time(0));
  result = 1;
  rand_n = random_range_uint32(upper);
  for (int i = 10; i <= 30; i += 10){
    size = pow_two_uint64(i); //KB, MB, GB
    printf("\tmemory block size: %lu bytes \n", size);
    mem_block = calloc(size, 1);
    assert(mem_block != NULL);
    mem_block[size - 1] = (uint8_t)pow_two_uint64(7);
    t = clock();
    mod_n = mem_mod_uint32(mem_block, size, rand_n);
    t = clock() - t;
    printf("\truntime: %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    result = (mod_n == pow_mod_uint32(2, pow_two_uint64(i) * 8 - 1, rand_n));
    printf("\tcorrectness: block bits = %u (mod %u)  --> ", mod_n, rand_n);
    print_test_result(result);
    free(mem_block);
  } 
}

/**
   Tests fast_mem_mod_uint32. A little-endian machine is assumed for 
   test purposes.
*/
void run_fast_mem_mod_uint32_test(){
  int num_trials = 100000;
  int result = 1;
  uint8_t *mem_block;
  uint32_t upper = (uint32_t)(pow_two_uint64(32) - 1);
  uint32_t upper_byte_val = (uint32_t)(pow_two_uint64(8) - 1);
  uint32_t rand_num;
  uint32_t rand_n;
  uint32_t mod_n;
  uint64_t size;
  clock_t t;
  size = 4;
  printf("Run fast_mem_mod_uint32 in a random test, "
	 "size = %lu bytes  --> ", size);
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_range_uint32(upper);
    rand_n = random_range_uint32(upper);
    result *= (rand_num % rand_n ==
	       fast_mem_mod_uint32(&rand_num, size, rand_n));
  }
  print_test_result(result);
  printf("Run fast_mem_mod_uint32 on large memory blocks \n");
  srandom(time(0));
  result = 1;
  rand_n = random_range_uint32(upper);
  for (int i = 10; i <= 30; i += 10){
    size = pow_two_uint64(i); //KB, MB, GB
    printf("\tmemory block size: %lu bytes \n", size);
    mem_block = calloc(size, 1);
    assert(mem_block != NULL);
    mem_block[size - 1] = (uint8_t)pow_two_uint64(7);
    t = clock();
    mod_n = fast_mem_mod_uint32(mem_block, size, rand_n);
    t = clock() - t;
    printf("\truntime: %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    result = (mod_n == pow_mod_uint32(2, pow_two_uint64(i) * 8 - 1, rand_n));
    printf("\tcorrectness: block bits = %u (mod %u)  --> ", mod_n, rand_n);
    print_test_result(result);
    free(mem_block);
  }
  printf("Run fast_mem_mod_uint32 and mem_mod_uint32 comparison "
	 "on random blocks of random size --> ");
  fflush(stdout);
  srandom(time(0));
  result = 1;
  for (int i = 0; i < num_trials; i++){
    size = random_range_uint64(pow_two_uint64(10));
    mem_block = calloc(size, 1);
    assert(mem_block != NULL);
    rand_n = random_range_uint32(upper);
    for (uint64_t i = 0; i < size; i++){
      mem_block[i] = (uint8_t)random_range_uint32(upper_byte_val);
    }  
    result *= (fast_mem_mod_uint32(mem_block, size, rand_n) ==
	       mem_mod_uint32(mem_block, size, rand_n));
    free(mem_block);
  }
  print_test_result(result);
}

/**
   Tests mul_mod_pow_two_64.
*/
void run_mul_mod_pow_two_64_test(){
  int num_trials = 1000000;
  int result = 1;
  uint64_t low_upper = pow_two_uint64(32) - 1;
  uint64_t upper = (pow_two_uint64(63) - 1) + pow_two_uint64(63);
  uint64_t rand_a, rand_b, h, l, ret;
  printf("Run mul_mod_pow_two_64 random test\n");
  printf("\ta, b <= 2^32 - 1  --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint64(low_upper);
    rand_b = random_range_uint64(low_upper);
    ret = mul_mod_pow_two_64(rand_a, rand_b);
    result *= (ret == rand_a * rand_b);
  }
  print_test_result(result);
  printf("\ta, b <= 2^64 - 1 --> ");
  result = 1;
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint64(upper);
    rand_b = random_range_uint64(upper);
    mul_uint64(rand_a, rand_b, &h, &l);
    ret = mul_mod_pow_two_64(rand_a, rand_b);
    result *= (ret == l);
  }
  print_test_result(result);
  printf("\tcorner cases --> ");
  result = 1;
  ret = mul_mod_pow_two_64(0, 0);
  result *= (ret == 0);
  ret = mul_mod_pow_two_64(1, 0);
  result *= (ret == 0);
  ret = mul_mod_pow_two_64(0, 1);
  result *= (ret == 0);
  ret = mul_mod_pow_two_64(1, 1);
  result *= (ret == 1);
  ret = mul_mod_pow_two_64(pow_two_uint64(32), pow_two_uint64(32));
  result *= (ret == 0);
  ret = mul_mod_pow_two_64(pow_two_uint64(63), pow_two_uint64(63));
  result *= (ret == 0);
  ret = mul_mod_pow_two_64(pow_two_uint64(63) + (pow_two_uint64(63) - 1),
			   pow_two_uint64(63) + (pow_two_uint64(63) - 1));
  result *= (ret == 1);
  print_test_result(result);
}

/** Binary representation */

/**
   Tests mul_mod_uint64.
*/
void run_mul_uint64_test(){
  int num_trials = 1000000;
  int result = 1;
  uint64_t low_upper = pow_two_uint64(32) - 1;
  uint64_t upper = (pow_two_uint64(63) - 1) + pow_two_uint64(63);
  uint64_t rand_a, rand_b, rand_n, h, l;
  uint64_t *hl = malloc(2 * sizeof(uint64_t));
  assert(hl != NULL);
  printf("Run mul_uint64 random test\n");
  printf("\ta, b <= 2^32 - 1  --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint64(low_upper);
    rand_b = random_range_uint64(low_upper);
    mul_uint64(rand_a, rand_b, &h, &l);
    result *= (h == 0);
    result *= (l == rand_a * rand_b);
  }
  print_test_result(result);
  printf("\ta, b <= 2^64 - 1 --> ");
  result = 1;
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint64(upper);
    rand_b = random_range_uint64(upper);
    rand_n = random_range_uint64(upper);
    mul_uint64(rand_a, rand_b, &h, &l);
    hl[0] = l;
    hl[1] = h;
    result *= (fast_mem_mod_uint64(hl, 2 * sizeof(uint64_t), rand_n) ==
	       mul_mod_uint64(rand_a, rand_b, rand_n));
  }
  print_test_result(result);
  printf("\tcorner cases --> ");
  result = 1;
  mul_uint64(0, 0, &h, &l);
  result *= (h == 0 && l == 0);
  mul_uint64(1, 0, &h, &l);
  result *= (h == 0 && l == 0);
  mul_uint64(0, 1, &h, &l);
  result *= (h == 0 && l == 0);
  mul_uint64(1, 1, &h, &l);
  result *= (h == 0 && l == 1);
  mul_uint64(pow_two_uint64(32), pow_two_uint64(32), &h, &l);
  result *= (h == 1 && l == 0);
  mul_uint64(pow_two_uint64(63), pow_two_uint64(63), &h, &l);
  result *= (h == pow_two_uint64(62) && l == 0);
  mul_uint64(pow_two_uint64(63) + (pow_two_uint64(63) - 1),
	     pow_two_uint64(63) + (pow_two_uint64(63) - 1), &h, &l);
  result *= (h == pow_two_uint64(63) + (pow_two_uint64(63) - 2) && l == 1);
  print_test_result(result);
}

/**
   Tests represent_uint64.
*/
void run_represent_uint64_test(){
  int k;
  int upper_k = 16;
  int result = 1;
  int arr_len = 15;
  uint64_t u;
  uint64_t primes[15] = {2, 3, 5, 7, 11, 
                        13, 17, 19, 23, 29, 
                        103991, 103993, 103997, 104003, 104009};
  uint64_t odds[15] = {9, 15, 21, 25, 27,
		      33, 35, 39, 45, 49,
		      103999, 104001, 104005, 104023, 104025};
  uint64_t num;
  printf("Run represent_uint64 primes test --> ");
  for (int i = 0; i < arr_len; i++){
    represent_uint64(primes[i], &k, &u);
    if (primes[i] == 2){
      result *= (k == 1 && u == 1);
    }else{
      result *= (k == 0 && u == primes[i]);
    }
  }
  print_test_result(result);
  result = 1;
  printf("Run represent_uint64 odds test --> ");
  for (int i = 0; i < arr_len; i++){
    represent_uint64(odds[i], &k, &u);
    result *= (k == 0 && u == odds[i]);
  }
  print_test_result(result);
  result = 1;
  printf("Run represent_uint64 odds * 2^k test --> ");
  for (int i = 0; i < arr_len; i++){
    for (int j = 0; j < upper_k; j++){
      num = pow_two_uint64(j) * odds[i];
      represent_uint64(num, &k, &u);
      result *= (k == j && u == odds[i]);
    }
  }
  print_test_result(result);
  result = 1;
  printf("Run represent_uint64 corner cases test --> ");
  represent_uint64(0, &k, &u);
  result *= (k == 64 && u == 0);
  represent_uint64(1, &k, &u);
  result *= (k == 0 && u == 1);
  print_test_result(result);
}

/**
   Tests pow_two_uint64.
*/
void run_pow_two_uint64_test(){
  int num_trials = 64;
  int result = 1;
  uint64_t prod = 1;
  for (int i = 0; i < num_trials; i++){
    result *= (prod == pow_two_uint64(i));
    prod *= 2;
  }
  printf("Run pow_two_uint64 test --> ");
  print_test_result(result);
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_random_uint64_test();
  run_random_uint32_test();
  run_random_range_uint64_test();
  run_random_range_uint32_test();
  run_bern_uint64_test();
  run_bern_uint32_test();
  run_pow_mod_uint64_test();
  run_pow_mod_uint32_test();
  run_mul_mod_uint64_test();
  run_sum_mod_uint64_test();
  run_mem_mod_uint64_test();
  run_fast_mem_mod_uint64_test();
  run_mem_mod_uint32_test();
  run_fast_mem_mod_uint32_test();
  run_mul_mod_pow_two_64_test();
  run_mul_uint64_test();
  run_represent_uint64_test();
  run_pow_two_uint64_test();
  return 0;
}
