/**
   queue.h

   Struct declarations and declarations of accessible functions of a
   generic dynamically allocated queue, providing a dynamic set of generic
   elements in the fifo queue form.

   A distinction is made between an element and an "elt_size block". During
   an insertion a contiguous block of size elt_size ("elt_size block") is 
   copied into a queue. An element may be within a contiguous or non-
   contiguous memory block. Given an element, the user decides what is
   copied into the elt_size block of the queue. If the element is within a
   contiguous memory block, then it can be entirely copied as an elt_size
   block, or a pointer to it can be copied as an elt_size block. If the
   element is within a non-contiguous memory block, then a pointer to it is
   copied as an elt_size block.

   When a pointer to an element is copied into a queue as an elt_size block,
   the user can also decide if only the pointer or the entire element is
   deleted during the free operation. By setting free_elt to NULL, only the
   pointer is deleted. Otherwise, the deletion is performed according to a
   non-NULL free_elt. For example, when an in-memory set of images are used
   as elements and pointers are copied into a queue, then setting free_elt
   to NULL will not affect the original set of images throughout the
   lifetime of the queue.
  
   The implementation is cache-efficient and provides a constant overhead
   per elt_size block across push and pop operations by maintaining the
   invariant that an elt_size block is copied within a queue at most once
   throughout its lifetime in the queue.
  
   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation
   provides an error message and an exit is executed if an integer overflow
   is attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.
   
   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.
*/


#ifndef QUEUE_H  
#define QUEUE_H

#include <stddef.h>

struct queue{
  size_t count;
  size_t init_count;
  size_t max_count;
  size_t num_elts;
  size_t num_popped_elts;
  size_t elt_size;
  void *elts;
  void (*free_elt)(void *);
};

/**
   Initializes a queue. By default the initialized queue can accomodate
   as many elt_size blocks as the system resources allow, starting from two
   elt_size blocks and growing by repetitive doubling.
   q           : pointer to a preallocated block of size sizeof(struct queue)
   elt_size    : non-zero size of an elt_size block; must account for
                 internal and trailing padding according to sizeof
   free_elt    : - NULL if only elt_size blocks should be deleted in the
                 queue_free operation (e.g. because elements were
                 entirely copied as elt_size blocks, or because pointers
                 were copied as elt_size blocks and only pointers should
                 be deleted)
                 - otherwise takes a pointer to the elt_size block of an
                 element as an argument, frees the memory of the element
                 except the elt_size block pointed to by the argument
*/
void queue_init(struct queue *q,
		size_t elt_size,
		void (*free_elt)(void *));

/**
   Sets the count of the elt_size blocks that can be simultaneously present
   in an initial queue without reallocation. The growth of the queue is then
   achieved by repetitive doubling upto the count that can accomodate
   max_count simultaneously present elt_size blocks. The bounds are valid
   for any sequence of push and pop operations. If max_count is greater than
   init_count and is not equal to init_count * 2**n for n > 0, then the last
   growth step sets the count of the queue to a count that accomodates
   max_count simultaneously present elt_size blocks. The operation is
   optionally called after queue_init is completed and before any other
   operation is called.
   q           : pointer to a queue struct initialized with queue_init
   init_count  : > 0 count of the elt_size blocks that can be
                 simultaneously present in an initial queue without
                 reallocation for any sequence of push and pop
                 operations
   max_count   : - if >= init_count, sets the maximum count of
                 the elt_size blocks that can be simultaneously present in a
                 queue for any sequence of push and pop operations; an error
                 message is provided and an exit is executed if an attempt
                 is made to exceed the allocated memory of the queue in
                 queue_push, which may accomodate upto 2 * max_count
                 elt_size blocks depending on the sequence of push and pop
                 operations
                 - otherwise, the count of the elt_size blocks that can be
                 simultaneously present in a queue is only limited by the
                 available system resources 
*/
void queue_bound(struct queue *q,
		 size_t init_count,
		 size_t max_count);

/**
   Pushes an element onto a queue.
   q           : pointer to an initialized queue struct 
   elt         : non-NULL pointer to the elt_size block of an element
*/
void queue_push(struct queue *q, const void *elt);

/**
   Pops an element of a queue.
   q           : pointer to an initialized queue struct   
   elt         : non-NULL pointer to a preallocated elt_size block; if the
                 queue is empty, the memory block pointed to by elt remains
                 unchanged
*/
void queue_pop(struct queue *q, void *elt);

/**
   If a queue is not empty, returns a pointer to the first elt_size block,
   otherwise returns NULL. The returned pointer is guaranteed to point to
   the first element until a queue modifying operation is performed. If
   non-NULL, the returned pointer can be dereferenced as a pointer to the 
   type of the elt_size block, or as a character pointer.
   s           : pointer to an initialized queue struct 
*/
void *queue_first(const struct queue *q);

/**
   Frees the memory of all elements that are in a queue according to 
   free_elt, frees the memory of the queue, and leaves the block of size
   sizeof(struct queue) pointed to by the q parameter.
*/
void queue_free(struct queue *q);

#endif
