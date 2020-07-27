/**
   heap-uint32.h

   Struct declarations and declarations of accessible functions of a generic 
   dynamicaly allocated (min) heap with upto 2^32 - 2 elements.

   Through user-defined comparison and deallocation functions, the 
   implementation provides a dynamic set in the heap form of any element 
   objects associated with priority values of basic type (e.g. char, int, 
   long, double). 

   The implementation assumes that for every element in a heap, the 
   corresponding block of size elt_size, pointed to by the elt parameter in
   heap_push is unique. Because any object in memory can be pushed with its 
   unique pointer, this invariant only prevents associating a given object in 
   memory with more than one priority value in the heap.

   The overflow-safe design of uint32_t index tests in heapify functions 
   enables a potentially simple upgrade to uint64_t number of elements.
*/

#ifndef HEAP_UINT32_H  
#define HEAP_UINT32_H

#include <stdbool.h>
#include "ht-div-uint32.h"

typedef struct{
  uint32_t heap_size;
  uint32_t heap_max_size;
  uint32_t num_elts;
  int pty_size;
  int elt_size;
  void *ptys;
  void *elts;
  float alpha;
  ht_div_uint32_t *ht;
  int (*cmp_pty_fn)(void *, void *);
  int (*cmp_elt_fn)(void *, void *); //compares two blocks of size elt_size
  void (*free_elt_fn)(void *);
} heap_t;

/**
   Initializes a heap. 
   init_heap_size: > 0.
   cmp_pty_fn: > 0 if the first priority value is greater,
               < 0 if the first priority value is lower,
               0 otherwise.
   cmp_elt_fn: 0 if the bits in two pointed blocks of size elt_size match, 
               non-zero otherwise.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in heap_push, 
                then the element is fully copied into the elts array of a 
                heap, and NULL as free_elt_fn is sufficient to free the heap;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in heap_push, then the pointer
                to the element is copied into the elts array of a heap, and an
                element-specific free_elt_fn is necessary to free the heap.
*/
void heap_init(heap_t *h,
	       uint32_t init_heap_size,
	       int pty_size,
	       int elt_size,
	       int (*cmp_pty_fn)(void *, void *),
	       int (*cmp_elt_fn)(void *, void *),
	       void (*free_elt_fn)(void *));

/**
   Pushes an element not yet in a heap and an associated priority value. 
   elt: a pointer to a block of size elt_size that is either a
        continuous memory block object (e.g. basic type, array, struct) or a
        pointer to an object, as reflected by elt_size.
   pty: a pointer to a block of size pty_size that is an object of basic 
        type (e.g. char, int, long, double), as reflected by pty_size.
*/
void heap_push(heap_t *h, void *pty, void *elt);

/** 
   Provides a membership test in O(1 + alpha) time in expectation under the 
   simple uniform hashing assumption.
*/
bool heap_member(heap_t *h, void *elt);

/**
   Updates the priority of an element that is already in a heap.
*/
void heap_update(heap_t *h, void *pty, void *elt);

/**
   Pops an element associated with a minimal priority according to cmp_pty_fn.
   If a heap is empty, the memory blocks pointed to by elt and pty remain 
   unchanged.
*/
void heap_pop(heap_t *h, void *pty, void *elt);

/**
   Frees the dynamically allocated components of a heap.
*/
void heap_free(heap_t *h);

#endif
