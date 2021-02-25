/**
   utilities-rand-uint64.c

   Randomness utility functions.

   The generation of random numbers in a given range is achieved in a
   randomized approach by exponentially decreasing the probability of not
   finding a number bounded by 0.5^N under the assumption of random number
   generator uniformity, where N is the number of generated number
   candidates. N is less or equal to 2 in expectation.

   The implementation is based on random() that returns a random number from
   0 to RAND_MAX, where RAND_MAX is 2^31 - 1, with a large period of approx.
   16 * (2^31 - 1). The provided functions are seeded by seeding random() 
   outside the provided functions. The implementation is not suitable for
   cryptographic use. (https://man7.org/linux/man-pages/man3/random.3.html)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utilities-rand-uint64.h"
#include "utilities-mod.h"

static const uint64_t FULL_BIT_COUNT = 8 * sizeof(uint64_t);
static const uint64_t HALF_BIT_COUNT = 4 * sizeof(uint64_t);
static const uint64_t RAND_MAX_UINT64_TEST = 2147483647;
static const uint64_t RAND_MAX_UINT64 = RAND_MAX; //associate with a type

static uint64_t random_mod_pow_two(uint64_t k);
static uint32_t random_gen_range(uint32_t n); //faster with uint32_t
static void fprintf_stderr_exit(const char *s, int line);

/**
   Returns a generator-uniform random uint64_t in [0 , n), where n > 0.
   In the while loop, the probability of not finding a number decreases
   exponentially and is less or equal to 0.5^n under the assumption of
   generator uniformity, where n is the number of random() calls if 
   n <= RAND_MAX + 1, and the number of random_uint64() calls otherwise.
*/
uint64_t random_range_uint64(uint64_t n){
  uint64_t n_shift = n;
  uint64_t k, ret;
  if (RAND_MAX_UINT64 != RAND_MAX_UINT64_TEST){
    fprintf_stderr_exit("RAND_MAX value error", __LINE__);
  }
  if (n <= RAND_MAX_UINT64 + 1){
    ret = random_gen_range(n);
  }else{
    k = HALF_BIT_COUNT;
    n_shift >>= HALF_BIT_COUNT; //~2X speedup
    while (n_shift >>= 1){ //~2X speedup by assigning in the condition
      k++;
    }
    if (n > pow_two(FULL_BIT_COUNT - 1)) k++;
    ret = random_mod_pow_two(k);
    while (ret > n - 1){
      ret = random_mod_pow_two(k);
    }
  }
  return ret;
}

/**
   Returns a generator-uniform random uint64_t. 
*/
uint64_t random_uint64(){
  if (RAND_MAX_UINT64 != RAND_MAX_UINT64_TEST){
    fprintf_stderr_exit("RAND_MAX value error", __LINE__);
  }
  return random_mod_pow_two(FULL_BIT_COUNT);
}

/**
   Returns a generator-uniform random uint64_t mod 2^k. 
*/
static uint64_t random_mod_pow_two(uint64_t k){
  uint64_t n, ret;
  ret = random();
  if (k < HALF_BIT_COUNT){
    ret = ret >> (HALF_BIT_COUNT - k - 1);
  }else if (k < FULL_BIT_COUNT - 1){
    n = random();
    n >>= (FULL_BIT_COUNT - k - 2);
    n <<= (HALF_BIT_COUNT - 1);
    ret |= n;
  }else{
    n = random();
    n <<= (HALF_BIT_COUNT - 1);
    ret |= n;
    n = random();
    n >>= (FULL_BIT_COUNT + HALF_BIT_COUNT - k - 3);
    n <<= (FULL_BIT_COUNT - 2);
    ret |= n;
  }
  return ret;
}

/**
   Returns a generator-uniform random uint32_t in [0 , n) where 
   0 < n <= RAND_MAX + 1. In the while loop, the probability of not finding
   a number decreases exponentially and is less or equal to 0.5^n under the
   assumption of generator uniformity, where n is the number of random()
   calls.
*/
static uint32_t random_gen_range(uint32_t n){
  uint32_t rem, ret;
  rem = (uint32_t)RAND_MAX_UINT64 % n;
  ret =  random();
  if (rem < n - 1){
    while (ret > (uint32_t)RAND_MAX_UINT64 - rem - 1){
      ret = random();
    }
  }
  if (ret > n - 1) ret = ret % n;
  return ret;
}

/**
   Prints an error message and exits.
*/
static void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
}
