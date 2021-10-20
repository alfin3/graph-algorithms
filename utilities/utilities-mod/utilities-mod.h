/**
   utilities-mod.h

   Declarations of accessible utility functions in modular arithmetic.
   The utility functions are integer overflow-safe. The provided
   implementations assume that the width of size_t is even.
*/

#ifndef UTILITIES_MOD_H  
#define UTILITIES_MOD_H

#include <stddef.h>

/**
   Computes overflow-safe mod n of the kth power.
*/
size_t pow_mod(size_t a, size_t k, size_t n);

/**
   Computes overflow-safe (a * b) mod n.
*/
size_t mul_mod(size_t a, size_t b, size_t n);

/**
   Computes overflow-safe (a + b) mod n.
*/
size_t sum_mod(size_t a, size_t b, size_t n);

/**
   Multiplies two numbers in an overflow-safe manner and copies the high and
   low bits of the product into the preallocated blocks pointed to by h
   and l.
*/
void mul_ext(size_t a, size_t b, size_t *h, size_t *l);

/**
   Represents n as u * 2**k, where u is odd.
*/
void represent_uint(size_t n, size_t *k, size_t *u);

/**
   Returns the kth power of 2, if 0 <= k < size_t width.
   Exits with an error otherwise.
*/
size_t pow_two_perror(size_t k);

#endif
