/**
   queue.c

   Implementation of a generic dynamically allocated fifo queue.
   
   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the fifo queue form. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "utilities-mem.h"

static void *elt_ptr(const void *elts, size_t i, size_t elt_size);
static void queue_grow(queue_t *q);
static void queue_move(queue_t *q);
static void fprintf_stderr_exit(const char *s, int line);

/**
   Initializes a queue.
   q                : pointer to a preallocated block of size sizeof(queue_t)
   init_count       : > 0
   elt_size         : - the size of an element, if the element is within a 
                      contiguous memory block
                      - the size of a pointer to an element, if the element 
                      is within a noncontiguous memory block
   free_elt         : - if an element is within a contiguous memory block, 
                      as reflected by elt_size, and a pointer to the element 
                      is passed as elt in queue_push, then the element is 
                      fully copied onto the queue, and NULL as free_elt is 
                      sufficient to free the queue;
                      - if an element is an object within a noncontiguous 
                      memory block, and a pointer to a pointer to the element
                      is passed as elt in queue_push, then the pointer to the
                      element is copied onto the queue, and an element-
                      specific free_elt, taking a pointer to a pointer to an
                      element as its parameter, is necessary to free the
                      queue.
*/
void queue_init(queue_t *q,
		size_t init_count,
		size_t elt_size,
		void (*free_elt)(void *)){
  q->count = init_count;
  q->count_max = QUEUE_COUNT_MAX; /* the only use of the macro in the file */
  if (q->count > q->count_max){
    fprintf_stderr_exit("init_count > count maximum", __LINE__);
  }
  q->num_elts = 0;
  q->num_popped_elts = 0;
  q->elt_size = elt_size;
  q->elts = malloc_perror(init_count, elt_size);
  q->free_elt = free_elt;
}

/**
   Pushes an element onto a queue. The elt parameter is not NULL.
*/
void queue_push(queue_t *q, const void *elt){
  if (q->count == q->num_popped_elts + q->num_elts) queue_grow(q);
  memcpy(elt_ptr(q->elts, q->num_popped_elts + q->num_elts, q->elt_size),
	 elt,
	 q->elt_size);
  q->num_elts++;
}

/**
   Pops an element off a queue. Elt points to a preallocated memory block of 
   size elt_size. If the queue is empty, the memory block pointed to by elt 
   remains unchanged.
*/
void queue_pop(queue_t *q, void *elt){
  if (q->num_elts == 0) return;
  memcpy(elt,
	 elt_ptr(q->elts, q->num_popped_elts, q->elt_size),
	 q->elt_size);
  q->num_elts--;
  q->num_popped_elts++;
  if (q->count - q->num_popped_elts <= q->num_popped_elts){
    queue_move(q);
  }
}

/**
   If a queue is not empty, returns a pointer to the first element,
   otherwise returns NULL. The returned pointer is guaranteed to point to
   the first element until a queue modifying operation is performed.
*/
void *queue_first(const queue_t *q){
  if (q->num_elts == 0) return NULL;
  return elt_ptr(q->elts, q->num_popped_elts, q->elt_size);
}

/**
   Frees a queue, and leaves a block of size sizeof(queue_t) pointed
   to by the q parameter.
*/
void queue_free(queue_t *q){
  size_t i;
  if (q->free_elt != NULL){
    for (i = 0; i < q->num_elts; i++){
      q->free_elt(elt_ptr(q->elts, q->num_popped_elts + i, q->elt_size));
    }
  }
  free(q->elts);
  q->elts = NULL;
}

/** Helper functions */

/**
   Computes a pointer to an element in an element array.
*/
static void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
}

/**
   Doubles the size of a queue, if possible. Amortized constant overhead for
   copying in the worst case of realloc calls. realloc's search is O(size of
   heap).
*/
static void queue_grow(queue_t *q){
  if (q->count == q->count_max){
    fprintf_stderr_exit("tried to exceed the count maximum", __LINE__);
  }
  if (q->count_max - q->count < q->count){
    q->count = q->count_max;
  }else{
    q->count *= 2;
  }
  q->elts = realloc_perror(q->elts, q->count, q->elt_size);
}

/**
   Moves elements to the beginning of the element array of a queue. 
   Constant overhead in the worst case because each element is 
   moved at most once. The destination and source regions must
   not overlap.
*/
static void queue_move(queue_t *q){
  memcpy(q->elts,
	 elt_ptr(q->elts, q->num_popped_elts, q->elt_size),
	 q->num_elts * q->elt_size);
  q->num_popped_elts = 0;
}

/**
   Prints an error message and exits.
*/
static void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
}
