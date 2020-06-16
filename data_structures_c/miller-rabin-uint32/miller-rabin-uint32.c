/**
   miller-rabin-uint32.c

   Functions for randomized primality testing for hashing applications.

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
#include "utilities-ds.h"

static bool composite(uint32_t n, int num_iter);
static bool witness(uint32_t a, uint32_t n);

/**
   Runs a randomized primality test.
*/
bool miller_rabin_uint32(uint32_t n){
  int num_iter = 100;
  if (n == 2){
    return true;
  }else if (n == 1 || !(n % 2)){
    return false;
  }else if (composite(n, num_iter)){
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
static bool composite(uint32_t n, int num_iter){
  assert(n & 1 && n > 2);
  uint32_t a;
  uint32_t upper = n - 2; //random in [2, n - 1] 
  srandom (time (0)); //sec resolution
  for (int i = 0; i < num_iter; i++){
    a = 2 + random_range_uint32(upper);
    if (witness(a, n)){return true;}
  }
  return false;
}

/**
   Determines if n is composite and a is its witness, otherwise n is likely 
   a prime.
*/
static bool witness(uint32_t a, uint32_t n){
  int t;
  uint32_t x[2];
  uint64_t u64;
  uint64_t n64 = n;
  represent_uint64(n64 - 1, &t, &u64);
  assert(t > 0); //n - 1 is even and >= 2
  assert(u64 < pow_two_uint64(32));
  x[0] = pow_mod_uint32(a, u64, n);
  for (int i = 0; i < t; i++){
    x[1] = pow_mod_uint32(x[0], 2, n);
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
