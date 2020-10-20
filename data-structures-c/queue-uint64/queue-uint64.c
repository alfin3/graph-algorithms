/**
   queue-uint64.c

   Implementation of a generic dynamically allocated fifo queue with upto
   (2^64 - 1) / elt_size elements.
   
   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the fifo queue form. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "queue-uint64.h"
#include "utilities-ds.h"

static void *elt_ptr(queue_uint64_t *q, uint64_t i);
static void queue_grow(queue_uint64_t *q);
static void queue_move(queue_uint64_t *q);

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
		       void (*free_elt_fn)(void *)){
  q->queue_size = init_queue_size;
  q->queue_max_size = (pow_two_uint64(63) +
                       (pow_two_uint64(63) - 1)) / elt_size;
  q->num_elts = 0;
  q->num_popped_elts = 0;
  q->elt_size = elt_size;
  q->elts = malloc(init_queue_size * elt_size);
  assert(q->elts != NULL);
  q->free_elt_fn = free_elt_fn;
}

/**
   Pushes an element into a queue. The elt parameter is not NULL.
*/
void queue_uint64_push(queue_uint64_t *q, void *elt){
  if (q->queue_size == q->num_popped_elts + q->num_elts){queue_grow(q);}
  uint64_t ix = q->num_popped_elts + q->num_elts;
  memcpy(elt_ptr(q, ix), elt, q->elt_size);
  q->num_elts++;
}

/**
   Pops an element from a queue. If the queue is empty, the memory block 
   pointed to by elt remains unchanged.
*/
void queue_uint64_pop(queue_uint64_t *q, void *elt){
  if (q->num_elts == 0){return;}
  uint64_t ix = q->num_popped_elts;
  memcpy(elt, elt_ptr(q, ix), q->elt_size);
  q->num_elts--;
  q->num_popped_elts++;
  if (q->queue_size - q->num_popped_elts <= q->num_popped_elts){
    queue_move(q);
  }
}

/**
   Frees a queue, and leaves a block of size sizeof(queue_uint64_t) pointed
   to by the q parameter.
*/
void queue_uint64_free(queue_uint64_t *q){
  if (q->free_elt_fn != NULL){
    for (uint64_t i = 0; i < q->num_elts; i++){
      q->free_elt_fn(elt_ptr(q, q->num_popped_elts + i));
    }
  }
  free(q->elts);
  q->elts = NULL;
}

/** Helper functions */

/**
   Computes a pointer to an element in an element array.
*/
static void *elt_ptr(queue_uint64_t *q, uint64_t i){
  return (void *)((char *)q->elts + i * q->elt_size);
}

/**
   Doubles the size of a queue, if possible. Amortized constant overhead for
   copying in the worst case of realloc calls. realloc's search is O(size of
   heap).
*/
static void queue_grow(queue_uint64_t *q){
  assert(q->queue_size < q->queue_max_size);
  if (q->queue_max_size - q->queue_size < q->queue_size){
    q->queue_size = q->queue_max_size;
  }else{
    q->queue_size *= 2;
  }
  q->elts = realloc(q->elts, q->queue_size * q->elt_size);
  assert(q->elts != NULL);
}

/**
   Moves elements to the beginning of the element array of a queue. 
   Constant overhead in the worst case because each element is 
   moved at most once (see queue_pop).
*/
static void queue_move(queue_uint64_t *q){
  void *target = q->elts;
  void *source = elt_ptr(q, q->num_popped_elts);
  memcpy(target, source, q->num_elts * q->elt_size); //no overlap guarantee
  q->num_popped_elts = 0;
}
