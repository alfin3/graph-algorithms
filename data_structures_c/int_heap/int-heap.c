/**
   int-heap.c

   Non-generic dynamicaly allocated (min) heap.
   Each entry consists of an integer element and integer priority value.

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "int-heap.h"

/** Main functions */

static void int_heap_grow(int_heap_t *h);
static void heapify_up(int_heap_t *h, int i);
static void heapify_down(int_heap_t *h, int i);

/**
   Initializes a min heap.
*/
void int_heap_init(int_heap_t *h, int heap_size){
  h->heap_size = heap_size;
  assert(h->heap_size > 0);
  h->num_elts = 0;
  h->elts = (int *)malloc(heap_size * sizeof(int));
  assert(h->elts != NULL);
  h->ptys = (int *)malloc(heap_size * sizeof(int));
  assert(h->ptys != NULL);
}

/**
   Pushes integer element associated with integer priority onto a min heap.
*/
void int_heap_push(int_heap_t *h, int *elt, int *pty){
  if (h->heap_size == h->num_elts){int_heap_grow(h);}
  h->elts[h->num_elts] = *elt;
  h->ptys[h->num_elts] = *pty;
  h->num_elts++;
  heapify_up(h, h->num_elts - 1);
}

/**
   Pops an element associated with the minimal priority.
*/
void int_heap_pop(int_heap_t *h, int *elt, int *pty){
  *elt = h->elts[0];
  *pty = h->ptys[0];
  h->elts[0] = h->elts[h->num_elts - 1];
  h->ptys[0] = h->ptys[h->num_elts - 1];
  h->num_elts--;
  heapify_down(h, 0);
}

/**
   If element is present on the heap, updates its priority and returns 1.
   Returns 0 otherwise.
*/
int int_heap_update(int_heap_t *h, int *elt, int *pty){
  // at this time, implementation w/o hash table => O(n) instead of O(logn)
  for (int i = 0; i < h->num_elts; i++){
    if (h->elts[i] == *elt){
      h->ptys[i] = *pty;
      int ju = (i - 1) / 2;
      if (ju >= 0 && h->ptys[ju] > h->ptys[i]){
	heapify_up(h, i);
      }else{
	heapify_down(h, i);
      }
      return 1;
    }
  }
  return 0;
}

/**
   Frees dynamically allocated arrays in a heap.
*/
void int_heap_free(int_heap_t *h){
  free(h->elts);
  h->elts = NULL;
  free(h->ptys);
  h->ptys = NULL;
}

/** Helper functions */

/**
   Doubles the size of heap.
*/
static void int_heap_grow(int_heap_t *h){
  h->heap_size *= 2;
  h->elts = (int *)realloc(h->elts, h->heap_size * sizeof(int));
  assert(h->elts != NULL);
  h->ptys = (int *)realloc(h->ptys, h->heap_size * sizeof(int));
  assert(h->ptys != NULL);
}

/**
   Swaps elements and priorities at indeces i and j.
*/
static void swap(int_heap_t *h, int i, int j){
  int temp_elt = h->elts[i];
  int temp_pty = h->ptys[i];
  h->elts[i] = h->elts[j];
  h->ptys[i] = h->ptys[j];
  h->elts[j] = temp_elt;
  h->ptys[j] = temp_pty;
}

/**
   Heapifies heap structure from ith element in array representation upwards.
*/
static void heapify_up(int_heap_t *h, int i){
  int ju = (i - 1) / 2; // if i is even, equivalent to (i - 2) / 2
  while(ju >= 0 && h->ptys[ju] > h->ptys[i]){
      swap(h, i, ju);
      i = ju;
      ju = (i - 1) / 2;
  }
}

/**
   Heapifies heap structure from ith element in array representation downwards.
*/
static void heapify_down(int_heap_t *h, int i){
  int jl = 2 * i + 1;
  int jr = 2 * i + 2;
  while (jr < h->num_elts && (h->ptys[i] > h->ptys[jl] ||
			      h->ptys[i] > h->ptys[jr])){
      if (h->ptys[jl] < h->ptys[jr]){
	swap(h, i, jl);
	i = jl;
      } else {
	swap(h, i, jr);
	i = jr;
      }
      jl = 2 * i + 1;
      jr = 2 * i + 2;
  }
  if (jl == h->num_elts - 1 && h->ptys[i] > h->ptys[jl]){
    swap(h, i, jl);
  }
}
