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

#define RGENS_SEED() do{srandom(time(0));}while (0)
#define RANDOM() (random())

static const uint32_t BYTE_BIT_COUNT = 8;
static const uint32_t FULL_BIT_COUNT = 8 * sizeof(uint32_t);

void print_test_result(int res);

/**
   Tests random_range_uint32.
*/
void run_random_range_uint32_test(){
  uint32_t trials = 10000000;
  uint32_t mask = 1;
  uint32_t n, upper;
  uint32_t *bit_counts = NULL, *bit_masks = NULL;
  clock_t t, t_randr;
  bit_counts = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  bit_masks = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
    bit_masks[i] = mask << i;
  }
  printf("Run random_range_uint32 test, # trials = %u\n", trials);
  for (uint32_t i = 0; i < FULL_BIT_COUNT + 1; i++){
    if (i == FULL_BIT_COUNT){
      upper = 0xffffffff;
      printf("\t[0, 2^%u - 1)\n", i);
    }else{
      upper = pow_two(i);
      printf("\t[0, 2^%u)\n", i);
    }
    fflush(stdout);
    t = clock();
    for (uint32_t i = 0; i < trials; i++){
      n = RANDOM();
    }
    t = clock() - t;
    t_randr = clock();
    for (uint32_t i = 0; i < trials; i++){
      n = random_range_uint32(upper);
    }
    t_randr = clock() - t_randr;
    for (uint32_t i = 0; i < trials; i++){
      n = random_range_uint32(upper);
      for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
	if (n & bit_masks[i]) bit_counts[i]++;
      }
    }
    printf("\t\trandom:               %.8f seconds\n"
	   "\t\trandom_range_uint32:  %.8f seconds\n",
	   (float)t / CLOCKS_PER_SEC,
	   (float)t_randr / CLOCKS_PER_SEC);
    for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
      if (i == 0){
	printf("\t\tP[bit is set]:        ");
      }else if (!(i % BYTE_BIT_COUNT)){
	printf("\n\t\t                      ");
      }	
      printf("%.4f ", (float)bit_counts[i] / trials);
    }
    printf("\n");
    memset(bit_counts, 0, FULL_BIT_COUNT * sizeof(uint32_t));
  }
  free(bit_counts);
  free(bit_masks);
  bit_counts = NULL;
  bit_masks = NULL;
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
  uint32_t mask = 1;
  uint32_t n;
  uint32_t *bit_counts = NULL, *bit_masks = NULL;
  clock_t t, t_rand;
  bit_counts = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  bit_masks = calloc_perror(FULL_BIT_COUNT, sizeof(uint32_t));
  for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
    bit_masks[i] = mask << i;
  }
  printf("Run random_uint32 test\n");
  for (uint32_t ti = 0; ti < trials_count; ti++){
    printf("\t# trials = %u\n", trials[ti]);
    t = clock();
    for (uint32_t i = 0; i < trials[ti]; i++){
      n = RANDOM();
    }
    t = clock() - t;
    t_rand = clock();
    for (uint32_t i = 0; i < trials[ti]; i++){
      n = random_uint32();
    }
    t_rand = clock() - t_rand;
    for (uint32_t i = 0; i < trials[ti]; i++){
      n = random_uint32();
      for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
	if (n & bit_masks[i]) bit_counts[i]++;
      }
    }
    printf("\t\trandom:               %.8f seconds\n"
	   "\t\trandom_uint32:        %.8f seconds\n",
	   (float)t / CLOCKS_PER_SEC,
	   (float)t_rand / CLOCKS_PER_SEC);
    for (uint32_t i = 0; i < FULL_BIT_COUNT; i++){
      if (i == 0){
	printf("\t\tP[bit is set]:        ");
      }else if (!(i % BYTE_BIT_COUNT)){
	printf("\n\t\t                      ");
      }	
      printf("%.4f ", (float)bit_counts[i] / trials[ti]);
    }
    printf("\n");
    memset(bit_counts, 0, FULL_BIT_COUNT * sizeof(uint32_t));
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
  run_random_range_uint32_test();
  run_random_uint32_test();
  return 0;
}
