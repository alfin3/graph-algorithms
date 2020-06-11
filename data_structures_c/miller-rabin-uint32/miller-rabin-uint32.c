/**
   miller-rabin-uint32.c

   Struct declarations and declarations of accessible functions for 
   randomized primality testing for hashing applications.

   See man random for additional information on random number generation. 
   Please note that "the GNU C Library does not provide a cryptographic 
   random number generator"
   https://www.gnu.org/software/libc/manual/html_node/Unpredictable-Bytes.html
   
   The implementation provides a "no overflow" guarantee given a number of
   type uint32_t, and preserves the generator-provided uniformity in random 
   processes. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

static bool composite(uint64_t n, int num_iter);
static bool witness(uint64_t a, uint64_t n);
static void represent(uint64_t n, int *t, uint64_t *u);
static uint64_t rep_sq_pow_mod(uint64_t a, uint64_t m, uint64_t n);
static uint64_t random_range(uint64_t n);
static uint64_t pow_of_two(int i);

/**
   Runs a randomized primality test.
*/
bool miller_rabin_uint32(uint32_t n){
  int num_iter = 100;
  uint64_t num = n; //<= 2^32 - 1 within 64 bits
  if (num == 2){
    return true;
  }else if (num == 1 || !(num % 2)){
    return false;
  }else if (composite(num, num_iter)){
    return false;
  }else{
    return true;
  }
}

/**
   Runs a randomized composite test on n with num_iter random bases. 
   Returns true, if a witness is detected. If after num_iter rounds no 
   witness is detected, returns false.
*/
static bool composite(uint64_t n, int num_iter){
  assert(RAND_MAX == 2147483647);
  assert(n > 2);
  uint64_t rand_max_64 = RAND_MAX;
  uint64_t a;
  uint64_t upper;
  srandom (time (0)); //sec resolution
  for (int i = 0; i < num_iter; i++){
    if (n - 1 <= 2 + rand_max_64){
      upper = n - 2; // n - 3 + 1
      a = 2 + random_range(upper);
    }else{
      //need two calls to the generator
      upper = rand_max_64 + 1;
      a = 2 + random_range(upper);
      upper = n - 2 - rand_max_64;
      a += random_range(upper);
    }
    if (witness(a, n)){
      return true;
    }
  }
  return false;
}

/**
   Determines if n is composite and a is its witness, otherwise n is likely 
   a prime.
*/
static bool witness(uint64_t a, uint64_t n){
  int t;
  uint64_t u;
  uint64_t x[2];
  represent(n - 1, &t, &u);
  x[0] = rep_sq_pow_mod(a, u, n);
  for (int i = 1; i < t + 1; i++){
    x[1] = rep_sq_pow_mod(x[0], 2, n);
    //nontrivial root => not prime => composite
    if (x[1] == 1 && !(x[0] == 1 || x[0] == n - 1)){
      return true;
    }
    x[0] = x[1];
  }
  //composite test based on Fermat's little theorem
  if (x[1] != 1){return true;}
  return false;
}

/**
   Represents n-1 as u*2^t for repetitive squaring in nontrivial root tests.
*/
static void represent(uint64_t n, int *t, uint64_t *u){
  int c = 0;
  uint64_t shift_n = n;
  while(shift_n){
    c++;
    shift_n <<= 1;
  }
  *t = sizeof(uint64_t) * 8 - c;
  *u = n >> *t; //*t <= sizeof(uint64_t)
}

/**
   Computes mod n of the mth power in O(logm) time, based on the 
   binary representation of m and the following relations :
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then 
   a1 a2 ≡ b1 b2 (mod n), and a1 + a2 ≡ b1 + b2 (mod n).
*/
static uint64_t rep_sq_pow_mod(uint64_t a, uint64_t m, uint64_t n){
  //"no overflow" guarantee
  assert(a <= pow_of_two(32) - 2);
  assert(n <= pow_of_two(32) - 1);
  //initial mods
  uint64_t r = 1; //1 mod n
  a = a % n;
  //mods of products
  while (m){
    if (m & 1){
      r = (r * a) % n; //update for each set bit
    }
    a = (a * a) % n; //repetitive squaring between updates
    m >>= 1;
  }
  return r;
}

/**
   Returns a uniformly random uint64_t in [0 , n),where 0 < n <= RAND_MAX + 1.
*/
static uint64_t random_range(uint64_t n){
  assert(RAND_MAX == 2147483647);
  assert(0 < n);
  uint64_t rand_max_64 = RAND_MAX;
  uint64_t cut;
  uint64_t rand_num;
  uint64_t r;
  if (rand_max_64 % n == n - 1){
    r =  random() % n;
  }else{
    cut = (rand_max_64 % n) + 1;
    rand_num = random();
    while(rand_num > rand_max_64 - cut){
      rand_num = random();
    }
    r = rand_num % n;
  }
  return r;
}

/**
   Returns the ith power of 2, where 0 <= i <= 63.
*/
static uint64_t pow_of_two(int i){
  assert(0 <= i && i <= 63);
  uint64_t r = 1;
  return r << i;
} 
