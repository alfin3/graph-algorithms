/**
   utilities-rand-uint64-main.c

   Tests of randomness utility functions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "utilities-rand-uint64.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

#define RGENS_SEED() do{srandom(time(0));}while (0)
#define RANDOM() (random())

static const uint64_t BYTE_BIT_COUNT = 8;
static const uint64_t FULL_BIT_COUNT = 8 * sizeof(uint64_t);
static const uint64_t UPPER_MAX = 0xffffffffffffffff;

void print_bit_probs(const uint64_t *counts, uint64_t trials);
void print_test_result(int res);

/**
   Tests random_range_uint64.
*/
void run_random_range_uint64_test(){
  uint64_t trials = 10000000;
  uint64_t n_low, n_mid, n_high;
  uint64_t upper_low, upper_mid, upper_high;
  uint64_t *counts_low = NULL, *counts_mid = NULL, *counts_high = NULL;
  uint64_t *masks = NULL;
  clock_t t, t_low, t_mid, t_high;
  counts_low = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  counts_mid = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  counts_high = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  masks = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
    masks[i] = pow_two(i);
  }
  printf("Run random_range_uint64 test, # trials = %lu\n", trials);
  for (uint64_t i = 0; i < FULL_BIT_COUNT + 1; i++){
   if (i == 0){
      upper_high = pow_two(i);
      upper_mid = pow_two(i);
      upper_low = pow_two(i);
      printf("\n\tlow: [0, %lu), mid: [0, %lu), high: [0, %lu)\n",
      	     upper_low, upper_mid, upper_high);
    }else if (i == 1){
      upper_low = pow_two(i - 1) + 1;
      upper_mid = pow_two(i - 1) + 1;
      upper_high = pow_two(i);
      printf("\n\tlow: [0, %lu), mid: [0, %lu), high: [0, %lu)\n",
      	     upper_low, upper_mid, upper_high);
    }else if (i == FULL_BIT_COUNT){
      upper_low = pow_two(i - 1) + 1;
      upper_mid = pow_two(i - 1) + (UPPER_MAX - pow_two(i - 1)) / 2;
      upper_high = UPPER_MAX;
      printf("\n\tlow: [0, %lu), mid: [0, %lu), high: [0, %lu)\n",
      	     upper_low, upper_mid, upper_high);
    }else{
      upper_low = pow_two(i - 1) + 1;
      upper_mid = pow_two(i - 1) + (pow_two(i) - pow_two(i - 1)) / 2;
      upper_high =  pow_two(i);
      printf("\n\tlow: [0, %lu), mid: [0, %lu), high: [0, %lu)\n",
      	     upper_low, upper_mid, upper_high);
    }
    fflush(stdout);
    t = clock();
    for (uint64_t i = 0; i < trials; i++){
      RANDOM();
    }
    t = clock() - t;
    t_low = clock();
    for (uint64_t i = 0; i < trials; i++){
      random_range_uint64(upper_low);
    }
    t_low = clock() - t_low;
    t_mid = clock();
    for (uint64_t i = 0; i < trials; i++){
      random_range_uint64(upper_mid);
    }
    t_mid = clock() - t_mid;
    t_high = clock();
    for (uint64_t i = 0; i < trials; i++){
      random_range_uint64(upper_high);
    }
    t_high = clock() - t_high;
    for (uint64_t i = 0; i < trials; i++){
      n_low = random_range_uint64(upper_low);
      n_mid = random_range_uint64(upper_mid);
      n_high = random_range_uint64(upper_high);
      for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
	if (n_low & masks[i]) counts_low[i]++;
	if (n_mid & masks[i]) counts_mid[i]++;
	if (n_high & masks[i]) counts_high[i]++;
      }
    }
    printf("\t\trandom:                    %.8f seconds\n"
	   "\t\trandom_range_uint64 low:   %.8f seconds\n"
	   "\t\trandom_range_uint64 mid:   %.8f seconds\n"
	   "\t\trandom_range_uint64 high:  %.8f seconds\n",
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
    memset(counts_low, 0, FULL_BIT_COUNT * sizeof(uint64_t));
    memset(counts_mid, 0, FULL_BIT_COUNT * sizeof(uint64_t));
    memset(counts_high, 0, FULL_BIT_COUNT * sizeof(uint64_t));
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
   Tests random_uint64.
*/
void run_random_uint64_test(){
  uint64_t trials_count = 7;
  uint64_t trials[7]= {100,
		       1000,
		       10000,
		       100000,
		       1000000,
		       10000000,
		       100000000};
  uint64_t n;
  uint64_t *counts = NULL;
  uint64_t *masks = NULL;
  clock_t t, t_rand;
  counts = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  masks = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
    masks[i] = pow_two(i);
  }
  printf("Run random_uint64 test\n");
  for (uint64_t ti = 0; ti < trials_count; ti++){
    printf("\t# trials = %lu\n", trials[ti]);
    t = clock();
    for (uint64_t i = 0; i < trials[ti]; i++){
      RANDOM();
    }
    t = clock() - t;
    t_rand = clock();
    for (uint64_t i = 0; i < trials[ti]; i++){
      random_uint64();
    }
    t_rand = clock() - t_rand;
    for (uint64_t i = 0; i < trials[ti]; i++){
      n = random_uint64();
      for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
	if (n & masks[i]) counts[i]++;
      }
    }
    printf("\t\trandom:                    %.8f seconds\n"
	   "\t\trandom_uint64:             %.8f seconds\n",
	   (float)t / CLOCKS_PER_SEC,
	   (float)t_rand / CLOCKS_PER_SEC);
    printf("\t\tP[bit is set]:");
    print_bit_probs(counts, trials[ti]);
    memset(counts, 0, FULL_BIT_COUNT * sizeof(uint64_t));
  }
  free(counts);
  free(masks);
  counts = NULL;
  masks = NULL;
}

/**
   Printing functions.
*/

void print_bit_probs(const uint64_t *counts, uint64_t trials){
  for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
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
  RGENS_SEED();
  run_random_range_uint64_test();
  run_random_uint64_test();
  return 0;
}
