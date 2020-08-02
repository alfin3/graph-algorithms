/**
   stack.h

   Struct declarations and declarations of accessible functions for a generic 
   dynamically allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form.
   
   Functions in stack.h and stack.c are similar to the implementation at 
   https://see.stanford.edu/Course/CS107 . 
*/

#ifndef STACK_H  
#define STACK_H

typedef struct{
  int stack_size;
  int num_elts;
  int elt_size;
  void *elts;
  void (*free_elt_fn)(void *);
} stack_t;

/**
   Initializes a stack. 
   init_stack_size: > 0.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in stack_push, 
                then the element is fully copied into the elts array, and a 
                NULL as free_elt_fn is sufficient to free the stack;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in stack_push, then the 
                pointer to the element is copied into the elts array, and an
                element-specific free_elt_fn is necessary to free the stack.
*/
void stack_init(stack_t *s,
	       int init_stack_size,
	       int elt_size,
	       void (*free_elt_fn)(void *));

/**
   Pushes an element onto a stack.
*/
void stack_push(stack_t *s, void *elt);

/**
   Pops an element of a stack.
*/
void stack_pop(stack_t *s, void *elt);


/**
   Frees each element of a stack according to free_elt_fn, as well as 
   the element array of the stack.
*/
void stack_free(stack_t *s);

#endif
