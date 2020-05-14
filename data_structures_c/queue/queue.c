/**
   queue.c

   Implementation of generic dynamically allocated fifo queue.
   
   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects, in fifo queue form. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "queue.h"

/** Main functions */

static void *elt_ptr(queue_t *q, int i);
static void queue_grow(queue_t *q);
static void queue_move(queue_t *q);

/**
   Initializes a queue. 
   init_queue_size: integer > 0.
   free_elt_fn: non-NULL.
*/
void queue_init(queue_t *q,
	        int init_queue_size,
	        int elt_size,
	        void (*free_elt_fn)(void *)){
  q->queue_size = init_queue_size;
  assert(q->queue_size > 0);
  q->num_elts = 0;
  q->num_popped_elts = 0;
  q->elt_size = elt_size;
  q->elts = malloc(init_queue_size * elt_size);
  assert(q->elts != NULL);
  q->free_elt_fn = free_elt_fn;
  assert(q->free_elt_fn != NULL);
}

/**
   Pushes an element into a queue.
   elt: pointer to element pointer, if element is a multilayer object,
        pointer to element, if element is fully stored in elts array,
        elt_size reflects these cases.
*/
void queue_push(queue_t *q, void *elt){
  if (q->queue_size == q->num_elts){queue_grow(q);}
  int ix = q->num_elts;
  void *elt_target = elt_ptr(q, ix);
  memcpy(elt_target, elt, q->elt_size);
  q->num_elts++;
}

/**
   Pops an element from queue.
*/
void queue_pop(queue_t *q, void *elt){
  assert(q->num_elts - q->num_popped_elts > 0);
  int ix = q->num_popped_elts;
  void *elt_source = elt_ptr(q, ix);
  memcpy(elt, elt_source, q->elt_size);
  q->num_popped_elts++;
  if (q->queue_size <= 2 * q->num_popped_elts){queue_move(q);}
}

/**
   Frees remaining elements, according to free_elt_fn, and element array.
*/
void queue_free(queue_t *q){
  for (int i = q->num_popped_elts; i < q->num_elts; i++){
    q->free_elt_fn(elt_ptr(q, i));
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
   Doubles the size of queue. Amortized constant overhead for copying in 
   worst case of realloc calls. realloc's search of heap is O(size of heap), 
   in worst case.
*/
static void queue_grow(queue_t *q){
  q->queue_size *= 2;
  q->elts = realloc(q->elts, q->queue_size * q->elt_size);
  assert(q->elts != NULL);
}

/**
   Moves elements to the beginning of element array. Each popped element 
   is moved at most once.
*/
static void queue_move(queue_t *q){
  void *target = q->elts;
  void *source = elt_ptr(q, q->num_popped_elts);
  int block_size = (q->num_elts - q->num_popped_elts) * q->elt_size;
  memcpy(target, source, block_size); //no overlap guarantee
  q->num_elts -= q->num_popped_elts;
  q->num_popped_elts = 0;
}
