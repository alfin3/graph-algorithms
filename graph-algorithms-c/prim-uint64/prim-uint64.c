/**
   prim-uint64.c

   Functions for running Prim's algorithm on an undirected graph with 
   generic weights, including negative weights.

   If there are vertices outside the connected component of start, an mst of 
   the connected component of start is computed.
    
   The number of vertices is bounded by 2^32 - 2, as in heap-uint32. 
   Edge weights are of any basic type (e.g. char, int, double).
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "prim-uint64.h"
#include "graph-uint64.h"
#include "heap-uint32.h"
#include "stack-uint64.h"

static uint64_t *vt_ptr(void *vts, uint64_t i);
static void *wt_ptr(void *wts, uint64_t i, int wt_size);
static int cmp_vt_fn(void *a, void *b){
  return *(uint64_t *)a - *(uint64_t *)b;
}

static const uint64_t nr = 0xffffffffffffffff; //not reached
static const uint64_t l_num_vts = 0xffffffff;

/**
   Computes and copies the edge weights of an mst to dist and previous 
   vertices to prev, with nr in prev for unreached vertices. Assumes 
   immutability of an adjacency list during execution.
*/
void prim_uint64(adj_lst_uint64_t *a,
		 uint64_t start,
		 void *dist,
		 uint64_t *prev,
		 void (*init_wt_fn)(void *),
		 int (*cmp_wt_fn)(void *, void *)){
  assert(a->num_vts < l_num_vts);
  heap_uint32_t h;
  int vt_size = sizeof(uint64_t);
  int wt_size = a->wt_size;
  uint64_t init_heap_size = 1;
  uint64_t u, v;
  void *wt_buf = malloc(wt_size);
  assert(wt_buf != NULL);
  void *v_wt_ptr = NULL;
  void *uv_wt_ptr = NULL;
  bool *in_heap = calloc(a->num_vts, sizeof(bool));
  assert(in_heap != NULL);
  bool *popped = calloc(a->num_vts, sizeof(bool)); 
  assert(popped != NULL);
  for (uint64_t i = 0; i < a->num_vts; i++){
    init_wt_fn(wt_ptr(dist, i, wt_size));
    prev[i] = nr;
  }
  heap_uint32_init(&h,
		   init_heap_size,
		   wt_size,
		   vt_size,
		   cmp_wt_fn,
		   cmp_vt_fn,
		   NULL);
  heap_uint32_push(&h, wt_ptr(dist, start, wt_size), &start);
  in_heap[start] = true;
  prev[start] = start;
  while (h.num_elts > 0){
    heap_uint32_pop(&h, wt_buf, &u); //weight in wt_buf discarded
    in_heap[u] = false;
    popped[u] = true;
    for (uint64_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      v_wt_ptr = wt_ptr(dist, v, wt_size);
      uv_wt_ptr = wt_ptr(a->wts[u]->elts, i, wt_size);
      //not popped and not in heap <=> not reached
      if (!popped[v] && !in_heap[v]){
	memcpy(v_wt_ptr, uv_wt_ptr, wt_size);
	heap_uint32_push(&h, v_wt_ptr, &v);
	in_heap[v] = true;
	prev[v] = u;
      //not popped and in heap => reached
      }else if (!popped[v] && cmp_wt_fn(v_wt_ptr, uv_wt_ptr) > 0){
	memcpy(v_wt_ptr, uv_wt_ptr, wt_size);
	heap_uint32_update(&h, v_wt_ptr, &v);
	prev[v] = u;
      }
    }
  }
  heap_uint32_free(&h);
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
static uint64_t *vt_ptr(void *vts, uint64_t i){
  return (uint64_t *)((char *)vts + i * sizeof(uint64_t));
}

/**
   Computes a pointer to an entry in an array of weights.
*/
static void *wt_ptr(void *wts, uint64_t i, int wt_size){
  return (void *)((char *)wts + i * wt_size);
}
