/**
   queue.c

   Implementation of a generic dynamically allocated fifo queue.
   
   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the fifo queue form. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "queue.h"

static void *elt_ptr(queue_t *q, int i);
static void queue_grow(queue_t *q);
static void queue_move(queue_t *q);

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
	        void (*free_elt_fn)(void *)){
  assert(init_queue_size > 0);
  q->queue_size = init_queue_size;
  q->num_elts = 0;
  q->num_popped_elts = 0;
  q->elt_size = elt_size;
  q->elts = malloc(init_queue_size * elt_size);
  assert(q->elts != NULL);
  q->free_elt_fn = free_elt_fn;
}

/**
   Pushes an element into a queue.
*/
void queue_push(queue_t *q, void *elt){
  if (q->queue_size == q->num_popped_elts + q->num_elts){queue_grow(q);}
  int ix = q->num_popped_elts + q->num_elts;
  void *elt_target = elt_ptr(q, ix);
  memcpy(elt_target, elt, q->elt_size);
  q->num_elts++;
}

/**
   Pops an element of a queue.
*/
void queue_pop(queue_t *q, void *elt){
  assert(q->num_elts > 0);
  int ix = q->num_popped_elts;
  void *elt_source = elt_ptr(q, ix);
  memcpy(elt, elt_source, q->elt_size);
  q->num_elts--;
  q->num_popped_elts++;
  if (q->queue_size <= 2 * q->num_popped_elts){queue_move(q);}
}

/**
   Frees each element of a queue according to free_elt_fn, as well as 
   the element array of the queue.
*/
void queue_free(queue_t *q){
  if (q->free_elt_fn != NULL){
    for (int i = 0; i < q->num_elts; i++){ 
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
static void *elt_ptr(queue_t *q, int i){
  return (void *)((char *)q->elts + i * q->elt_size);
}

/**
   Doubles the size of a queue. Amortized constant overhead for copying in 
   the worst case of realloc calls. realloc's search is O(size of heap).
*/
static void queue_grow(queue_t *q){
  q->queue_size *= 2;
  q->elts = realloc(q->elts, q->queue_size * q->elt_size);
  assert(q->elts != NULL);
}

/**
   Moves elements to the beginning of an element array of a queue. 
   Constant overhead in the worst case because each element is 
   moved at most once (see queue_pop).
*/
static void queue_move(queue_t *q){
  assert(q->num_popped_elts >= q->num_elts);
  void *target = q->elts;
  void *source = elt_ptr(q, q->num_popped_elts);
  int block_size = q->num_elts * q->elt_size;
  memcpy(target, source, block_size); //no overlap guarantee
  q->num_popped_elts = 0;
}
