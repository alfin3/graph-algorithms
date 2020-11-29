/**
   utilities-mem.c

   Utility functions for memory management.
*/

#include <stdio.h>
#include <stdlib.h>
#include "utilities-mem.h"

/**
   Malloc, realloc, and calloc with wrapped error checking.
*/

void *malloc_perror(size_t size){
  void *ptr = malloc(size);
  if (ptr == NULL){
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

void *realloc_perror(void *ptr, size_t new_size){
  void *new_ptr = realloc(ptr, new_size);
  if (new_ptr == NULL){
    perror("realloc failed");
    exit(EXIT_FAILURE);
  }
  return new_ptr;
}

void *calloc_perror(size_t num, size_t size){
  void *ptr = calloc(num, size);
  if (ptr == NULL){
    perror("calloc failed");
    exit(EXIT_FAILURE);
  }
  return ptr;
}
