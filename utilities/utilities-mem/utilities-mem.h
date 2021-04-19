/**
   utilities-mem.h

   Declarations of accessible utility functions for memory management.
*/

#ifndef UTILITIES_MEM_H
#define UTILITIES_MEM_H

#include <stdlib.h>

/**
   Malloc, realloc, and calloc with wrapped error checking, including
   integer overflow checking. The latter is also included in calloc_perror
   because there is no guarantee that a calloc implementation checks for
   integer overflow.
*/

void *malloc_perror(size_t num, size_t size);

void *realloc_perror(void *ptr, size_t num, size_t size);

void *calloc_perror(size_t num, size_t size);

#endif
