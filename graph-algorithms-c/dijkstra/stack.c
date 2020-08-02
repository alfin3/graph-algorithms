/**
   stack.c

   Examples of generic dynamically allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set in stack form of any objects. 
   
   Functions in stack.h and stack.c are similar to the implementation in 
   https://see.stanford.edu/Course/CS107 . 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "stack.h"

/** Main functions */

static void *elt_ptr(stack_t *s, int i);
static void stack_grow(stack_t *s);

/**
   Initializes a stack. 
   init_stack_size: integer > 0.
   free_elt_fn: non-NULL.
*/
void stack_init(stack_t *s,
	       int init_stack_size,
	       int elt_size,
	       void (*free_elt_fn)(void *)){
  s->stack_size = init_stack_size;
  assert(s->stack_size > 0);
  s->num_elts = 0;
  s->elt_size = elt_size;
  s->elts = malloc(init_stack_size * elt_size);
  assert(s->elts != NULL);
  s->free_elt_fn = free_elt_fn;
  assert(s->free_elt_fn != NULL);
}

/**
   Pushes an element onto a stack.
   elt: pointer to element pointer, if element is a multilayer object,
        pointer to element, if element is fully stored in elts array,
        elt_size reflects these cases.
*/
void stack_push(stack_t *s, void *elt){
  if (s->stack_size == s->num_elts){stack_grow(s);}
  int ix = s->num_elts;
  void *elt_target = elt_ptr(s, ix);
  memcpy(elt_target, elt, s->elt_size);
  s->num_elts++;
}

/**
   Pops an element from stack.
*/
void stack_pop(stack_t *s, void *elt){
  int ix = s->num_elts - 1;
  assert(ix >= 0);
  void *elt_source = elt_ptr(s, ix);
  memcpy(elt, elt_source, s->elt_size);
  s->num_elts--;
}

/**
   Frees elements, according to free_elt_fn, and element array.
*/
void stack_free(stack_t *s){
  for (int i = 0; i < s->num_elts; i++){
    s->free_elt_fn(elt_ptr(s, i));
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
   Doubles the size of stack. Amortized constant overhead for copying in 
   worst case of realloc calls. realloc's search of heap is O(size of heap), 
   in worst case.
*/
static void stack_grow(stack_t *s){
  s->stack_size *= 2;
  s->elts = realloc(s->elts, s->stack_size * s->elt_size);
  assert(s->elts != NULL);
}
