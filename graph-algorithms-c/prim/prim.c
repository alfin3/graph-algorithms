/**
   prim.c

   Functions for running Prim's algorithm on an undirected graph with 
   generic weights, including negative weights.

   If there are vertices outside the connected component of s, an mst of 
   the connected component of s is returned.

   Edge weights are of any basic type (e.g. char, int, double, long double).
   Edge weight initialization and operations are defined in init_wt_fn, 
   add_wt_fn, and cmp_wt_fn functions. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "stack.h"
#include "heap.h"
#include "graph.h"
#include "prim.h"

static int *vt_ptr(adj_lst_t *a, void *vts, int i);
static void *wt_ptr(adj_lst_t *a, void *wts, int i);
static int cmp_vt_fn(void *a, void *b){return *(int *)a - *(int *)b;}
static void free_vt_fn(void *a){}

/**
   Computes and copies weights of an mst to dist array and previous 
   vertices to prev array, with -1 in prev array for unreached vertices.
*/
void prim(adj_lst_t *a,
	  int s,
	  void *dist,
	  int *prev,
	  void (*init_wt_fn)(void *),
	  int (*cmp_wt_fn)(void *, void *)){
  heap_t h;
  int h_size = 1;
  int vt_size = sizeof(int);
  int wt_size = a->wt_size;
  int u, v;
  void *wt_buf = malloc(wt_size); //single edge weight buffer
  assert(wt_buf != NULL);
  void *uv_wt_ptr; //edge weight pointer
  bool *in_heap = calloc(a->num_vts, sizeof(bool));
  assert(in_heap != NULL);
  bool *popped = calloc(a->num_vts, sizeof(bool)); 
  assert(popped != NULL);
  for (int i = 0; i < a->num_vts; i++){
    init_wt_fn(wt_ptr(a, dist, i));
    prev[i] = -1;
  }
  heap_init(&h, h_size, vt_size, wt_size, cmp_vt_fn, cmp_wt_fn, free_vt_fn);
  heap_push(&h, &s, wt_ptr(a, dist, s));
  in_heap[s] = true;
  prev[s] = s;
  while (h.num_elts > 0){
    heap_pop(&h, &u, wt_buf); //popped edge weight in wt_buf discarded
    in_heap[u] = false;
    popped[u] = true;
    for (int i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a, a->vts[u]->elts, i);
      uv_wt_ptr = wt_ptr(a, a->wts[u]->elts, i);
      //not popped and not in heap <=> not reached <=> infinity
      if (!popped[v] && !in_heap[v]){
	memcpy(wt_ptr(a, dist, v), uv_wt_ptr, wt_size);
	heap_push(&h, &v, wt_ptr(a, dist, v));
	in_heap[v] = true;
	prev[v] = u;
      //not popped and in heap => reached => not infinity
      }else if (!popped[v] && cmp_wt_fn(wt_ptr(a, dist, v), uv_wt_ptr) > 0){
	memcpy(wt_ptr(a, dist, v), uv_wt_ptr, wt_size);
	heap_update(&h, &v, wt_ptr(a, dist, v));
	prev[v] = u;
      }
    }
  }
  heap_free(&h);
  free(wt_buf);
  free(in_heap);
  free(popped);
  wt_buf = NULL;
  in_heap = NULL;
  popped = NULL;
}

/** Helper functions */

/**
   Computes a pointer to an entry in an array of vertices.
*/
static int *vt_ptr(adj_lst_t *a, void *vts, int i){
  return (int *)((char *)vts + i * sizeof(int));
}

/**
   Computes a pointer to an entry in an array of weights.
*/
static void *wt_ptr(adj_lst_t *a, void *wts, int i){
  return (void *)((char *)wts + i * a->wt_size);
}
