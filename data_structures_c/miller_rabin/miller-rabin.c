/**
   miller-rabin.c

   Struct declarations and declarations of accessible functions for 
   randomized primality testing for hashing applications.

   See man random for additional information on random number generation. 
   Please note that "the GNU C Library does not provide a cryptographic 
   random number generator"
   https://www.gnu.org/software/libc/manual/html_node/Unpredictable-Bytes.html

   TODO
   1) make a function for generating a with generator-provided 
   uniformity and full range 
   2) to make sure that no overflow can occur keep a and n at 
   <= 2^32 - 1 within a 64 bit cast, because (2^32 - 2)^2 < 2^64 - 1, 
   thus no overflow in product prior to mod n in rep_sq_pow_mod.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

static bool composite(uint64_t n, uint8_t num_iter);
static bool witness(uint64_t a, uint64_t n);
static void represent(uint64_t n, unsigned int *t, uint64_t *u);
static uint64_t rep_sq_pow_mod(uint64_t b, uint64_t n, uint64_t m);

bool miller_rabin_prime(uint64_t n){
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
   Returns true, if a witness is detected. If after num_iter rounds 
   no witness is detected, returns false.
*/
static bool composite(uint64_t n, uint8_t num_iter){
  uint64_t a;
  srandom (time (0)); //sec resolution
  for (int i = 0; i < num_iter; i++){
    //2 <= a <= min(2^31 + 2, n - 1), within 64 bits
    if (n - 1 > (uint64_t)RAND_MAX + 2){ 
      a = 2 + (uint64_t)random(); //uniform, but range problem
    }else{
      a = 2 + (uint64_t)random() % (n - 2); //currently nonuniform
    }
    if (witness(a, n)){
      return true;
    }
  }
  return false;
}

/**
   Determines if n is composite and a is its witness, otherwise n is 
   likely a prime.
*/
static bool witness(uint64_t a, uint64_t n){
  unsigned int t;
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
static void represent(uint64_t n, unsigned int *t, uint64_t *u){
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
   Computes mod m of the nth power in O(logn) time, based on the 
   binary representation of n and modular arithmetic :
   if a1 ≡ b1 (mod m) and a2 ≡ b2 (mod m) then a1 a2 ≡ b1 b2 (mod m).
 */
static uint64_t rep_sq_pow_mod(uint64_t b, uint64_t n, uint64_t m){
  // initial mods
  uint64_t a = 1; //1 mod m
  b = b % m;
  // mods of products
  while (n){
    if (n & 1){
      a = (a * b) % m; //update for each set bit
    }
    b = (b * b) % m; //repetitive squaring between updates
    n >>= 1;
  }
  return a;
}
