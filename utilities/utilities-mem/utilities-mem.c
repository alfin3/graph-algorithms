/**
   utilities-mem.c

   Utility functions for memory management.
*/

#include <stdio.h>
#include <stdlib.h>

static const size_t C_SIZE_ULIMIT = (size_t)-1;


/**
   size_t addition and multiplication with wrapped overflow checking.
*/

size_t add_sz_perror(size_t a, size_t b){
  if (a > C_SIZE_ULIMIT - b){
    perror("addition size_t overflow");
    exit(EXIT_FAILURE);
  }
  return a + b;
}

size_t mul_sz_perror(size_t a, size_t b){
  if (a > C_SIZE_ULIMIT / b){
    perror("multiplication size_t overflow");
    exit(EXIT_FAILURE);
  }
  return a * b;
}

/**
   size_t multiplication with wrapped overflow and zero checking. Useful
   for computing end pointers for pointer iteration.
*/
size_t mul_nzero_sz_perror(size_t a, size_t b){
  if (a < 1 || b < 1){
    perror("zero multiplication");
    exit(EXIT_FAILURE);
  }
  return mul_sz_perror(a, b);
}

/**
   Malloc, realloc, and calloc with wrapped error checking, including
   integer overflow checking. The latter is also included in calloc_perror
   because there is no guarantee that a calloc implementation checks for
   integer overflow.
*/

void *malloc_perror(size_t num, size_t size){
  void *ptr = NULL;
  if (num > C_SIZE_ULIMIT / size){
    perror("malloc integer overflow");
    exit(EXIT_FAILURE);
  }
  ptr = malloc(num * size);
  if (ptr == NULL){
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

void *realloc_perror(void *ptr, size_t num, size_t size){
  void *new_ptr = NULL;
  if (num > C_SIZE_ULIMIT / size){
    perror("realloc integer overflow");
    exit(EXIT_FAILURE);
  }
  new_ptr = realloc(ptr, num * size);
  if (new_ptr == NULL){
    perror("realloc failed");
    exit(EXIT_FAILURE);
  }
  return new_ptr;
}

void *calloc_perror(size_t num, size_t size){
  void *ptr = NULL;
  if (num > C_SIZE_ULIMIT / size){
    perror("calloc integer overflow");
    exit(EXIT_FAILURE);
  }
  ptr = calloc(num, size);
  if (ptr == NULL){
    perror("calloc failed");
    exit(EXIT_FAILURE);
  }
  return ptr;
}
