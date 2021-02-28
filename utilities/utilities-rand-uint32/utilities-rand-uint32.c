/**
   utilities-rand-uint32.c

   Randomness utility functions.

   The generation of (pseudo-)random numbers in a given range is achieved
   in a randomized approach by exponentially decreasing the probability of
   not finding a number bounded by 0.5^N under the assumption of generator
   uniformity, where N is the number of generated number candidates. N is
   less or equal to 2 in expectation.

   Primality testing is performed in a randomized approach according to
   Miller and Rabin.

   The implementation is based on a generator that returns a number
   from 0 to RAND_MAX, where RAND_MAX is 2^31 - 1, as set by 
   UTILITIES_RAND_UINT32_RANDOM() and seeded by UTILITIES_RAND_UINT32_SEED().
   Other generators may be accomodated in the future. The implementation is
   not suitable for cryptographic use. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utilities-rand-uint32.h"
#include "utilities-mod.h"

static const uint32_t FULL_BIT_COUNT = 8 * sizeof(uint32_t);
static const uint32_t HIGH_MASK = 2147483648U; //2^31
static const uint32_t RAND_MAX_UINT32_TEST = 2147483647U;
static const uint32_t RAND_MAX_UINT32 = RAND_MAX; //associate with a type
static const int COMPOSITE_TRIALS = 50;

//number generation
static uint32_t random_mod_pow_two(uint32_t k);
static uint32_t random_gen_range(uint32_t n);

//primality testing
static int composite(uint32_t n, int trials);
static int witness(uint32_t a, uint32_t n);
static void represent_uint32(uint32_t n, uint32_t *k, uint32_t *u);

//auxiliary functions
static void fprintf_stderr_exit(const char *s, int line);

/* Number generation */

/**
   Returns a generator-uniform uint32_t in [0 , n), where n > 0.
   In the while loop, the probability of not finding a number decreases
   exponentially and is less or equal to 0.5^n under the assumption of
   generator uniformity, where n is the number of generator calls if
   n <= RAND_MAX + 1, and the number of random_uint32() calls otherwise.
*/
uint32_t random_range_uint32(uint32_t n){
  uint32_t ret;
  if (RAND_MAX_UINT32 != RAND_MAX_UINT32_TEST){
    fprintf_stderr_exit("RAND_MAX value error", __LINE__);
  }
  if (n <= RAND_MAX_UINT32 + 1){
    ret = random_gen_range(n);
  }else{
    ret = random_uint32();
    while (ret > n - 1){
      ret = random_uint32();
    }
  }
  return ret;
}

/**
   Returns a generator-uniform uint32_t. 
*/
uint32_t random_uint32(){
  if (RAND_MAX_UINT32 != RAND_MAX_UINT32_TEST){
    fprintf_stderr_exit("RAND_MAX value error", __LINE__);
  }
  return random_mod_pow_two(FULL_BIT_COUNT);
}

/**
   Returns a generator-uniform uint32_t mod 2^k. 
*/
static uint32_t random_mod_pow_two(uint32_t k){
  uint32_t n, ret;
  ret = UTILITIES_RAND_UINT32_RANDOM();
  if (k < FULL_BIT_COUNT){
    ret >>= FULL_BIT_COUNT - k - 1;
  }else{
    n = UTILITIES_RAND_UINT32_RANDOM();
    n <<= 1;
    n &= HIGH_MASK;
    ret |= n;
  }
  return ret;
}

/**
   Returns a generator-uniform uint32_t in [0 , n) where
   0 < n <= RAND_MAX + 1. In the while loop, the probability of not finding
   a number decreases exponentially and is less or equal to 0.5^n under the
   assumption of generator uniformity, where n is the number of generator
   calls.
*/
static uint32_t random_gen_range(uint32_t n){
  uint32_t rem, ret;
  rem = RAND_MAX_UINT32 % n;
  ret =  UTILITIES_RAND_UINT32_RANDOM();
  if (rem < n - 1){
    while (ret > RAND_MAX_UINT32 - rem - 1){
      ret = UTILITIES_RAND_UINT32_RANDOM();
    }
  }
  if (ret > n - 1) ret = ret % n;
  return ret;
}

/* Primality testing */

/**
   Runs a randomized primality test. Returns 1 if n is prime and 0
   otherwise.
*/
int miller_rabin_uint32(uint32_t n){
  int ret;
  if (n == 2){
    ret = 1;
  }else if (n == 1 || !(n & 1) || composite(n, COMPOSITE_TRIALS)){
    ret = 0;
  }else{
    ret = 1;
  }
  return ret;
}

/**
   Runs a randomized composite test on n across random_range_uint32-generated
   bases in the number of trials that equals to the value of the trials
   parameter. Returns 1 if a witness is detected and 0 otherwise.
*/
static int composite(uint32_t n, int trials){
  uint32_t a;
  uint32_t upper = n - 2;
  for (int i = 0; i < trials; i++){
    a = 2 + random_range_uint32(upper); //[2, n - 1] 
    if (witness(a, n)) return 1;
  }
  return 0;
}

/**
   Determines if n is composite and a is its witness, otherwise n is likely 
   a prime. n must be odd and greater or equal to 3.
*/
static int witness(uint32_t a, uint32_t n){
  uint32_t t, u, x[2];
  represent_uint32(n - 1, &t, &u);
  x[0] = pow_mod(a, u, n);
  x[1] = pow_mod(x[0], 2, n); //t > 0
  for (uint32_t i = 0; i < t; i++){
    if (x[1] == 1 && !(x[0] == 1 || x[0] == n - 1)){
      return 1; //nontrivial root => composite
    }
    if (i < t - 1){
      x[0] = x[1];
      x[1] = pow_mod(x[0], 2, n);
    }
  }
  if (x[1] != 1) return 1; //composite based on Fermat's little theorem
  return 0;
}

/**
   Represents n as u * 2^k, where u is odd.
*/
static void represent_uint32(uint32_t n, uint32_t *k, uint32_t *u){
  uint32_t c = 0;
  uint32_t shift_n = n;
  while (shift_n){
    c++;
    shift_n <<= 1;
  }
  *k = FULL_BIT_COUNT - c;
  *u = n >> *k;
}

/**
   Prints an error message and exits.
*/
static void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
}
