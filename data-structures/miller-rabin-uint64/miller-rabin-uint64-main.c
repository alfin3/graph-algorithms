/**
   miller-rabin-uint64-main.c

   Examples of randomized primality testing for hashing applications.

   See man random for additional information on random number generation. 
   Please note that "the GNU C Library does not provide a cryptographic 
   random number generator"
   https://www.gnu.org/software/libc/manual/html_node/Unpredictable-Bytes.html
   
   The implementation provides a "no overflow" guarantee given a number of
   type uint64_t, and preserves the generator-provided uniformity in random 
   processes. The generator is not seeded by miller_rabin_uint64.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include "miller-rabin-uint64.h"
#include "utilities-rand-mod.h"

/**
   Test miller_rabin_uint64 on pre-defined subsets of numbers.
*/
void print_test_result(int result);

void run_true_test(uint64_t arr[], int arr_size){
  int result = 1;
  for (int i = 0; i < arr_size; i++){
    result *= miller_rabin_uint64(arr[i]);
  }
  print_test_result(result);
}

void run_false_test(uint64_t arr[], int arr_size){
  int result = 0;
  for (int i = 0; i < arr_size; i++){
    result += miller_rabin_uint64(arr[i]);
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
   Tests miller_rabin_uint64 on large primes.
*/
void run_large_prime_test(){
  int result = 1;
  int c = 55;
  uint64_t n;
  int primes_pow_56_63_minus[80] = {5, 27, 47, 57, 89,
				    93, 147, 177, 189, 195,
				    13, 25, 49, 61, 69,
				    111, 195, 273, 363, 423,
				    27, 57, 63, 137, 141,
				    147, 161, 203, 213, 251,
				    55, 99, 225, 427, 517,
				    607, 649, 687, 861, 871,
				    93, 107, 173, 179, 257,
				    279, 369, 395, 399, 453,
				    1, 31, 45, 229, 259,
				    283, 339, 391, 403, 465,
				    57, 87, 117, 143, 153,
				    167, 171, 195, 203, 273,
				    25, 165, 259, 301, 375,
				    387, 391, 409, 457, 471};
  printf("Run a miller_rabin_uint64 test on large primes --> ");
  fflush(stdout);
  for (int i = 0; i < 80; i++){
    if(i % 10 == 0){c++;}
    n = pow_two_uint64(c) - primes_pow_56_63_minus[i];
    result *= miller_rabin_uint64(n); 
  }
  print_test_result(result);
}

/**
   Tests miller_rabin_uint64 on random composites.
*/
void run_random_composite_test(){
  int num_trials = 100000;
  int result = 0;
  uint64_t upper = pow_two_uint64(32) - 3;
  uint64_t rand_a;
  uint64_t rand_b;
  printf("Run a miller_rabin_uint64 test on %d random composites \n",
	 num_trials);
  printf("\tn = a * b, where 2 <= a <= 2^32 - 1, 2 <= b <= 2^32 - 1 --> ");
  fflush(stdout);
  srandom(time(0));
  for (int i = 0; i < num_trials; i++){
    rand_a = 2 + random_range_uint64(upper);
    rand_b = 2 + random_range_uint64(upper);
    result += miller_rabin_uint64(rand_a * rand_b);
  }
  print_test_result(result == 0);
}

/**
   Tests miller_rabin_uint64 on finding a prime within a range.
*/
static void find_prime_test_helper(uint64_t low,
			           uint64_t high,
			           int num_trials);

void run_find_prime_test(){
  int num_trials = 100;
  int pow_two_start = 10;
  int pow_two_end = 63;
  uint64_t low, high;
  printf("Run a miller_rabin_uint64 test on finding a prime within a range, "
	 "in %d trials per range \n", num_trials);
  fflush(stdout);
  srandom(time(0));
  for (int i = pow_two_start; i < pow_two_end; i++){
    low = pow_two_uint64(i);
    high = pow_two_uint64(i + 1);
    find_prime_test_helper(low, high, num_trials);
  }
}

static void find_prime_test_helper(uint64_t low,
			           uint64_t high,
			           int num_trials){
  uint64_t n;
  int c = 1;
  clock_t t;
  t = clock();
  for (int i = 0; i < num_trials; i++){
    n = low + random_range_uint64(high - low);
    while (!miller_rabin_uint64(n)){
      n--;
      c++;
      if (n < low){
	n = low + random_range_uint64(high - low);
      }	
    }
  }
  t = clock() - t;
  printf("\t[%lu, %lu], # tests/trial:  %.1f, "
	 "runtime/trial : %.5f seconds \n",
	 low, high, (float)c / (float)num_trials,
	 (float)t / ((float)num_trials * CLOCKS_PER_SEC));
}

int main(){;
  uint64_t primes[30] = {2, 3, 5, 7, 11, 
                         13, 17, 19, 23, 29, 
                         31, 37, 41, 43, 47,
                         103991, 103993, 103997, 104003, 104009, 
                         104021, 104033, 104047, 104053, 104059, 
                         899809363, 920419813, 920419823, 941083981, 941083987};
  uint64_t non_primes[30] = {0, 1, 4, 6, 8, 
                             9, 10, 12, 14, 
                             15, 16, 18, 20, 
                             951, 952, 954, 955, 956, 
                             957, 958, 959, 960, 961, 
                             962, 963, 964, 965, 966};
  uint64_t carmichael_nums[30] = {561, 1105, 1729, 2465, 2821,
			     6601, 8911, 10585, 15841, 29341,
			     41041, 46657, 52633, 62745, 63973,
			     75361, 101101, 115921, 126217, 162401,
			     172081, 188461, 252601, 278545, 294409,
			     314821, 334153, 340561, 399001, 410041};
  srandom(time(0));
  printf("Run miller_rabin_uint64 test on small primes --> ");
  fflush(stdout);
  run_true_test(primes, 30);
  run_large_prime_test();
  printf("Run miller_rabin_uint64 test on non-primes --> ");
  fflush(stdout);
  run_false_test(non_primes, 30);
  printf("Run miller_rabin_uint64 test on Carmichael numbers --> ");
  fflush(stdout);
  run_false_test(carmichael_nums, 30);
  run_random_composite_test();
  run_find_prime_test();
  return 0;
}

