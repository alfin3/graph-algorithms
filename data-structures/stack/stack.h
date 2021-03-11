/**
   stack.h

   Struct declarations, declarations of accessible functions, and macro
   definitions for a generic dynamically allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form. 
   
   Functions in stack.h and stack.c are similar to the implementation in 
   https://see.stanford.edu/Course/CS107. This implementation provides
   a generalization to size_t.
*/

#ifndef STACK_H  
#define STACK_H

#include <stddef.h>

typedef struct{
  size_t count;
  size_t count_max;
  size_t num_elts;
  size_t elt_size;
  void *elts;
  void (*free_elt)(void *);
} stack_t;

/**
   Initializes a stack.
   s                : pointer to a preallocated block of size sizeof(stack_t)
   init_count       : > 0
   elt_size         : - the size of an element, if the element is within a
                      contiguous memory block
                      - the size of a pointer to an element, if the element
                      is within a noncontiguous memory block
   free_elt         : - if an element is within a contiguous memory block,
                      as reflected by elt_size, and a pointer to the element
                      is passed as elt in stack_push, then the element is
                      fully copied onto the stack, and NULL as free_elt is
                      sufficient to free the stack
                      - if an element is an object within a noncontiguous
                      memory block, and a pointer to a pointer to the element
                      is passed as elt in stack_push, then the pointer to the
                      element is copied onto the stack, and an element-
                      specific free_elt, taking a pointer to a pointer to an
                      element as its parameter, is necessary to free the
                      stack.
*/
void stack_init(stack_t *s,
		size_t init_count,
		size_t elt_size,
		void (*free_elt)(void *));

/**
   Pushes an element onto a stack. The elt parameter is not NULL.
*/
void stack_push(stack_t *s, const void *elt);

/**
   Pops an element of a stack. Elt points to a preallocated memory block of
   size elt_size. If the stack is empty, the memory block pointed to by elt
   remains unchanged.
*/
void stack_pop(stack_t *s, void *elt);

/**
   If a stack is not empty, returns a pointer to the first element,
   otherwise returns NULL. The returned pointer is guaranteed to point to
   the first element until a stack modifying operation is performed.
*/
void *stack_first(const stack_t *s);

/**
   Frees a stack, and leaves a block of size sizeof(stack_t) pointed to by
   the s parameter.
*/
void stack_free(stack_t *s);

/**
   Sets the stack count maximum that may be reached, if possible, as a stack
   grows by repetitive doubling from its initial count and by adding, if
   necessary, the difference between STACK_COUNT_MAX and the last count in
   the last step.

   The program exits with an error message, if a) the value of the init_count
   parameter in stack_init is greater than STACK_COUNT_MAX, or b) if a stack
   growth step is attempted after STACK_COUNT_MAX was reached. The macro is
   set to the maximal value of size_t by default and is used as size_t.
*/
#define STACK_COUNT_MAX ((size_t)-1)

#endif
