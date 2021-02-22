/**
   utilities-rand-uint32.c

   Utility functions in randomness.

   The implementation is based on the random() function that returns a
   random number from 0 to RAND_MAX, where RAND_MAX is 2^31 - 1, with a
   large period of approx. 16 * (2^31 - 1). The provided functions are
   seeded by seeding random() outside the provided functions. The
   implementation is not suitable for cryptographic use.
   (https://man7.org/linux/man-pages/man3/random.3.html)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utilities-rand-uint32.h"
#include "utilities-mod.h"

static const size_t HALF_BIT_COUNT = 4 * sizeof(uint32_t);
static const uint32_t RAND_MAX_UINT32_TEST = 2147483647;
static const uint32_t RAND_MAX_UINT32 = RAND_MAX; //associate with type

static uint32_t random_gen_range(uint32_t n);
static void fprintf_stderr_exit(const char *s, int line);

/**
   Returns a generator-uniform random uint32_t. 
*/
uint32_t random_uint32(){
  uint32_t n = pow_two(HALF_BIT_COUNT);
  return random_gen_range(n) + (random_gen_range(n) << HALF_BIT_COUNT);
}

/**
   Returns a generator-uniform random uint32_t in [0 , n) where 
   0 < n <= RAND_MAX + 1.
*/
static uint32_t random_gen_range(uint32_t n){
  uint32_t rand_val, rem, ret;
  if (RAND_MAX_UINT32 != RAND_MAX_UINT32_TEST){
    fprintf_stderr_exit("RAND_MAX value error", __LINE__);
  }
  rem = RAND_MAX_UINT32 % n;
  if (rem == n - 1){
    ret =  random();
  }else{
    ret = random();
    while(ret > RAND_MAX_UINT32 - rem - 1){
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
