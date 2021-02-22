/**
   utilities-rand-uint32.h

*/

#ifndef UTILITIES_RAND_UINT32_H  
#define UTILITIES_RAND_UINT32_H

#include <stdint.h>

/**
   Returns a generator-uniform random uint32_t in [0 , n).
*/
uint32_t random_range_uint32(uint32_t n);

/**
   Returns a generator-uniform random uint32_t. 
*/
uint32_t random_uint32();

#endif
