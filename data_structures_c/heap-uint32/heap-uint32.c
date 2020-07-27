/**
   heap-uint32.c

   Implementation of a generic dynamicaly allocated (min) heap with upto 
   2^32 - 2 elements.

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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "heap-uint32.h"
#include "ht-div-uint32.h"
#include "utilities-ds.h"

static void *pty_ptr(heap_t *h, uint32_t i);
static void *elt_ptr(heap_t *h, uint32_t i);
static void swap(heap_t *h, uint32_t i, uint32_t j);
static void heap_grow(heap_t *h);
static void heapify_up(heap_t *h, uint32_t i);
static void heapify_down(heap_t *h, uint32_t i);

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
	       void (*free_elt_fn)(void *)){
  h->heap_size = init_heap_size;
  assert(h->heap_size > 0);
  //2^32 - 2 due to uint32_t index tests in heapify_down involving i + 2
  h->heap_max_size = (uint32_t)(pow_two_uint64(32) - 2);
  assert(h->heap_size <= h->heap_max_size);
  h->num_elts = 0;
  h->pty_size = pty_size;
  h->elt_size = elt_size;
  h->ptys = malloc(init_heap_size * pty_size);
  assert(h->ptys != NULL);
  h->elts = malloc(init_heap_size * elt_size);
  assert(h->elts != NULL);
  h->alpha = 1.0;
  h->ht = malloc(sizeof(ht_div_uint32_t));
  assert(h->ht != NULL);
  //an ht mapping elt_size blocks to uint32_t indices
  ht_div_uint32_init(h->ht,
                     elt_size, //key_size for ht purposes
	             sizeof(uint32_t),
		     h->alpha,
                     cmp_elt_fn, //compares two blocks of size elt_size
		     NULL);
  h->cmp_elt_fn = cmp_elt_fn;
  h->cmp_pty_fn = cmp_pty_fn;
  h->free_elt_fn = free_elt_fn;
}

/**
   Pushes an element not yet in a heap and an associated priority value. 
   elt: a pointer to a block of size elt_size that is either a
        continuous memory block object (e.g. basic type, array, struct) or a
        pointer to an object, as reflected by elt_size.
   pty: a pointer to a block of size pty_size that is an object of basic 
        type (e.g. char, int, long, double), as reflected by pty_size.
*/
void heap_push(heap_t *h, void *pty, void *elt){
  if (h->heap_size == h->num_elts){heap_grow(h);}
  uint32_t ix = h->num_elts;
  memcpy(elt_ptr(h, ix), elt, h->elt_size);
  memcpy(pty_ptr(h, ix), pty, h->pty_size);
  ht_div_uint32_insert(h->ht, elt, &ix);
  h->num_elts++;
  heapify_up(h, ix);
}

/** 
   Provides a membership test in O(1 + alpha) time in expectation under the 
   simple uniform hashing assumption.
*/
bool heap_member(heap_t *h, void *elt){
  if (ht_div_uint32_search(h->ht, elt) != NULL){return true;}
  return false;
}

/**
   Updates the priority of an element that is already in a heap.
*/
void heap_update(heap_t *h, void *pty, void *elt){
  uint32_t *ix_ptr = ht_div_uint32_search(h->ht, elt);
  assert(ix_ptr != NULL);
  uint32_t ix = *ix_ptr;
  memcpy(pty_ptr(h, ix), pty, h->pty_size);
  heapify_up(h, ix);
  heapify_down(h, ix);
}

/**
   Pops an element associated with a minimal priority according to cmp_pty_fn.
   If a heap is empty, the memory blocks pointed to by elt and pty remain 
   unchanged.
*/
void heap_pop(heap_t *h, void *pty, void *elt){
  if (h->num_elts == 0){return;}
  uint32_t ix = 0;
  uint32_t buf;
  memcpy(elt, elt_ptr(h, ix), h->elt_size);
  memcpy(pty, pty_ptr(h, ix), h->pty_size);
  swap(h, ix, h->num_elts - 1);
  ht_div_uint32_remove(h->ht, elt, &buf);
  h->num_elts--;
  heapify_down(h, ix);
}

/**
   Frees the dynamically allocated components of a heap.
*/
void heap_free(heap_t *h){
  if (h->free_elt_fn != NULL){
    for (uint32_t i = 0; i < h->num_elts; i++){
      h->free_elt_fn(elt_ptr(h, i));
    } 
  }
  free(h->elts);
  h->elts = NULL;
  free(h->ptys);
  h->ptys = NULL;
  ht_div_uint32_free(h->ht);
  h->ht = NULL;
}

