/**
   utilities-ds.h

   Declarations of accessible utility functions across the areas of 
   randomness, modular arithmetic, and binary representation.

   Update: 6/14/2020
*/

#ifndef UTILITIES_DS_H  
#define UTILITIES_DS_H

#include <stdint.h>

/**
   Returns a generator-uniform random uint32_t in [0 , n).
*/
uint32_t random_range_uint32(uint32_t n);

/**
   Computes mod n of the kth power in O(logk) time.
*/
uint32_t pow_mod_uint32(uint32_t a, uint32_t k, uint32_t n);

/**
   Represents n as u * 2^k, where u is odd.
*/
void represent_uint64(uint64_t n, int *k, uint64_t *u);

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
uint64_t pow_two_uint64(int k);

#endif
