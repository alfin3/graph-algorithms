/**
   stack.c

   Implementation of a generic dynamically allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form. 
   
   Functions in stack.h and stack.c are similar to the implementation in 
   https://see.stanford.edu/Course/CS107 . 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "stack.h"

static void *elt_ptr(stack_t *s, int i);
static void stack_grow(stack_t *s);

/**
   Initializes a stack. 
   init_stack_size: > 0.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in stack_push, 
                then the element is fully copied into the elts array, and a 
                NULL as free_elt_fn is sufficient to free the stack;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in stack_push, then a 
                pointer to the element is copied into the elts array, and an
                element-specific free_elt_fn is necessary to free the stack.
*/
void stack_init(stack_t *s,
		int init_stack_size,
		int elt_size,
		void (*free_elt_fn)(void *)){
  assert(init_stack_size > 0);
  s->stack_size = init_stack_size;
  s->num_elts = 0;
  s->elt_size = elt_size;
  s->elts = malloc(init_stack_size * elt_size);
  assert(s->elts != NULL);
  s->free_elt_fn = free_elt_fn;
}

/**
   Pushes an element onto a stack.
*/
void stack_push(stack_t *s, void *elt){
  if (s->stack_size == s->num_elts){stack_grow(s);}
  int ix = s->num_elts;
  void *elt_target = elt_ptr(s, ix);
  memcpy(elt_target, elt, s->elt_size);
  s->num_elts++;
}

/**
   Pops an element of a stack.
*/
void stack_pop(stack_t *s, void *elt){
  int ix = s->num_elts - 1;
  assert(ix >= 0);
  void *elt_source = elt_ptr(s, ix);
  memcpy(elt, elt_source, s->elt_size);
  s->num_elts--;
}

/**
   Frees the elements of a stack, according to free_elt_fn, as well as 
   the element array of the stack.
*/
void stack_free(stack_t *s){
  if (s->free_elt_fn != NULL){
    for (int i = 0; i < s->num_elts; i++){
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
static void *elt_ptr(stack_t *s, int i){
  return (void *)((char *)s->elts + i * s->elt_size);
}

/**
   Doubles the size of a stack. Amortized constant overhead for copying in 
   the worst case of realloc calls. realloc's search is O(size of heap).
*/
static void stack_grow(stack_t *s){
  s->stack_size *= 2;
  s->elts = realloc(s->elts, s->stack_size * s->elt_size);
  assert(s->elts != NULL);
}
