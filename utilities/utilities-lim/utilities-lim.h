/**
   utilities-lim.h

   Numerical limits in addition to limits.h requiring only C89/C99.
*/

#ifndef UTILITIES_LIM_H
#define UTILITIES_LIM_H

#include <limits.h>

/**
   Provides the precision number of bits given the maximum value of an integer
   from 1 to 2**2040 - 1. The width of an unsigned integer is the same as
   precision. Adopted from:
   https://groups.google.com/g/comp.lang.c/c/1kiXXt5T0TQ/m/S_B_8D4VmOkJ
*/
#define PRECISION_FROM_ULIMIT(m) ((m) / ((m) % 255u + 1u) / 255u % 255u * 8u \
				  + 7u - \
				  86u / ((m) % 255u + 12u))

#endif
