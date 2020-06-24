/**
   utilities-ds.c

   Utility functions across the areas of randomness, modular arithmetic, 
   and binary representation.

   Update: 6/24/2020 10:00am
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include "utilities-ds.h"

static uint32_t random_gen_range(uint32_t n);

/** Randomness */

/**
   Returns a generator-uniform random uint64_t.
*/
uint64_t random_uint64(){
  return ((uint64_t)random_uint32() +
	  pow_two_uint64(32) * (uint64_t)random_uint32());
}

/**
   Returns a generator-uniform random uint32_t. 
*/
uint32_t random_uint32(){
  uint32_t upper = (uint32_t)pow_two_uint64(16);
  return (random_gen_range(upper) +
	  (uint32_t)pow_two_uint64(16) * random_gen_range(upper));
}

/**
   Returns a generator-uniform random uint64_t in [0 , n],
   where 0 <= n <= 2^64 - 1. Bernoulli for the lowest set bit in the 
   highest 32 bits is used as the prior for number construction, when n
   exceeds 32 bits.
*/
uint64_t random_range_uint64(uint64_t n){
  uint32_t upper;
  uint64_t upper_max = pow_two_uint64(32) - 1;
  uint64_t ret;
  uint64_t high_bits;
  uint64_t low_bits;
  if (n <= upper_max){
    upper = (uint32_t)n; 
    ret = (uint64_t)random_range_uint32(upper);
  }else{
    //n >= 2^32
    high_bits = n >> 32;
    assert(high_bits);
    low_bits = n - high_bits * pow_two_uint64(32); //[0, 2^32 - 1]
    if (bern_uint64(low_bits + 1, 0, n)){
      //lowest set bit in high_bits is set
      upper = (uint32_t)low_bits;
      ret = (uint64_t)random_range_uint32(upper);
      ret += high_bits * pow_two_uint64(32);
    }else{
      //lowest set bit in high_bits is not set
      upper = (uint32_t)(pow_two_uint64(32) - 1);
      ret = (uint64_t)random_range_uint32(upper);
      upper = (uint32_t)(high_bits - 1);
      ret += ((uint64_t)random_range_uint32(upper) *
	      (uint64_t)pow_two_uint64(32));
    }
  }
  return ret;
}

/**
   Returns a generator-uniform random uint32_t in [0 , n],
   where 0 <= n <= 2^32 - 1. Bernoulli for the most significant (32nd) bit 
   is used as the prior for number construction, when n exceeds 31 bits.
*/
uint32_t random_range_uint32(uint32_t n){
  assert(RAND_MAX == 2147483647);
  uint32_t upper;
  uint32_t rand_max = RAND_MAX; //2^31 - 1
  uint32_t low_bits;
  uint32_t ret;
  if (n <= rand_max){
    upper = n + 1; 
    ret = random_gen_range(upper);
  }else{
    //n >= 2^31
    assert(n >> 31);
    low_bits = n - (uint32_t)pow_two_uint64(31); //[0, 2^31 - 1]
    if (bern_uint32(low_bits + 1, 0, n)){
      //most significant bit set
      upper = low_bits + 1;
      ret = random_gen_range(upper) + (uint32_t)pow_two_uint64(31);
    }else{
      //most significant bit not set
      upper = (uint32_t)pow_two_uint64(31);
      ret = random_gen_range(upper);
    }
  }
  return ret;
}

/**
   Returns a generator-uniform random uint32_t in [0 , n) where 
   0 < n <= RAND_MAX + 1.
*/
static uint32_t random_gen_range(uint32_t n){
  uint32_t rand_max = RAND_MAX;
  uint32_t cut;
  uint32_t rand_num;
  uint32_t ret;
  if (rand_max % n == n - 1){
    ret =  random() % n;
  }else{
    cut = (rand_max % n) + 1;
    rand_num = random();
    while(rand_num > rand_max - cut){
      rand_num = random();
    }
    ret = rand_num % n;
  }
  return ret;
}

/**
   Given a threshold in [low, high], where high > low, returns true with 
   probability (threshold - low)/(high - low). 
*/
bool bern_uint64(uint64_t threshold, uint64_t low, uint64_t high){
  assert(high > low && threshold >= low && threshold <= high);
  assert(sizeof(long double) == 16);
  if(threshold == high){return true;} //p = 1.0
  if(threshold == low){return false;} //p = 0.0
  uint64_t rand_n = random_uint64();
  uint64_t denom = (pow_two_uint64(63) - 1) + pow_two_uint64(63);
  //rand_n in [0, denom - 2], need <= denom + 1 - 2 values for any high - low
  while (rand_n > denom - 2){
    rand_n = random_uint64();
  }
  long double p = (long double)(threshold - low) / (long double)(high - low);
  long double f = (long double)rand_n / (long double)denom;
  return f < p;
}

/**
   Given a threshold in [low, high], where high > low, returns true with 
   probability (threshold - low)/(high - low). 
*/
bool bern_uint32(uint32_t threshold, uint32_t low, uint32_t high){
  assert(high > low && threshold >= low && threshold <= high);
  assert(sizeof(double) == 8);
  if(threshold == high){return true;} //p = 1.0
  if(threshold == low){return false;} //p = 0.0
  uint32_t rand_n = random_uint32();
  uint32_t denom = (uint32_t)(pow_two_uint64(32) - 1);
  //rand_n in [0, denom - 2], need <= denom + 1 - 2 values for any high - low
  while (rand_n > denom - 2){
    rand_n = random_uint32();
  }
  double p = (double)(threshold - low) / (double)(high - low);
  double f = (double)rand_n / (double)denom;
  return f < p;
}

/** Modular arithmetic */

