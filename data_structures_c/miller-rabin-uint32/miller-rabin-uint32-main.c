/**
   miller-rabin-uint32-main.c

   Examples of randomized primality testing for hashing applications.

   See man random for additional information on random number generation. 
   Please note that "the GNU C Library does not provide a cryptographic 
   random number generator"
   https://www.gnu.org/software/libc/manual/html_node/Unpredictable-Bytes.html
   
   The implementation provides a "no overflow" guarantee given a number of
   type uint32_t, and preserves the generator-provided uniformity in random 
   processes. The generator is not seeded by miller_rabin_uint32.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include "miller-rabin-uint32.h"
#include "utilities-ds.h"

/**
   Test miller_rabin_uint32 on pre-defined subsets of numbers.
*/
void print_test_result(int result);

void run_true_test(uint32_t arr[], int arr_size){
  int result = 1;
  for (int i = 0; i < arr_size; i++){
    result *= miller_rabin_uint32(arr[i]);
  }
  print_test_result(result);
}

void run_false_test(uint32_t arr[], int arr_size){
  int result = 0;
  for (int i = 0; i < arr_size; i++){
    result += miller_rabin_uint32(arr[i]);
  }
  print_test_result(!result);
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

/**
   Tests miller_rabin_uint32 on random composites.
*/
void run_random_composite_test(){
  int num_trials = 10000000;
  int result = 0;
  uint32_t upper_a = (uint32_t)(pow_two_uint64(16) - 3);
  uint32_t upper_b = (uint32_t)(pow_two_uint64(16) - 3);
  uint32_t rand_a;
  uint32_t rand_b;
  printf("Run miller_rabin_uint32 test on random composites \n");
  printf("\tn = a * b, where 2 <= a <= 2^16 - 1, 2 <= b <= 2^16 - 1 --> ");
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = 2 + random_range_uint32(upper_a);
    rand_b = 2 + random_range_uint32(upper_b);
    result += miller_rabin_uint32(rand_a * rand_b);
  }
  print_test_result(result == 0);
}

int main(){;
  uint32_t primes[30] = {2, 3, 5, 7, 11, 
                         13, 17, 19, 23, 29, 
                         31, 37, 41, 43, 47,
                         103991, 103993, 103997, 104003, 104009, 
                         104021, 104033, 104047, 104053, 104059, 
                         899809363, 920419813, 920419823, 941083981, 941083987};
  uint32_t non_primes[30] = {0, 1, 4, 6, 8, 
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
  srandom(time(0));
  printf("Run miller_rabin_uint32 test on primes --> ");
  run_true_test(primes, 30);
  printf("Run miller_rabin_uint32 test on non-primes --> ");
  run_false_test(non_primes, 30);
  printf("Run miller_rabin_uint32 test on Carmichael numbers --> ");
  run_false_test(carmichael_nums, 30);
  srandom(time(0));
  run_random_composite_test();
  return 0;
}

