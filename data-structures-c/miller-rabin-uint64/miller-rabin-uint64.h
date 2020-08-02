/**
   miller-rabin-uint64.h

   Declarations of accessible functions for randomized primality testing 
   for hashing applications.
   
   See man random for additional information on random number generation. 
   Please note that "the GNU C Library does not provide a cryptographic 
   random number generator"
   https://www.gnu.org/software/libc/manual/html_node/Unpredictable-Bytes.html
   
   The implementation provides a "no overflow" guarantee given a number of
   type uint64_t, and preserves the generator-provided uniformity in random 
   processes. The generator is not seeded by miller_rabin_uint64.
*/

#ifndef MILLER_RABIN_UINT64_H  
#define MILLER_RABIN_UINT64_H

#include <stdint.h>
#include <stdbool.h>

/**
   Runs a randomized primality test.
*/
bool miller_rabin_uint64(uint64_t n);

#endif
