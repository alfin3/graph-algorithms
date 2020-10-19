/**
   stack-uint64.c

   Implementation of a generic dynamically allocated stack with upto 2^64 - 1
   elements.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form. 
   
   Functions in stack.h and stack.c are similar to the implementation in 
   https://see.stanford.edu/Course/CS107 . 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "stack-uint64.h"
#include "utilities-ds.h"

static void *elt_ptr(stack_uint64_t *s, uint64_t i);
static void stack_grow(stack_uint64_t *s);

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
		       void (*free_elt_fn)(void *)){
  s->stack_size = init_stack_size;
  s->stack_max_size = pow_two_uint64(63) + (pow_two_uint64(63) - 1);
  s->num_elts = 0;
  s->elt_size = elt_size;
  s->elts = malloc(init_stack_size * elt_size);
  assert(s->elts != NULL);
  s->free_elt_fn = free_elt_fn;
}

/**
   Pushes an element onto a stack. The elt parameter is not NULL.
*/
void stack_uint64_push(stack_uint64_t *s, void *elt){
  if (s->num_elts == s->stack_size){stack_grow(s);}
  uint64_t ix = s->num_elts;
  memcpy(elt_ptr(s, ix), elt, s->elt_size);
  s->num_elts++;
}

/**
   Pops an element of a stack. If the stack is empty, the memory block 
   pointed to by elt remains unchanged.
*/
void stack_uint64_pop(stack_uint64_t *s, void *elt){
  if (s->num_elts == 0){return;}
  uint64_t ix = s->num_elts - 1;
  memcpy(elt, elt_ptr(s, ix), s->elt_size);
  s->num_elts--;
}

/**
   Frees a stack, and leaves a block of size sizeof(stack_uint64_t) pointed
   to by the s parameter.
*/
void stack_uint64_free(stack_uint64_t *s){
  if (s->free_elt_fn != NULL){
    for (uint64_t i = 0; i < s->num_elts; i++){
      s->free_elt_fn(elt_ptr(s, i));
    } 
  }
  free(s->elts);
  s->elts = NULL;
}

/** Helper functions */

/**
   Computes a pointer to an element in an element array.
*/
static void *elt_ptr(stack_uint64_t *s, uint64_t i){
  return (void *)((char *)s->elts + i * s->elt_size);
}

/**
   Doubles the size of a stack, if possible. Amortized constant overhead for 
   copying in the worst case of realloc calls. realloc's search is 
   O(size of heap).
*/
static void stack_grow(stack_uint64_t *s){
  assert(s->stack_size < s->stack_max_size);
  if (s->stack_max_size - s->stack_size < s->stack_size){
    s->stack_size = s->stack_max_size;
  }else{
    s->stack_size *= 2;
  }
  s->elts = realloc(s->elts, s->stack_size * s->elt_size);
  assert(s->elts != NULL);
}
