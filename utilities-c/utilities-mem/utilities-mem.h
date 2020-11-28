/**
   utilities-mem.h

   Declarations of accessible utility functions for memory management.
*/

#ifndef UTILITIES_MEM_H
#define UTILITIES_MEM_H

#include <stdlib.h>

/**
   Malloc, realloc, and calloc with wrapped error checking.
*/

void *malloc_perror(size_t size);

void *realloc_perror(void *ptr, size_t new_size);

void *calloc_perror(size_t num, size_t size);

#endif
