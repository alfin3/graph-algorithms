/**
   miller-rabin-uint64.c

   Functions for randomized primality testing for hashing applications.

   See man random for additional information on random number generation. 
   Please note that "the GNU C Library does not provide a cryptographic 
   random number generator"
   https://www.gnu.org/software/libc/manual/html_node/Unpredictable-Bytes.html
   
   The implementation provides a "no overflow" guarantee given a number of
   type uint64_t, and preserves the generator-provided uniformity in random 
   processes. The generator is not seeded by miller_rabin_uint64.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include "utilities-ds.h"

static bool composite(uint64_t n, int num_iter);
static bool witness(uint64_t a, uint64_t n);

/**
   Runs a randomized primality test.
*/
bool miller_rabin_uint64(uint64_t n){
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
static bool composite(uint64_t n, int num_iter){
  assert(n & 1 && n > 2);
  uint64_t a;
  uint64_t upper = n - 3; //random in [2, n - 1] 
  for (int i = 0; i < num_iter; i++){
    a = 2 + random_range_uint64(upper);
    if (witness(a, n)){return true;}
  }
  return false;
}

/**
   Determines if n is composite and a is its witness, otherwise n is likely 
   a prime.
*/
static bool witness(uint64_t a, uint64_t n){
  int t;
  uint64_t x[2];
  uint64_t u;
  represent_uint64(n - 1, &t, &u);
  assert(t > 0); //n - 1 is even and >= 2
  x[0] = pow_mod_uint64(a, u, n);
  for (int i = 0; i < t; i++){
    x[1] = pow_mod_uint64(x[0], 2, n);
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
