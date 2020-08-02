/**
   queue.h

   Struct declarations and declarations of accessible functions of a generic 
   dynamically allocated fifo queue.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the fifo queue form.
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
   init_queue_size: > 0.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in queue_push, 
                then the element is fully copied into the elts array, and a 
                NULL as free_elt_fn is sufficient to free the queue;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in queue_push, then the
                pointer to the element is copied into the elts array, and an
                element-specific free_elt_fn is necessary to free the queue.
*/
void queue_init(queue_t *q,
	        int init_queue_size,
	        int elt_size,
	        void (*free_elt_fn)(void *));

/**
   Pushes an element into a queue.
*/
void queue_push(queue_t *q, void *elt);

/**
   Pops an element of a queue.
*/
void queue_pop(queue_t *q, void *elt);

/**
   Frees each element of a queue according to free_elt_fn, as well as 
   the element array of the queue.
*/
void queue_free(queue_t *q);

#endif
