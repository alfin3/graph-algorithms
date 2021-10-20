/**
   utilities-mod.c

   Utility functions in modular arithmetic. The utility functions are
   integer overflow-safe. The provided implementations assume that 
   the width of size_t is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "utilities-mod.h"
#include "utilities-lim.h"

static const size_t C_BYTE_BIT = CHAR_BIT;
static const size_t C_FULL_BIT = UINT_WIDTH_FROM_MAX((size_t)-1);
static const size_t C_HALF_BIT = UINT_WIDTH_FROM_MAX((size_t)-1) / 2u;
static const size_t C_LOW_MASK = ((size_t)-1 >>
				  (UINT_WIDTH_FROM_MAX((size_t)-1) / 2u));

/**
   Computes overflow-safe mod n of the kth power in O(logk) time,
   based on the binary representation of k and inductively applying the
   following relations:
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then 
   a1 a2 ≡ b1 b2 (mod n), and a1 + a2 ≡ b1 + b2 (mod n).
*/
size_t pow_mod(size_t a, size_t k, size_t n){
  size_t k_shift = k;
  size_t ret;
  if (n == 1) return 0;
  ret = 1;
  while (k_shift){
    if (k_shift & 1){
      ret = mul_mod(ret, a, n); /* update for each set bit */
    }
    a = mul_mod(a, a, n); /* repetitive squaring between updates */
    k_shift >>= 1;
  }
  return ret;
}

/**
   Computes overflow-safe (a * b) mod n, by applying the following relation:
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then a1 + a2 ≡ b1 + b2 (mod n).
*/
size_t mul_mod(size_t a, size_t b, size_t n){
  size_t al, bl, ah, bh, ah_bh;
  size_t ret;
  size_t i;
  /* comparisons for speed up */
  if (n == 1 || a == 0 || b == 0) return 0;
  if (a < pow_two_perror(C_HALF_BIT) && b < pow_two_perror(C_HALF_BIT)){
    return (a * b) % n;
  }
  al = a & C_LOW_MASK;
  bl = b & C_LOW_MASK;
  ah = a >> C_HALF_BIT;
  bh = b >> C_HALF_BIT;
  ah_bh = ah * bh;
  for (i = 0; i < C_HALF_BIT; i++){
    ah_bh = sum_mod(ah_bh, ah_bh, n);
  }
  ret = sum_mod(ah_bh, ah * bl, n);
  ret = sum_mod(ret, al * bh, n);
  for (i = 0; i < C_HALF_BIT; i++){
    ret = sum_mod(ret, ret, n);
  }
  ret = sum_mod(ret, al * bl, n);
  return ret;
}

/**
   Computes overflow-safe (a + b) mod n, by applying the following relation:
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then a1 + a2 ≡ b1 + b2 (mod n).
   Note: this version with last unpredictable branch is faster at -O3 than
   the below non-branching version tested with gcc on hash table performance:
   rem = n - a;
   mask = (size_t)-(rem > b);
   return (a & mask) + b - (rem & ~mask);
*/
size_t sum_mod(size_t a, size_t b, size_t n){
  size_t rem;
  if (n == 1) return 0;
  if (a == 0) return b % n;
  if (b == 0) return a % n;
  if (a >= n) a = a % n;
  if (b >= n) b = b % n;
  /* a, b < n, can subtract at most one n from a + b */
  rem = n - a; /* >= 1 */
  if (rem <= b){
    return b - rem;
  }else{
    return a + b;
  }
}

/**
   Computes mod n of a memory block, treating each byte of the block in
   the little-endian order and inductively applying the following relations:
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then 
   a1 a2 ≡ b1 b2 (mod n), and a1 + a2 ≡ b1 + b2 (mod n).
   Does not require a little-endian machine.
*/
size_t mem_mod(const void *s, size_t size, size_t n){
  unsigned char *ptr = (unsigned char *)s; /* defined and not trap rep */
  size_t val, prod;
  size_t ptwo = 1, ptwo_inc;
  size_t ret = 0;
  size_t i;
  if (n == 1) return 0;
  ptwo_inc = mul_mod(pow_two_perror(C_BYTE_BIT - 1), 2, n);
  for (i = 0; i < size; i++){
    val = *ptr;
    /* comparison for speed up across a large memory block */
    if (val >= n) val = val % n;
    prod = mul_mod(ptwo, val, n);
    ret = sum_mod(ret, prod, n);
    ptwo = mul_mod(ptwo, ptwo_inc, n);
    ptr++;
  }
  return ret;
}

/**
   Multiplies two numbers in an overflow-safe manner and copies the high and
   low bits of the product into the preallocated blocks pointed to by h
   and l.
*/
void mul_ext(size_t a, size_t b, size_t *h, size_t *l){
  size_t al, bl, ah, bh, al_bl, al_bh;
  size_t overlap;
  al = a & C_LOW_MASK;
  bl = b & C_LOW_MASK;
  ah = a >> C_HALF_BIT;
  bh = b >> C_HALF_BIT;
  al_bl = al * bl;
  al_bh = al * bh;
  overlap = ((bl * ah & C_LOW_MASK) +
	     (al_bh & C_LOW_MASK) +
	     (al_bl >> C_HALF_BIT));
  *h = ((overlap >> C_HALF_BIT) +
	ah * bh +
	(ah * bl >> C_HALF_BIT) +
	(al_bh >> C_HALF_BIT));
  *l = (overlap << C_HALF_BIT) + (al_bl & C_LOW_MASK);
}

/**
   Represents n as u * 2**k, where u is odd.
*/
void represent_uint(size_t n, size_t *k, size_t *u){
  size_t c = 0;
  size_t shift_n = n;
  while (shift_n){
    c++;
    shift_n <<= 1;
  }
  *k = C_FULL_BIT - c;
  *u = n >> *k;
}

/**
   Returns the kth power of 2, if 0 <= k < size_t width.
   Exits with an error otherwise.
*/
size_t pow_two_perror(size_t k){
  if (k >= C_FULL_BIT){
    perror("pow_two size_t overflow");
    exit(EXIT_FAILURE);
  }
  return (size_t)1 << k;
} 
