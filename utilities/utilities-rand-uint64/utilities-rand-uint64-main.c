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

void print_test_result(int res);

/**
   Tests random_range_uint64.
*/
void run_random_range_uint64_test(){
  uint64_t trials = 10000000;
  uint64_t n, upper;
  uint64_t *bit_counts = NULL, *bit_masks = NULL;
  clock_t t, t_randr;
  bit_counts = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  bit_masks = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
    bit_masks[i] = pow_two(i);
  }
  printf("Run random_range_uint64 test, # trials = %lu\n", trials);
  for (uint64_t i = 0; i < FULL_BIT_COUNT + 1; i++){
    if (i == FULL_BIT_COUNT){
      upper = UPPER_MAX;
      printf("\t[0, 2^%lu - 1)\n", i);
    }else{
      upper = pow_two(i);
      printf("\t[0, 2^%lu)\n", i);
    }
    fflush(stdout);
    t = clock();
    for (uint64_t i = 0; i < trials; i++){
      n = RANDOM();
    }
    t = clock() - t;
    t_randr = clock();
    for (uint64_t i = 0; i < trials; i++){
      n = random_range_uint64(upper);
    }
    t_randr = clock() - t_randr;
    for (uint64_t i = 0; i < trials; i++){
      n = random_range_uint64(upper);
      for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
	if (n & bit_masks[i]) bit_counts[i]++;
      }
    }
    printf("\t\trandom:               %.8f seconds\n"
	   "\t\trandom_range_uint64:  %.8f seconds\n",
	   (float)t / CLOCKS_PER_SEC,
	   (float)t_randr / CLOCKS_PER_SEC);
    for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
      if (i == 0){
	printf("\t\tP[bit is set]:        ");
      }else if (!(i % BYTE_BIT_COUNT)){
	printf("\n\t\t                      ");
      }	
      printf("%.4f ", (float)bit_counts[i] / trials);
    }
    printf("\n");
    memset(bit_counts, 0, FULL_BIT_COUNT * sizeof(uint64_t));
  }
  free(bit_counts);
  free(bit_masks);
  bit_counts = NULL;
  bit_masks = NULL;
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
  uint64_t *bit_counts = NULL, *bit_masks = NULL;
  clock_t t, t_rand;
  bit_counts = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  bit_masks = calloc_perror(FULL_BIT_COUNT, sizeof(uint64_t));
  for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
    bit_masks[i] = pow_two(i);
  }
  printf("Run random_uint64 test\n");
  for (uint64_t ti = 0; ti < trials_count; ti++){
    printf("\t# trials = %lu\n", trials[ti]);
    t = clock();
    for (uint64_t i = 0; i < trials[ti]; i++){
      n = RANDOM();
    }
    t = clock() - t;
    t_rand = clock();
    for (uint64_t i = 0; i < trials[ti]; i++){
      n = random_uint64();
    }
    t_rand = clock() - t_rand;
    for (uint64_t i = 0; i < trials[ti]; i++){
      n = random_uint64();
      for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
	if (n & bit_masks[i]) bit_counts[i]++;
      }
    }
    printf("\t\trandom:               %.8f seconds\n"
	   "\t\trandom_uint64:        %.8f seconds\n",
	   (float)t / CLOCKS_PER_SEC,
	   (float)t_rand / CLOCKS_PER_SEC);
    for (uint64_t i = 0; i < FULL_BIT_COUNT; i++){
      if (i == 0){
	printf("\t\tP[bit is set]:        ");
      }else if (!(i % BYTE_BIT_COUNT)){
	printf("\n\t\t                      ");
      }	
      printf("%.4f ", (float)bit_counts[i] / trials[ti]);
    }
    printf("\n");
    memset(bit_counts, 0, FULL_BIT_COUNT * sizeof(uint64_t));
  }
  free(bit_counts);
  free(bit_masks);
  bit_counts = NULL;
  bit_masks = NULL;
}

/**
   Prints a test results.
*/
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
