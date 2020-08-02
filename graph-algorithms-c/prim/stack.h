/**
   stack.h

   Struct declarations and declarations of accessible functions of generic 
   dynamicaly allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set in stack form of any objects. 
   
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
   init_stack_size: integer > 0.
   free_elt_fn: non-NULL.
*/
void stack_init(stack_t *s,
	       int init_stack_size,
	       int elt_size,
	       void (*free_elt_fn)(void *));

/**
   Pushes an element onto a stack.
   elt: pointer to element pointer, if element is a multilayer object,
        pointer to element, if element is fully stored in elts array,
        elt_size reflects these cases.
*/
void stack_push(stack_t *s, void *elt);

/**
   Pops an element from stack.
*/
void stack_pop(stack_t *s, void *elt);


/**
   Frees elements, according to free_elt_fn, and element array.
*/
void stack_free(stack_t *s);

#endif
