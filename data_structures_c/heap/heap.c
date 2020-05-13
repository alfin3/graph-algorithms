/**
   heap.c

   Implementation of generic dynamicaly allocated (min) heap.

   Through user-defined comparison and deallocation functions, the 
   implementation provides a dynamic set in heap form of any objects 
   associated with priority values of basic type of choice (e.g. char, int, 
   long, double). 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "heap.h"

/** Main functions */

static void *pty_ptr(heap_t *h, int i);
static void *elt_ptr(heap_t *h, int i);
static void swap(heap_t *h, int i, int j);
static void heap_grow(heap_t *h);
static void heapify_up(heap_t *h, int i);
static void heapify_down(heap_t *h, int i);

/**
   Initializes a heap. 
   init_heap_size: integer > 0.
   cmp_elt_fn: returns 0 if elements match, non-zero otherwise.
   cmp_pty_fn: returns  0 if priorities are equal,
                       > 0 if first priority is greater,
                       < 0 if first priority is lower.
   free_elt_fn: non-NULL.
*/
void heap_init(heap_t *h,
	       int init_heap_size,
	       int elt_size,
	       int pty_size,
	       int (*cmp_elt_fn)(void *, void *),
	       int (*cmp_pty_fn)(void *, void *),
	       void (*free_elt_fn)(void *)){
  h->heap_size = init_heap_size;
  assert(h->heap_size > 0);
  h->num_elts = 0;
  h->elt_size = elt_size;
  h->pty_size = pty_size;
  h->elts = malloc(init_heap_size * elt_size);
  assert(h->elts != NULL);
  h->ptys = malloc(init_heap_size * pty_size);
  assert(h->ptys != NULL);
  h->cmp_elt_fn = cmp_elt_fn;
  h->cmp_pty_fn = cmp_pty_fn;
  h->free_elt_fn = free_elt_fn;
  assert(h->free_elt_fn != NULL);
}

/**
   Pushes an element and priority value onto a heap.
   elt: pointer to element pointer, if element is a multilayer object,
        pointer to element, if element is fully stored in elts array,
        elt_size reflects these cases.
   pty: pointer to object of basic type (e.g. char, int, long, double),
        pty_size reflects a type.
*/
void heap_push(heap_t *h, void *elt, void *pty){
  if (h->heap_size == h->num_elts){heap_grow(h);}
  int ix = h->num_elts;
  void *elt_target = elt_ptr(h, ix);
  void *pty_target = pty_ptr(h, ix);
  memcpy(elt_target, elt, h->elt_size);
  memcpy(pty_target, pty, h->pty_size);
  h->num_elts++;
  heapify_up(h, ix);
}

/**
   Pops an element and a minimal priority according to cmp_pty_fn.
*/
void heap_pop(heap_t *h, void *elt, void *pty){
  int ix = 0;
  void *elt_source = elt_ptr(h, ix);
  void *pty_source = pty_ptr(h, ix);
  memcpy(elt, elt_source, h->elt_size);
  memcpy(pty, pty_source, h->pty_size);
  swap(h, ix, h->num_elts - 1);
  h->num_elts--; // decrement prior to heapify_down
  heapify_down(h, ix);
}

/**
   If element is present on heap according to cmp_elt_fn, updates its 
   priority and returns 1, otherwise returns 0.
*/
int heap_update(heap_t *h, void *elt, void *pty){
  // at this time, w/o hash table => O(n) instead of O(logn)
  for (int i = 0; i < h->num_elts; i++){
    if (h->cmp_elt_fn(elt_ptr(h, i), elt) == 0){
      int ju = (i - 1) / 2;
      void *pty_target = pty_ptr(h, i);
      memcpy(pty_target, pty, h->pty_size);
      if (ju >= 0 && h->cmp_pty_fn(pty_ptr(h, ju), pty_ptr(h, i)) > 0){
	heapify_up(h, i);
      } else {
	heapify_down(h, i);
      }
      return 1;
    }
  }
  return 0;
}

/**
   Frees elements, according to free_elt_fn, and element and priority arrays.
*/
void heap_free(heap_t *h){
  for (int i = 0; i < h->num_elts; i++){
    h->free_elt_fn(elt_ptr(h, i));
  } 
  free(h->elts);
  h->elts = NULL;
  free(h->ptys);
  h->ptys = NULL;
}

/** Helper functions */

/**
   Computes a pointer to an element in an element array.
*/
static void *elt_ptr(heap_t *h, int i){
  return (void *)((char *)h->elts + i * h->elt_size);
}

/**
   Computes a pointer to a priority in a priority array.
*/
static void *pty_ptr(heap_t *h, int i){
  return (void *)((char *)h->ptys + i * h->pty_size);
}

/**
   Swaps elements and priorities at indeces i and j.
*/
static void swap(heap_t *h, int i, int j){
  // swap elements
  char buffer_elt[h->elt_size]; //char used for exact # of bytes
  memcpy(buffer_elt, elt_ptr(h, i), h->elt_size);
  memcpy(elt_ptr(h, i), elt_ptr(h, j), h->elt_size);
  memcpy(elt_ptr(h, j), buffer_elt, h->elt_size);
  // swap priorities
  char buffer_pty[h->pty_size]; 
  memcpy(buffer_pty, pty_ptr(h, i), h->pty_size);
  memcpy(pty_ptr(h, i), pty_ptr(h, j), h->pty_size);
  memcpy(pty_ptr(h, j), buffer_pty, h->pty_size);
}

/**
   Doubles the size of heap. Amortized constant overhead in worst case of 
   realloc calls across heap_grow calls.
*/
static void heap_grow(heap_t *h){
  h->heap_size *= 2;
  h->elts = realloc(h->elts, h->heap_size * h->elt_size);
  assert(h->elts != NULL);
  h->ptys = realloc(h->ptys, h->heap_size * h->pty_size);
  assert(h->ptys != NULL);
}

/**
   Heapifies heap structure from ith element in array representation upwards.
*/
static void heapify_up(heap_t *h, int i){
  int ju = (i - 1) / 2; // if i is even, equivalent to (i - 2) / 2
  while(ju >= 0 && h->cmp_pty_fn(pty_ptr(h, ju), pty_ptr(h, i)) > 0){
      swap(h, i, ju);
      i = ju;
      ju = (i - 1) / 2;
  }
}

/**
   Heapifies heap structure from ith element in array representation downwards.
*/
static void heapify_down(heap_t *h, int i){
  int jl = 2 * i + 1;
  int jr = 2 * i + 2;
  while (jr < h->num_elts &&
	 (h->cmp_pty_fn(pty_ptr(h, i), pty_ptr(h, jl)) > 0 ||
	  h->cmp_pty_fn(pty_ptr(h, i), pty_ptr(h, jr)) > 0)){
    if (h->cmp_pty_fn(pty_ptr(h, jl), pty_ptr(h, jr)) < 0){
      swap(h, i, jl);
      i = jl;
    }else{
      swap(h, i, jr);
      i = jr;
    }
    jl = 2 * i + 1;
    jr = 2 * i + 2;
  }
  if (jl == h->num_elts - 1 &&
      h->cmp_pty_fn(pty_ptr(h, i), pty_ptr(h, jl)) > 0){
    swap(h, i, jl);
  }
}
