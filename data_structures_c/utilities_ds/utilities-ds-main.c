/**
   utilities-ds-main.c

   Examples of utility functions across the areas of randomness,
   modular arithmetic, and binary representation.

   Update: 6/13/2020
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
   Tests random_range_uint32.
*/
void run_random_range_uint32_test(){
  int num_trials = 10000000;
  int result = 1;
  uint32_t rand_num;
  uint32_t upper = 1;
  printf("Run random_range_uint32 test: n = %u --> ", upper);
  srandom (time (0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_range_uint32(upper);
    result *= (rand_num == 0);
  }
  print_test_result(result);
  int id_count[2] = {0, 0};
  upper = 2;
  printf("Run random_range_uint32 test: n = %u --> ", upper);
  srandom (time (0));
  for (int i = 0; i < num_trials; i++){
    rand_num = random_range_uint32(upper);
    if (rand_num == 0){id_count[0]++;};
    if (rand_num == 1){id_count[1]++;};   
  }
  result = (id_count[0] > 0 &&
	    id_count[1] > 0 &&
	    id_count[0] + id_count[1] == num_trials);
  print_test_result(result);
}

/** Modular arithmetic */

/**
   Tests pow_mod_uint32.
*/
void run_pow_mod_uint32_test(){
  int num_trials = 1000000;
  int result = 1;
  uint32_t upper_a = 16;
  uint32_t upper_k = 17;
  uint32_t upper_n = (uint32_t)(pow_two_uint64(32) - 1);
  uint32_t rand_a;
  uint32_t rand_k;
  uint32_t rand_n;
  uint64_t r;
  uint64_t r_wo_pow;
  printf("Run pow_mod_uint32 random test: --> ");
  srandom (time (0));
  for (int i = 0; i < num_trials; i++){
    rand_a = random_range_uint32(upper_a);
    rand_k = random_range_uint32(upper_k);
    rand_n = random_range_uint32(upper_n);
    r = pow_mod_uint32(rand_a, rand_k, rand_n);
    r_wo_pow = 1;
    for (uint32_t i = 0; i < rand_k; i++){
      r_wo_pow *= (uint64_t)rand_a;
    }
    r_wo_pow = r_wo_pow % (uint64_t)rand_n;
    result *= (r ==  r_wo_pow);
  }
  print_test_result(result);
  printf("Run pow_mod_uint32 corner cases test: --> ");
  result = 1;
  result *= (pow_mod_uint32(0, 0, 1) == 0);
  result *= (pow_mod_uint32(2, 0, 1) == 0);
  result *= (pow_mod_uint32(0, 0, 2) == 1);
  result *= (pow_mod_uint32(2, 0, 2) == 1);
  result *= (pow_mod_uint32(4294967295, 4294967295, 4294967295) == 0);
  result *= (pow_mod_uint32(4294967294, 4294967295, 4294967295) == 4294967294);
  print_test_result(result);
}

/** Binary representation */

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
  printf("Run represent_uint64 primes test: --> ");
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
  printf("Run represent_uint32 odds test: --> ");
  for (int i = 0; i < arr_len; i++){
    represent_uint64(odds[i], &k, &u);
    result *= (k == 0 && u == odds[i]);
  }
  print_test_result(result);
  result = 1;
  printf("Run represent_uint32 odds product test: --> ");
  for (int i = 0; i < arr_len; i++){
    for (int j = 0; j < upper_k; j++){
      num = pow_two_uint64(j) * odds[i];
      represent_uint64(num, &k, &u);
      result *= (k == j && u == odds[i]);
    }
  }
  print_test_result(result);
  result = 1;
  printf("Run represent_uint32 corner cases test: --> ");
  represent_uint64(0, &k, &u);
  result *= (k == 64, u == 0);
  represent_uint64(1, &k, &u);
  result *= (k == 0, u == 1);
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
  printf("Run pow_two_uint64 test: --> ");
  print_test_result(result);
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){;
  run_random_range_uint32_test();
  run_pow_mod_uint32_test();
  run_represent_uint64_test();
  run_pow_two_uint64_test();
  return 0;
}

