/**
   queue-uint64.h

   Struct declarations and declarations of accessible functions of a generic 
   dynamically allocated fifo queue with upto (2^64 - 1) / elt_size elements.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the fifo queue form.
*/

#ifndef QUEUE_UINT64_H  
#define QUEUE_UINT64_H

#include <stdint.h>

typedef struct{
  uint64_t queue_size;
  uint64_t queue_max_size;
  uint64_t num_elts;
  uint64_t num_popped_elts;
  int elt_size;
  void *elts;
  void (*free_elt_fn)(void *);
} queue_uint64_t;

/**
   Initializes a queue. 
   init_queue_size: > 0.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in 
                queue_uint64_push, then the element is fully copied into the 
                elts array, and a NULL as free_elt_fn is sufficient to free 
                the queue;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in queue_uint64_push, then 
                the pointer to the element is copied into the elts array, 
                and an element-specific free_elt_fn is necessary to free the 
                queue.
*/
void queue_uint64_init(queue_uint64_t *q,
		       uint64_t init_queue_size,
		       int elt_size,
		       void (*free_elt_fn)(void *));

/**
   Pushes an element into a queue. The elt parameter is not NULL.
*/
void queue_uint64_push(queue_uint64_t *q, void *elt);

/**
   Pops an element from a queue. If the queue is empty, the memory block 
   pointed to by elt remains unchanged.
*/
void queue_uint64_pop(queue_uint64_t *q, void *elt);

/**
   If a queue is not empty, returns a pointer to the first element,
   otherwise returns NULL. The returned pointer is guaranteed to point to
   the first element until a queue modifying operation is performed.
*/
void *queue_uint64_first(queue_uint64_t *q);

/**
   Frees a queue, and leaves a block of size sizeof(queue_uint64_t) pointed
   to by the q parameter.
*/
void queue_uint64_free(queue_uint64_t *q);

#endif