/**
   Computes overflow-safe unsigned mod n of the kth power in O(logk) time 
   and O(1) space overhead, based on the binary representation of k and 
   inductively applying the following relations :
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then 
   a1 a2 ≡ b1 b2 (mod n), and a1 + a2 ≡ b1 + b2 (mod n). 
   The latter relation is applied by calling mul_mod_uint64.
*/
uint64_t pow_mod_uint64(uint64_t a, uint64_t k, uint64_t n){
  assert(n > 0);
  if(n == 1){return 0;}
  if(!k){return 1;};
  //initial mods
  uint64_t ret = 1;
  a = a % n;
  //mods of products
  while (k){
    if (k & 1){
      ret = mul_mod_uint64(ret, a, n); //update for each set bit
    }
    a = mul_mod_uint64(a, a, n); //repetitive squaring between updates
    k >>= 1;
  }
  return ret;
}

/**
   Computes overflow-safe unsigned mod n of the kth power in O(logk) time 
   and O(1) space overhead, based on the binary representation of k and 
   inductively applying the following relation :
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then a1 a2 ≡ b1 b2 (mod n).
*/
uint32_t pow_mod_uint32(uint32_t a, uint64_t k, uint32_t n){
  assert(n > 0);
  if(n == 1){return 0;}
  if(!k){return 1;};
  //"no overflow" guarantee
  uint64_t a64 = a;
  uint64_t n64 = n;
  //initial mods
  uint64_t ret = 1;
  a64 = a64 % n64;
  //mods of products
  while (k){
    if (k & 1){
      ret = (ret * a64) % n64; //update for each set bit
    }
    a64 = (a64 * a64) % n64; //repetitive squaring between updates
    k >>= 1;
  }
  assert(ret < pow_two_uint64(32));
  return (uint32_t)ret;
}

/**
   Computes overflow-safe unsigned (a * b) mod n, by applying the 
   following relation:
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then a1 + a2 ≡ b1 + b2 (mod n).
*/
uint64_t mul_mod_uint64(uint64_t a, uint64_t b, uint64_t n){
  assert(n > 0);
  if(n == 1){return 0;}
  uint64_t ah = a >> 32;
  uint64_t al = a - ah * pow_two_uint64(32);
  uint64_t bh = b >> 32;
  uint64_t bl = b - bh * pow_two_uint64(32);
  //a{h,l}, b{h,l} <= 2^32 - 1
  uint64_t ah_bh = (ah * bh) % n;
  uint64_t ah_bl = (ah * bl) % n;
  uint64_t al_bh = (al * bh) % n;
  uint64_t al_bl = (al * bl) % n;
  uint64_t ret;
  //(ah_bh * 2^32) mod n
  for (int i = 0; i < 32; i++){
    ah_bh = sum_mod_uint64(ah_bh, ah_bh, n);
  }
  ret = sum_mod_uint64(ah_bh, ah_bl, n);
  ret = sum_mod_uint64(ret, al_bh, n);
  //(2^32 * (ah_bh * 2^32 + ah_bl + al_bh)) mod n
  for (int i = 0; i < 32; i++){
    ret = sum_mod_uint64(ret, ret, n);
  }
  ret = sum_mod_uint64(ret, al_bl, n);
  return ret;
}

/**
   Computes overflow-safe unsigned (a + b) mod n, by applying the 
   following relation:
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then a1 + a2 ≡ b1 + b2 (mod n).
*/
uint64_t sum_mod_uint64(uint64_t a, uint64_t b, uint64_t n){
  assert(n > 0);
  if(n == 1){return 0;}
  uint64_t ret;
  uint64_t rem;
  a = a % n;
  b = b % n;
  //a, b < n, can subtract at most one n from a + b
  rem = n - a; //>= 1
  if (rem <= b){
    ret = b - rem;
  }else{
    ret = a + b;
  }
  return ret;
}

/**
   Computes mod n of a memory block in O(block size) time and O(1) 
   space overhead, treating each byte of the block in the little-
   endian order and inductively applying the following relations:
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then 
   a1 a2 ≡ b1 b2 (mod n), and a1 + a2 ≡ b1 + b2 (mod n).
   Does not require a little-endian machine.
*/
uint32_t mem_mod_uint32(void *s, uint64_t size, uint32_t n){
  assert(n > 0);
  if(n == 1){return 0;}
  uint8_t *ptr;
  //"no overflow" guarantee
  uint64_t n64 = n;
  uint64_t byte_val;
  uint64_t pow_two = 1; //1 mod n
  uint64_t prod = 1; //1 mod n
  uint64_t ret = 0; //0 mod n
  uint64_t pow_two_inc = pow_two_uint64(8) % n64;
  for (uint64_t i = 0; i < size; i++){
    ptr = (uint8_t *)s + i;
    byte_val = (uint64_t)(*ptr);
    prod = (pow_two * (byte_val % n64)) % n64;
    ret = (ret + prod) % n64;
    pow_two = (pow_two * pow_two_inc) % n64;
  }
  assert(ret < pow_two_uint64(32));
  return (uint32_t)ret;
}

/** Binary representation */

/**
   Represents n as u * 2^k, where u is odd.
*/
void represent_uint64(uint64_t n, int *k, uint64_t *u){
  int c = 0;
  uint64_t shift_n = n;
  while(shift_n){
    c++;
    shift_n <<= 1;
  }
  *k = sizeof(uint64_t) * 8 - c;
  *u = n >> *k; //*k <= sizeof(uint64_t)
}

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
uint64_t pow_two_uint64(int k){
  assert(0 <= k && k <= 63);
  uint64_t ret = 1;
  return ret << k;
} 
