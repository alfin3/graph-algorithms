/**
   utilities-rand-uint32-main.c

   Tests of randomness utility functions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "utilities-rand-uint32.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

static const uint32_t BYTE_BIT_COUNT = 8;
static const uint32_t FULL_BIT_COUNT = 8 * sizeof(uint32_t);
static const uint32_t HALF_BIT_COUNT = 4 * sizeof(uint32_t);
static const uint32_t UPPER_MAX = 0xffffffff;

void print_bit_probs(const uint32_t *counts, uint32_t trials);
void print_test_result(int res);

/**
   Tests random_range_uint32.
*/
void run_random_range_uint32_test(){
  uint32_t trials = 10000000;
  uint32_t n_low, n_mid, n_high;
  uint32_t upper_low, upper_mid, upper_high;
  uint32_t *counts_low = NULL, *counts_mid = NULL, *counts_high = NULL;
  uint32_t *masks = NULL;
  clock_t t, t_low, t_mid, t_high;
  counts_low = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  counts_mid = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  counts_high = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  masks = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
    masks[i] = pow_two(i);
  }
  printf("Run random_range_uint32 test, # trials = %u\n", trials);
  for (uint32_t i = 0; i < FULL_BIT_COUNT + 1; i++){
   if (i == 0){
      upper_high = pow_two(i);
      upper_mid = pow_two(i);
      upper_low = pow_two(i);
      printf("\n\tlow: [0, %u), mid: [0, %u), high: [0, %u)\n",
      	     upper_low, upper_mid, upper_high);
    }else if (i == 1){
      upper_low = pow_two(i - 1) + 1;
      upper_mid = pow_two(i - 1) + 1;
      upper_high = pow_two(i);
      printf("\n\tlow: [0, %u), mid: [0, %u), high: [0, %u)\n",
      	     upper_low, upper_mid, upper_high);
    }else if (i == FULL_BIT_COUNT){
      upper_low = pow_two(i - 1) + 1;
      upper_mid = pow_two(i - 1) + (UPPER_MAX - pow_two(i - 1)) / 2;
      upper_high = UPPER_MAX;
      printf("\n\tlow: [0, %u), mid: [0, %u), high: [0, %u)\n",
      	     upper_low, upper_mid, upper_high);
    }else{
      upper_low = pow_two(i - 1) + 1;
      upper_mid = pow_two(i - 1) + (pow_two(i) - pow_two(i - 1)) / 2;
      upper_high =  pow_two(i);
      printf("\n\tlow: [0, %u), mid: [0, %u), high: [0, %u)\n",
      	     upper_low, upper_mid, upper_high);
    }
    fflush(stdout);
    t = clock();
    for (uint32_t i = 0; i < trials; i++){
      UTILITIES_RAND_UINT32_RANDOM();
    }
    t = clock() - t;
    t_low = clock();
    for (uint32_t i = 0; i < trials; i++){
      random_range_uint32(upper_low);
    }
    t_low = clock() - t_low;
    t_mid = clock();
    for (uint32_t i = 0; i < trials; i++){
      random_range_uint32(upper_mid);
    }
    t_mid = clock() - t_mid;
    t_high = clock();
    for (uint32_t i = 0; i < trials; i++){
      random_range_uint32(upper_high);
    }
    t_high = clock() - t_high;
    for (uint32_t i = 0; i < trials; i++){
      n_low = random_range_uint32(upper_low);
      n_mid = random_range_uint32(upper_mid);
      n_high = random_range_uint32(upper_high);
      for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
	if (n_low & masks[i]) counts_low[i]++;
	if (n_mid & masks[i]) counts_mid[i]++;
	if (n_high & masks[i]) counts_high[i]++;
      }
    }
    printf("\t\tgenerator:                 %.8f seconds\n"
	   "\t\trandom_range_uint32 low:   %.8f seconds\n"
	   "\t\trandom_range_uint32 mid:   %.8f seconds\n"
	   "\t\trandom_range_uint32 high:  %.8f seconds\n",
	   (float)t / CLOCKS_PER_SEC,
	   (float)t_low / CLOCKS_PER_SEC,
	   (float)t_mid / CLOCKS_PER_SEC,
	   (float)t_high / CLOCKS_PER_SEC);
    printf("\t\tP[bit is set in low]:");
    print_bit_probs(counts_low, trials);
    printf("\t\tP[bit is set in mid]:");
    print_bit_probs(counts_mid, trials);
    printf("\t\tP[bit is set in high]:");
    print_bit_probs(counts_high, trials);
    memset(counts_low, 0, FULL_BIT_COUNT * sizeof(uint32_t));
    memset(counts_mid, 0, FULL_BIT_COUNT * sizeof(uint32_t));
    memset(counts_high, 0, FULL_BIT_COUNT * sizeof(uint32_t));
  }
  free(counts_low);
  free(counts_mid);
  free(counts_high);
  free(masks);
  counts_low = NULL;
  counts_mid = NULL;
  counts_high = NULL;
  masks = NULL;
}

