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
#include "utilities-mod.h"

#define RGENS_SEED() do{srandom(time(0)); srand48(random());}while (0)
#define RANDOM() (random())
#define DRAND48() (drand48())

static const uint32_t FULL_BIT_COUNT = 8 * sizeof(uint32_t);

void print_test_result(int res);

/**
   Tests random_range_uint32.
*/
void run_random_range_uint32_test(){
  int res = 1;
  uint32_t ptwo_start = 1, ptwo_end = FULL_BIT_COUNT + 1;
  uint32_t precision = 1000;
  uint32_t trials = 10000000;
  uint32_t n, upper, threshold, counts[2];
  clock_t t_rand, t_randr;
  printf("Run random_range_uint32 test, # trials = %u\n", trials);
  for (uint32_t i = ptwo_start; i < ptwo_end; i++){
    upper = (i == FULL_BIT_COUNT) ? 0xffffffff : pow_two(i);
    threshold = pow_two(i - 1);
    counts[0] = 0;
    counts[1] = 0;
    printf("\t[0, %u)\n", upper);
    fflush(stdout);
    t_rand = clock();
    for (uint32_t i = 0; i < trials; i++){
      n = random();
    }
    t_rand = clock() - t_rand;
    t_randr = clock();
    for (uint32_t i = 0; i < trials; i++){
      n = random_range_uint32(upper);
    }
    t_randr = clock() - t_randr;
    for (uint32_t i = 0; i < trials; i++){
      n = random_range_uint32(upper);
      if (n < threshold) counts[0]++;
      if (n >= threshold && n < upper) counts[1]++;
    }
    res = (abs(counts[0] - counts[1]) < trials / precision &&
	   counts[0] + counts[1] == trials);
    printf("\t\trandom:               %.8f seconds\n"
	   "\t\trandom_range_uint32:  %.8f seconds\n",
	   (float)t_rand / CLOCKS_PER_SEC,
	   (float)t_randr / CLOCKS_PER_SEC);
    printf("\t\tcorrectness:          ");
    print_test_result(res);
    res = 1;
  }
  printf("Run random_range_uint32 corner case test\n");
  upper = pow_two(0);
  threshold = 0;
  counts[0] = 0;
  counts[1] = 0;
  printf("\t[0, %u) --> ", upper);
  for (uint32_t i = 0; i < trials; i++){
    n = random_range_uint32(upper);
    if (n < threshold) counts[0]++;
    if (n >= threshold && n < upper) counts[1]++;
  }
  res = (counts[1] == trials);
  print_test_result(res);
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
  //run_random_uint32_test();
  return 0;
}
