/**
   heap-uint32.h

   Struct declarations and declarations of accessible functions of a generic 
   dynamicaly allocated (min) heap with upto 2^32 - 2 elements.

   Through user-defined comparison and deallocation functions, the 
   implementation provides a dynamic set in the heap form for any element 
   objects associated with priority values of basic type (e.g. char, int, 
   long, double). 

   The implementation assumes that for every element in a heap, the 
   corresponding block of size elt_size, pointed to by the elt parameter in
   heap_uint32_push is unique (unique bit pattern). Because any object in 
   memory can be pushed with its unique pointer, this invariant only 
   prevents associating a given object in memory with more than one priority
   value in a heap.
*/

#ifndef HEAP_UINT32_H  
#define HEAP_UINT32_H

#include "ht-div-uint32.h"

typedef struct{
  uint32_t heap_size;
  uint32_t heap_max_size;
  uint32_t num_elts;
  int pty_size;
  int elt_size;
  void *ptys;
  void *elts;
  float alpha; //hash table load factor upper bound
  ht_div_uint32_t *ht;
  int (*cmp_pty_fn)(void *, void *);
  int (*cmp_elt_fn)(void *, void *);
  void (*free_elt_fn)(void *);
} heap_uint32_t;

/**
   Initializes a heap. 
   init_heap_size: > 0.
   cmp_pty_fn: > 0 if the first priority value is greater,
               < 0 if the first priority value is lower,
               0 otherwise.
   cmp_elt_fn: 0 if the bit patterns in two pointed blocks of size elt_size 
               match, non-zero otherwise.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in 
                heap_uint32_push, then the element is fully copied into the 
                elts array of a heap, and NULL as free_elt_fn is sufficient 
                to free the heap;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in heap_uint32_push, then the
                pointer to the element is copied into the elts array of a 
                heap, and an element-specific free_elt_fn is necessary to 
                free the heap.
*/
void heap_uint32_init(heap_uint32_t *h,
		      uint32_t init_heap_size,
		      int pty_size,
		      int elt_size,
		      int (*cmp_pty_fn)(void *, void *),
		      int (*cmp_elt_fn)(void *, void *),
		      void (*free_elt_fn)(void *));

/**
   Pushes an element not yet in a heap and an associated priority value. 
   Prior to pushing, the membership of an element can be tested, if 
   necessary, with heap_uint32_search in O(1 + alpha) time in expectation 
   under the simple uniform hashing assumption.
   elt: a pointer to a block of size elt_size that is either a continuous 
        memory block object (e.g. basic type, array, struct) or a pointer to
        a multilayered object, as reflected by elt_size.
   pty: a pointer to a block of size pty_size that is an object of basic 
        type (e.g. char, int, long, double), as reflected by pty_size.
*/
void heap_uint32_push(heap_uint32_t *h, void *pty, void *elt);

/** 
   Returns a pointer to the priority of an element in a heap or NULL if
   the element is not in the heap in O(1 + alpha) time in expectation under 
   the simple uniform hashing assumption. The returned pointer is guaranteed
   to point to the current priority value until another heap operation is 
   performed.
*/
void *heap_uint32_search(heap_uint32_t *h, void *elt);

/**
   Updates the priority value of an element that is already in a heap. Prior
   to updating, the membership of an element can be tested, if necessary, 
   with heap_uint32_search in O(1 + alpha) time in expectation under the 
   simple uniform hashing assumption.
*/
void heap_uint32_update(heap_uint32_t *h, void *pty, void *elt);

/**
   Pops an element associated with a minimal priority value in a heap 
   according to cmp_pty_fn. If the heap is empty, the memory blocks pointed 
   to by elt and pty remain unchanged.
*/
void heap_uint32_pop(heap_uint32_t *h, void *pty, void *elt);

/**
   Frees a heap and leaves a block of size sizeof(heap_uint32_t) pointed
   to by the h parameter.
*/
void heap_uint32_free(heap_uint32_t *h);

#endif