/**
   Tests random_uint32.
*/
void run_random_uint32_test(){
  uint32_t trials_count = 7;
  uint32_t trials[7]= {100,
		       1000,
		       10000,
		       100000,
		       1000000,
		       10000000,
		       100000000};
  uint32_t n;
  uint32_t *counts = NULL;
  uint32_t *masks = NULL;
  clock_t t, t_rand;
  counts = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  masks = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
    masks[i] = pow_two(i);
  }
  printf("Run random_uint32 test\n");
  for (uint32_t ti = 0; ti < trials_count; ti++){
    printf("\t# trials = %u\n", trials[ti]);
    t = clock();
    for (uint32_t i = 0; i < trials[ti]; i++){
      UTILITIES_RAND_UINT32_RANDOM();
    }
    t = clock() - t;
    t_rand = clock();
    for (uint32_t i = 0; i < trials[ti]; i++){
      random_uint32();
    }
    t_rand = clock() - t_rand;
    for (uint32_t i = 0; i < trials[ti]; i++){
      n = random_uint32();
      for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
	if (n & masks[i]) counts[i]++;
      }
    }
    printf("\t\tgenerator:                 %.8f seconds\n"
	   "\t\trandom_uint32:             %.8f seconds\n",
	   (float)t / CLOCKS_PER_SEC,
	   (float)t_rand / CLOCKS_PER_SEC);
    printf("\t\tP[bit is set]:");
    print_bit_probs(counts, trials[ti]);
    memset(counts, 0, FULL_BIT_COUNT * sizeof(uint32_t));
  }
  free(counts);
  free(masks);
  counts = NULL;
  masks = NULL;
}

/**
   Tests the correctness of miller_rabin_uint32 on prime and composite
   numbers.
*/
void run_primality_test(){
  int res_comp = 0, res_prime = 1;
  int small_prime_count = 30;
  int small_comp_count = 30;
  int carmichael_count = 30;
  int trials_comp  = 100000;
  uint32_t upper, a, b;
  uint32_t small_prime_nums[30] = {2, 3, 5, 7, 11,
				   13, 17, 19, 23, 29,
				   31, 37, 41, 43, 47,
				   103991, 103993, 103997, 104003, 104009,
				   104021, 104033, 104047, 104053, 104059,
				   899809363, 920419813, 920419823,
				   941083981, 941083987};
  uint32_t small_comp_nums[30] = {0, 1, 4, 6, 8,
				  9, 10, 12, 14,
				  15, 16, 18, 20,
				  951, 952, 954, 955, 956,
				  957, 958, 959, 960, 961,
				  962, 963, 964, 965, 966};
  uint32_t carmichael_nums[30] = {561, 1105, 1729, 2465, 2821,
				  6601, 8911, 10585, 15841, 29341,
				  41041, 46657, 52633, 62745, 63973,
				  75361, 101101, 115921, 126217, 162401,
				  172081, 188461, 252601, 278545, 294409,
				  314821, 334153, 340561, 399001, 410041};
  printf("Run a miller_rabin_uint32 test on prime and composite numbers\n");
  for (int i = 0; i < small_prime_count; i++){
    res_prime *= miller_rabin_uint32(small_prime_nums[i]);
  }
  for (int i = 0; i < small_comp_count; i++){
    res_comp += miller_rabin_uint32(small_comp_nums[i]);
  }
  for (int i = 0; i < carmichael_count; i++){
    res_comp += miller_rabin_uint32(carmichael_nums[i]);
  }
  upper = pow_two(HALF_BIT_COUNT) - 2;
  for (int i = 0; i < trials_comp; i++){
    a = 2 + random_range_uint32(upper);
    b = 2 + random_range_uint32(upper);
    res_comp += miller_rabin_uint32(a * b);
  }
  printf("\tprime correctness:                 ");
  print_test_result(res_prime);
  printf("\tcomposite correctness:             ");
  print_test_result(res_comp == 0);
}

/**
   Tests miller_rabin_uint32 on finding a prime within a range.
*/
void run_prime_scan_test(){
  uint32_t ptwo_start = 10;
  uint32_t trials = 1000;
  uint32_t c;
  uint32_t low, high;
  uint32_t *starts = NULL, *nums = NULL;
  clock_t t;
  printf("Run a miller_rabin_uint32 test on finding %u primes "
	 "in a range \n", trials);
  fflush(stdout);
  starts = calloc_perror(trials, sizeof(uint32_t));
  nums = calloc_perror(trials, sizeof(uint32_t));
  for (uint32_t i = ptwo_start; i < FULL_BIT_COUNT; i++){
    c = 1;
    low = pow_two(i);
    high = (i == FULL_BIT_COUNT - 1) ? UPPER_MAX : pow_two(i + 1);
    printf("\t[%u, %u)\n", low, high);
    for (uint32_t i = 0; i < trials; i++){
      starts[i] = low + random_range_uint32(high - low);
    }
    memcpy(nums, starts, trials * sizeof(uint32_t)); 
    t = clock();
    for (uint32_t j = 0; j < trials; j++){
      while (!miller_rabin_uint32(nums[j])){
	nums[j] = (nums[j] == low) ? high - 1 : nums[j] - 1;
      }
    }
    t = clock() - t;
    memcpy(nums, starts, trials * sizeof(uint32_t));
    for (uint32_t j = 0; j < trials; j++){
      while (!miller_rabin_uint32(nums[j])){
	nums[j] = (nums[j] == low) ? high - 1 : nums[j] - 1;
	c++;
      }
    }
    printf("\t\tave # tests/trial:         %.1f\n "
	   "\t\ttotal runtime:             %.8f seconds\n",
	   (float)c / trials,
	   (float)t / CLOCKS_PER_SEC);
    printf("\n");
  }
  free(starts);
  free(nums);
  starts = NULL;
  nums = NULL;
}

/**
   Printing functions.
*/

void print_bit_probs(const uint32_t *counts, uint32_t trials){
  for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
    if (!(i % BYTE_BIT_COUNT)){
      printf("\n\t\t                           ");
    }
    printf("%.4f ", (float)counts[i] / trials);
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
  UTILITIES_RAND_UINT32_SEED();
  run_random_range_uint32_test();
  run_random_uint32_test();
  run_primality_test();
  run_prime_scan_test();
  return 0;
}
