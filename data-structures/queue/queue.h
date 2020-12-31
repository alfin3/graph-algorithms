/**
   queue.h

   Struct declarations and declarations of accessible functions of a generic 
   dynamically allocated fifo queue.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the fifo queue form.
*/

#ifndef QUEUE_H  
#define QUEUE_H

#include <stdlib.h>

typedef struct{
  size_t count;
  size_t count_max;
  size_t num_elts;
  size_t num_popped_elts;
  size_t elt_size;
  void *elts;
  void (*free_elt)(void *);
} queue_t;

/**
   Initializes a queue. 
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
                      element as its only parameter, is necessary to free the
                      queue.
*/
void queue_init(queue_t *q,
		size_t init_count,
		size_t elt_size,
		void (*free_elt)(void *));

/**
   Pushes an element onto a queue. The elt parameter is not NULL.
*/
void queue_push(queue_t *q, void *elt);

/**
   Pops an element off a queue. Elt points to a preallocated memory block of 
   size elt_size. If the queue is empty, the memory block pointed to by elt 
   remains unchanged.
*/
void queue_pop(queue_t *q, void *elt);

/**
   If a queue is not empty, returns a pointer to the first element,
   otherwise returns NULL. The returned pointer is guaranteed to point to
   the first element until a queue modifying operation is performed.
*/
void *queue_first(queue_t *q);

/**
   Frees a queue, and leaves a block of size sizeof(queue_t) pointed
   to by the q parameter.
*/
void queue_free(queue_t *q);

/**
   Sets the queue count maximum reached, if possible, by growing the queue
   from its initial count. By default it is set to SIZE_MAX. QUEUE_COUNT_MAX
   must be greater or equal to the value of the init_count parameter in
   stack_init, otherwise the program will exit with an error message. The
   program will also exit with an error message if it tries to grow the queue
   and exceed QUEUE_COUNT_MAX. The macro is used as size_t.
*/
#define QUEUE_COUNT_MAX (SIZE_MAX)

#endif
