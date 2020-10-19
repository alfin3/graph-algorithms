/**
   stack-uint64.h

   Struct declarations and declarations of accessible functions for a 
   generic dynamically allocated stack with upto (2^64 - 1) / elt_size 
   elements.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form.
   
   Functions in stack.h and stack.c are similar to the implementation at 
   https://see.stanford.edu/Course/CS107 . 
*/

#ifndef STACK_UINT64_H  
#define STACK_UINT64_H

#include <stdint.h>

typedef struct{
  uint64_t stack_size;
  uint64_t stack_max_size;
  uint64_t num_elts;
  int elt_size;
  void *elts;
  void (*free_elt_fn)(void *);
} stack_uint64_t;

/**
   Initializes a stack. 
   init_stack_size: > 0.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in 
                stack_uint64_push, then the element is fully copied into the 
                elts array, and a NULL as free_elt_fn is sufficient to free 
                the stack;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in stack_uint64_push, then 
                the pointer to the element is copied into the elts array, 
                and an element-specific free_elt_fn is necessary to free the 
                stack.
*/
void stack_uint64_init(stack_uint64_t *s,
		       uint64_t init_stack_size,
		       int elt_size,
		       void (*free_elt_fn)(void *));

/**
   Pushes an element onto a stack. The elt parameter is not NULL.
*/
void stack_uint64_push(stack_uint64_t *s, void *elt);

/**
   Pops an element of a stack. If the stack is empty, the memory block 
   pointed to by elt remains unchanged.
*/
void stack_uint64_pop(stack_uint64_t *s, void *elt);


/**
   Frees a stack, and leaves a block of size sizeof(stack_uint64_t) pointed
   to by the s parameter.
*/
void stack_uint64_free(stack_uint64_t *s);

#endif