/** Helper functions */

/**
   Computes a pointer to an element in the element array of a heap.
*/
static void *elt_ptr(heap_t *h, uint32_t i){
  return (void *)((char *)h->elts + i * h->elt_size);
}

/**
   Computes a pointer to a priority in the priority array of a heap.
*/
static void *pty_ptr(heap_t *h, uint32_t i){
  return (void *)((char *)h->ptys + i * h->pty_size);
}

/**
   Swaps elements and priorities at indices i and j.
*/
static void swap(heap_t *h, uint32_t i, uint32_t j){
  if (i == j){return;}
  //swap elements
  char buffer_elt[h->elt_size]; //char used for exact # of bytes
  memcpy(buffer_elt, elt_ptr(h, i), h->elt_size);
  memcpy(elt_ptr(h, i), elt_ptr(h, j), h->elt_size);
  memcpy(elt_ptr(h, j), buffer_elt, h->elt_size);
  //swap priorities
  char buffer_pty[h->pty_size]; 
  memcpy(buffer_pty, pty_ptr(h, i), h->pty_size);
  memcpy(pty_ptr(h, i), pty_ptr(h, j), h->pty_size);
  memcpy(pty_ptr(h, j), buffer_pty, h->pty_size);
  //update ht
  ht_div_uint32_insert(h->ht, elt_ptr(h, i), &i);
  ht_div_uint32_insert(h->ht, elt_ptr(h, j), &j);
}

/**
   Doubles the size of a heap upto the maximal heap size. Amortized constant 
   overhead per push operation, without considering realloc's search of the 
   memory heap.
*/
static void heap_grow(heap_t *h){
  assert(h->heap_size < h->heap_max_size);
  if (h->heap_max_size - h->heap_size < h->heap_size){
    h->heap_size = h->heap_max_size;
  }else{
    h->heap_size *= 2;
  }
  h->elts = realloc(h->elts, h->heap_size * h->elt_size);
  assert(h->elts != NULL);
  h->ptys = realloc(h->ptys, h->heap_size * h->pty_size);
  assert(h->ptys != NULL);
}

/**
   Heapifies the heap structure from the ith element upwards. Only uses
   uint32_t indices and is overflow-safe.
*/
static void heapify_up(heap_t *h, uint32_t i){
  if (i == 0){return;}
  uint32_t ju = (i - 1) / 2; //if i is even, equivalent to (i - 2) / 2
  while(h->cmp_pty_fn(pty_ptr(h, ju), pty_ptr(h, i)) > 0){
    swap(h, i, ju);
    i = ju;
    if (i == 0){break;}
    ju = (i - 1) / 2;
  }
}

/**
   Heapifies the heap structure from the ith element downwards. Only uses
   uint32_t indices and is overflow-safe.
*/
static void heapify_down(heap_t *h, uint32_t i){
  uint32_t jl;
  uint32_t jr;
  //uint32_t safe: 0 <= i <= num_elts - 1 <= 2^32 - 3
  if (h->num_elts == 0 || i + 1 > h->num_elts - 1 - i){
    return; 
  }else if (i + 2 > h->num_elts - 1 - i){
    jl = 2 * i + 1;
    if (h->cmp_pty_fn(pty_ptr(h, i), pty_ptr(h, jl)) > 0){
      swap(h, i, jl);
    }
  }else{
    while (i + 2 <= h->num_elts - 1 - i){
      //both next left and next right indices have elements
      jl = 2 * i + 1;
      jr = 2 * i + 2;
      if (h->cmp_pty_fn(pty_ptr(h, i), pty_ptr(h, jl)) > 0 &&
	  h->cmp_pty_fn(pty_ptr(h, jl), pty_ptr(h, jr)) <= 0){
	swap(h, i, jl);
	i = jl;
      }else if (h->cmp_pty_fn(pty_ptr(h, i), pty_ptr(h, jr)) > 0){
	swap(h, i, jr);
	i = jr;
      }
    }
    if (i + 1 == h->num_elts - 1 - i){
      jl = 2 * i + 1;
      if (h->cmp_pty_fn(pty_ptr(h, i), pty_ptr(h, jl)) > 0){
	swap(h, i, jl);
      }
    }
  }
}
