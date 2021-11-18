/**
   stack.c

   Implementation of a generic dynamically allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form.

   A distinction is made between an element and an "elt_size block". During
   an insertion a contiguous block of size elt_size ("elt_size block") is 
   copied into a stack. An element may be within a contiguous or non-
   contiguous memory block. Given an element, the user decides what is
   copied into the elt_size block of the stack. If the element is within a
   contiguous memory block, then it can be entirely copied as an elt_size
   block, or a pointer to it can be copied as an elt_size block. If the
   element is within a non-contiguous memory block, then a pointer to it is
   copied as an elt_size block.

   When a pointer to an element is copied into a stack as an elt_size block,
   the user can also decide if only the pointer or the entire element is
   deleted during the free operation. By setting free_elt to NULL, only the
   pointer is deleted. Otherwise, the deletion is performed according to a
   non-NULL free_elt. For example, when an in-memory set of images are used
   as elements and pointers are copied into a stack, then setting free_elt
   to NULL will not affect the original set of images throughout the
   lifetime of the stack.
  
   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation
   provides an error message and an exit is executed if an integer overflow
   is attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.
   
   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.
   
   Functions in stack.h and stack.c are similar to the implementation in 
   https://see.stanford.edu/Course/CS107, with i) additional bound
   parameters to minimize the memory use in problems with the prior
   knowledge of the count of elements, ii) integer overflow safety, iii)
   generalization to size_t to represent indices and sizes, iv) C89/C90
   and C99 compliance, and v) comments.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "utilities-mem.h"

static void stack_grow(stack_t *s);
static void *ptr(const void *block, size_t i, size_t size);
static void fprintf_stderr_exit(const char *s, int line);

/**
   Initializes a stack. By default the initialized stack can accomodate
   as many elt_size blocks as the system resources allow, starting from one
   elt_size block and growing by repetitive doubling.
   s           : pointer to a preallocated block of size sizeof(stack_t)
   elt_size    : non-zero size of an elt_size block; must account for
                 internal and trailing padding according to sizeof
   free_elt    : - NULL if only elt_size blocks should be deleted throughout
                 the lifetime of a stack (e.g. because elements were
                 entirely copied as elt_size blocks, or because pointers
                 were copied as elt_size blocks and only pointers should
                 be deleted)
                 - otherwise takes a pointer to the elt_size block of an
                 element as an argument, frees the memory of the element
                 except the elt_size block pointed to by the argument
*/
void stack_init(stack_t *s,
		size_t elt_size,
		void (*free_elt)(void *)){
  s->count = 1;
  s->init_count = 1;
  s->max_count = 0;
  s->num_elts = 0;
  s->elt_size = elt_size;
  s->elts = malloc_perror(s->init_count, elt_size);
  s->free_elt = free_elt;
}

/**
   Sets the count of an initially allocated stack to the count that can
   accomodate init_count elt_size blocks without reallocation. The growth of
   the stack is then achieved by repetitive doubling upto the count that
   equals to max_count. If max_count is greater than init_count and is not
   equal to init_count * 2**n for n > 0, then the last growth step sets
   the count of the stack to max_count. The operation is optionally called
   after stack_init is completed and before any other operation is called.
   s           : pointer to a stack_t struct initialized with stack_init
   init_count  : > 0 count of the elt_size blocks that can be simultaneously
                 present in an initial stack without reallocation
   max_count   : - if >= init_count, sets the maximum count of the elt_size
                 blocks that can be simultaneously present in a stack; an
                 error message is provided and an exit is executed if an
                 attempt is made to exceed it
                 - otherwise, the count of the elt_size blocks that can be
                 simultaneously present in a stack is only limited by the
                 available system resources 
*/
void stack_bound(stack_t *s,
		 size_t init_count,
		 size_t max_count){
  s->init_count = init_count;
  s->max_count = max_count;
  s->elts = realloc_perror(s->elts, s->init_count, s->elt_size);
}

/**
   Pushes an element onto a stack.
   s           : pointer to an initialized stack_t struct 
   elt         : non-NULL pointer to the elt_size block of an element
*/
void stack_push(stack_t *s, const void *elt){
  if (s->num_elts == s->count) stack_grow(s);
  memcpy(ptr(s->elts, s->num_elts, s->elt_size), elt, s->elt_size);
  s->num_elts++;
}

/**
   Pops an element of a stack.
   s           : pointer to an initialized stack_t struct   
   elt         : non-NULL pointer to a preallocated elt_size block; if the
                 stack is empty, the memory block pointed to by elt remains
                 unchanged
*/
void stack_pop(stack_t *s, void *elt){
  if (s->num_elts == 0) return;
  memcpy(elt,
	 ptr(s->elts, s->num_elts - 1, s->elt_size),
	 s->elt_size);
  s->num_elts--;
}

/**
   If a stack is not empty, returns a pointer to the first elt_size block,
   otherwise returns NULL. The returned pointer is guaranteed to point to
   the first element until a stack modifying operation is performed. If
   non-NULL, the returned pointed can be dereferenced as a pointer to the 
   type of the elt_size block, or as a character pointer.
   s           : pointer to an initialized stack_t struct 
*/
void *stack_first(const stack_t *s){
  if (s->num_elts == 0) return NULL;
  return ptr(s->elts, s->num_elts - 1, s->elt_size);
}

/**
   Frees the memory of all elements that are in a stack according to 
   free_elt, frees the memory of the stack, and leaves the block of size
   sizeof(stack_t) pointed to by the s parameter.
*/
void stack_free(stack_t *s){
  size_t i;
  if (s->free_elt != NULL){
    for (i = 0; i < s->num_elts; i++){
      s->free_elt(ptr(s->elts, i, s->elt_size));
    } 
  }
  free(s->elts);
  s->elts = NULL;
}

/** Helper functions */

/**
   Doubles the count of a stack, according to the bound parameter values
   and the available system resources. Amortized constant overhead for
   copying in the worst case of realloc calls. realloc's search is
   O(size of heap).
*/
static void stack_grow(stack_t *s){
  if (s->count == s->max_count){
    /* always enter if init_count == max_count */
    fprintf_stderr_exit("tried to exceed the count maximum", __LINE__);
  }else if (s->max_count > s->init_count &&
	    s->max_count - s->count < s->count){
    s->count = s->max_count;
  }else{
    /* always enter if init_count > max_count */
    s->count = mul_sz_perror(2, s->count);
  }
  s->elts = realloc_perror(s->elts, s->count, s->elt_size);
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

/**
   Prints an error message and exits.
*/
static void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
}
