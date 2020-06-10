/**
   miller-rabin.h

   Struct declarations and declarations of accessible functions for 
   randomized primality testing for hashing applications.
*/

#ifndef MILLER_RABIN_H  
#define MILLER_RABIN_H

#include <stdint.h>
#include <stdbool.h>

/**
   
*/
bool miller_rabin_prime(uint64_t n);

#endif
