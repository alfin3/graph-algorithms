/**
   stack.c

   Implementation of a generic dynamically allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form. 
   
   Functions in stack.h and stack.c are similar to the implementation in 
   https://see.stanford.edu/Course/CS107. This implementation provides
   a generalization to size_t.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "stack.h"
#include "utilities-mem.h"

static void *elt_ptr(const void *elts, size_t i, size_t elt_size);
static void stack_grow(stack_t *s);

/**
   Initializes a stack. 
   init_count       : > 0
   elt_size         : - the size of an element, if the element is within a 
                      contiguous memory block
                      - the size of a pointer to an element, if the element 
                      is within a noncontiguous memory block
   free_elt         : - if an element is within a contiguous memory block, 
                      as reflected by elt_size, and a pointer to the element 
                      is passed as elt in stack_push, then the element is 
                      fully copied onto the stack, and NULL as free_elt is 
                      sufficient to free the stack;
                      - if an element is an object within a noncontiguous 
                      memory block, and a pointer to a pointer to the element
                      is passed as elt in stack_push, then the pointer to the
                      element is copied onto the stack, and an element-
                      specific free_elt, taking a pointer to a pointer to an
                      element as its only parameter, is necessary to free the stack.
*/
void stack_init(stack_t *s,
		size_t init_count,
		size_t elt_size,
		void (*free_elt)(void *)){
  s->count = init_count;
  s->count_max = STACK_COUNT_MAX; //the only use of the macro in the file
  s->num_elts = 0;
  s->elt_size = elt_size;
  s->elts = malloc_perror(init_count * elt_size);
  s->free_elt = free_elt;
}

/**
   Pushes an element onto a stack. The elt parameter is not NULL.
*/
void stack_push(stack_t *s, void *elt){
  if (s->num_elts == s->count) stack_grow(s);
  memcpy(elt_ptr(s->elts, s->num_elts, s->elt_size),
	 elt,
	 s->elt_size);
  s->num_elts++;
}

/**
   Pops an element of a stack. Elt points to a preallocated memory block of 
   size elt_size. If the stack is empty, the memory block pointed to by elt 
   remains unchanged.
*/
void stack_pop(stack_t *s, void *elt){
  if (s->num_elts == 0) return;
  memcpy(elt,
	 elt_ptr(s->elts, s->num_elts - 1, s->elt_size),
	 s->elt_size);
  s->num_elts--;
}

/**
   Frees a stack, and leaves a block of size sizeof(stack_t) pointed to by
   the s parameter.
*/
void stack_free(stack_t *s){
  if (s->free_elt != NULL){
    for (size_t i = 0; i < s->num_elts; i++){
      s->free_elt(elt_ptr(s->elts, i, s->elt_size));
    } 
  }
  free(s->elts);
  s->elts = NULL;
}

/** Helper functions */

/**
   Computes a pointer to an element in an element array.
*/
static void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
}

/**
   Doubles the size of a stack, if possible. Amortized constant overhead for 
   copying in the worst case of realloc calls. realloc's search is 
   O(size of heap).
*/
static void stack_grow(stack_t *s){
  if (s->count >= s->count_max){
    fprintf(stderr, "tried to exceed or exceeded the stack count maximum\n");
    exit(EXIT_FAILURE);
  }
  if (s->count_max - s->count < s->count){
    s->count = s->count_max;
  }else{
    s->count *= 2;
  }
  s->elts = realloc_perror(s->elts, s->count * s->elt_size);
}
