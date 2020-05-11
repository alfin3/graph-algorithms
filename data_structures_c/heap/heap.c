/**
   heap.c

   Implementation of generic dynamicaly allocated (min) heap.

   Through user-defined comparison and deallocation functions, the 
   implementation enables a dynamic set in heap form of any objects 
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
static void swap(heap_t *h, int i, int j);
static void heap_grow(heap_t *h);
static void heapify_up(heap_t *h, int i);
static void heapify_down(heap_t *h, int i);

/**
   Initializes a heap.                  
*/
void heap_init(heap_t *h,
	       int init_heap_size,
	       int elt_size,
	       int pty_size,
	       int (*cmp_elt_fn)(void *, void *),
	       int (*cmp_pty_fn)(void *, void *),
	       void (*free_elt_fn)(void *)){
  h->heap_size = init_heap_size;
  h->num_elts = 0;
  h->elt_size = elt_size;
  h->pty_size = pty_size;
  h->elts = (void **)malloc(init_heap_size * sizeof(char *));
  assert(h->elts != NULL);
  h->ptys = malloc(init_heap_size * pty_size);
  assert(h->ptys != NULL);
  h->cmp_elt_fn = cmp_elt_fn;
  h->cmp_pty_fn = cmp_pty_fn;
  h->free_elt_fn = free_elt_fn;
}

/**
   Pushes an element onto a heap.
*/
void heap_push(heap_t *h, void *elt, void *pty){
  if (h->heap_size == h->num_elts){heap_grow(h);}
  h->elts[h->num_elts] = malloc(h->elt_size);
  assert(h->elts[h->num_elts] != NULL);
  void *elt_target = h->elts[h->num_elts];
  void *pty_target = pty_ptr(h, h->num_elts);
  memcpy(elt_target, elt, h->elt_size);
  memcpy(pty_target, pty, h->pty_size);
  h->num_elts++;
  heapify_up(h, h->num_elts - 1);
}

/**
   Pops an element and the minimal priority value according to cmp_elts_fn.
*/
void heap_pop(heap_t *h, void *elt, void *pty){
  void *elt_source = h->elts[0];
  void *pty_source = pty_ptr(h, 0);
  memcpy(elt, elt_source, h->elt_size);
  memcpy(pty, pty_source, h->pty_size);
  swap(h, 0, h->num_elts - 1);
  h->num_elts--;
  heapify_down(h, 0);
}

/**
   If element is present on a heap, updates its priority and returns 1,
   otherwise returns 0.
*/
int heap_update(heap_t *h, void *elt, void *pty){
  // at this time, implementation w/o hash table => O(n) instead of O(logn)
  for (int i = 0; i < h->num_elts; i++){
    if (h->cmp_elt_fn(h->elts[i], elt) == 0){
      void *pty_target = pty_ptr(h, i);
      int ju = (i - 1) / 2;
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
   Frees dynamically allocated elements and priority values. Memory allocated
   to elements pointed to from elts is freed according to free_elt_fn.
*/
void heap_free(heap_t *h){
  for (int i = 0; i < h->num_elts; i++){
    if (h->free_elt_fn != NULL){
       h->free_elt_fn(h->elts[i]);
    } else {
      free(h->elts[i]);
    }
  } 
  free(h->elts);
  free(h->ptys);
}

/** Helper functions */

/**
   Computes a pointer to a priority value in a priority array.
*/
static void *pty_ptr(heap_t *h, int i){
  return (void *)((char *)h->ptys + i * h->pty_size);
}

/**
   Swaps priority values and element pointers at indeces i and j.
*/
static void swap(heap_t *h, int i, int j){
  // swap element pointers
  void *temp_elt = h->elts[i];
  h->elts[i] = h->elts[j];
  h->elts[j] = temp_elt;
  // swap priority values
  char buffer[h->pty_size]; //char used for exact # of bytes
  memcpy(buffer, pty_ptr(h, i), h->pty_size);
  memcpy(pty_ptr(h, i), pty_ptr(h, j), h->pty_size);
  memcpy(pty_ptr(h, j), buffer, h->pty_size);
}

/**
   Doubles the size of heap. Amortized constant overhead in worst case 
   of realloc calls when a memory block cannot be extended.
*/
static void heap_grow(heap_t *h){
  h->elts = (void **)realloc(h->elts, 2 * h->heap_size * sizeof(char *));
  assert(h->elts != NULL);
  h->ptys = realloc(h->ptys, 2 * h->heap_size * h->pty_size);
  assert(h->ptys != NULL);
  h->heap_size = h->heap_size * 2;
}

/**
   Heapifies heap structure from ith element in array representation upwards.
*/
static void heapify_up(heap_t *h, int i){
  int ju = (i - 1) / 2; // if i is even, equivalent to (i - 2) / 2
  while(ju >= 0 &&
	h->cmp_pty_fn(pty_ptr(h, ju), pty_ptr(h, i)) > 0){
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
      } else {
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
