/**
   miller-rabin.h

   Declarations of accessible functions for randomized primality testing 
   for hashing applications.
   
   See man random for additional information on random number generation. 
   Please note that "the GNU C Library does not provide a cryptographic 
   random number generator"
   https://www.gnu.org/software/libc/manual/html_node/Unpredictable-Bytes.html
   
   The implementation provides a "no overflow" guarantee given a number of
   type uint32_t, and preserves the generator-provided uniformity in random 
   processes.
*/

#ifndef MILLER_RABIN_H  
#define MILLER_RABIN_H

#include <stdint.h>
#include <stdbool.h>

/**
   
*/
bool miller_rabin_prime(uint64_t n);

#endif
