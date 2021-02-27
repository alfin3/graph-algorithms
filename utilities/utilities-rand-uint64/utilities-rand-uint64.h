/**
   utilities-rand-uint64.h

   Declarations of accessible randomness utility functions.

   The generation of (pseudo-)random numbers in a given range is achieved
   in a randomized approach by exponentially decreasing the probability of
   not finding a number bounded by 0.5^N under the assumption of generator
   uniformity, where N is the number of generated number candidates. N is
   less or equal to 2 in expectation.

   Primality testing is performed in a randomized approach according to
   Miller and Rabin.

   The implementation is based on a generator that returns a number
   from 0 to RAND_MAX, where RAND_MAX is 2^31 - 1, as set by 
   UTILITIES_RAND_UINT64_RANDOM() and seeded by UTILITIES_RAND_UINT64_SEED().
   The implementation is not suitable for cryptographic use. 
*/

#ifndef UTILITIES_RAND_UINT64_H  
#define UTILITIES_RAND_UINT64_H

#include <stdint.h>
#include <time.h>

/**
   Returns a generator-uniform uint64_t in [0 , n).
*/
uint64_t random_range_uint64(uint64_t n);

/**
   Returns a generator-uniform uint64_t. 
*/
uint64_t random_uint64();

/**
   Runs a randomized primality test. Returns 1 if n is prime and 0
   otherwise.
*/
int miller_rabin_uint64(uint64_t n);

/**
   Macro for setting the random number generator that returns a number
   from 0 to RAND_MAX, where RAND_MAX is 2^31 - 1. By default, the
   generator is set to random() that returns a pseudo-random number
   with a large period of approximately 16 * (2^31 - 1). 
   (https://man7.org/linux/man-pages/man3/random.3.html)
*/
#define UTILITIES_RAND_UINT64_SEED() do{srandom(time(0));}while (0)
#define UTILITIES_RAND_UINT64_RANDOM() (random())

#endif
