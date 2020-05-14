/**
   queue.h

   Struct declarations and declarations of accessible functions of generic 
   dynamicaly allocated fifo queue.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects, in fifo queue form 
*/

#ifndef QUEUE_H  
#define QUEUE_H

typedef struct{
  int queue_size;
  int num_elts;
  int num_popped_elts;
  int elt_size;
  void *elts;
  void (*free_elt_fn)(void *);
} queue_t;

/**
   Initializes a queue. 
   init_stack_size: integer > 0.
   free_elt_fn: non-NULL.
*/
void queue_init(queue_t *q,
	        int init_queue_size,
	        int elt_size,
	        void (*free_elt_fn)(void *));

/**
   Pushes an element into a queue.
   elt: pointer to element pointer, if element is a multilayer object,
        pointer to element, if element is fully stored in elts array,
        elt_size reflects these cases.
*/
void queue_push(queue_t *q, void *elt);

/**
   Pops an element from queue.
*/
void queue_pop(queue_t *q, void *elt);


/**
   Frees elements, according to free_elt_fn, and element array.
*/
void queue_free(queue_t *q);

#endif
