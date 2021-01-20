/**
   dijkstra-uint64.c

   Functions for running Dijkstra's algorithm on a graph with generic 
   non-negative weights.
    
   The number of vertices is > 0 and bounded by 2^32 - 2, as in heap-uint32.
   Edge weights are of any basic type (e.g. char, int, double).
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "dijkstra-uint64.h"
#include "graph.h"
#include "heap-uint32.h"
#include "stack.h"
#include "utilities-mem.h"

static uint64_t *vt_ptr(void *vts, uint64_t i);
static void *wt_ptr(void *wts, uint64_t i, int wt_size);
static int cmp_vt_fn(const void *a, const void *b);

static const uint64_t nr = 0xffffffffffffffff; //not reached
static const uint64_t l_num_vts = 0xffffffff;

/**
   Computes and copies the shortest distances from start to dist array and 
   previous vertices to prev array, with nr in prev for unreached vertices.
   Assumes immutability of an adjacency list during execution.
*/
void dijkstra_uint64(adj_lst_t *a,
		     uint64_t start,
		     void *dist,
		     uint64_t *prev,
		     void (*init_wt_fn)(void *),
		     void (*add_wt_fn)(void *, void *, void *),
		     int (*cmp_wt_fn)(const void *, const void *)){
  assert(a->num_vts > 0 && a->num_vts < l_num_vts);
  heap_uint32_t h;
  int vt_size = sizeof(uint64_t);
  int wt_size = a->wt_size;
  uint64_t init_heap_size = 1;
  uint64_t u, v;
  void *wt_buf = malloc_perror(wt_size);
  void *v_wt_ptr = NULL;
  bool *in_heap = calloc_perror(a->num_vts, sizeof(bool));
  for (uint64_t i = 0; i < a->num_vts; i++){
    init_wt_fn(wt_ptr(dist, i, wt_size));
    prev[i] = nr; //use as infinity
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
    for (uint64_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      v_wt_ptr = wt_ptr(dist, v, wt_size);
      add_wt_fn(wt_buf,
		wt_ptr(dist, u, wt_size),
		wt_ptr(a->wts[u]->elts, i, wt_size));
      if (prev[v] == nr || cmp_wt_fn(v_wt_ptr, wt_buf) > 0){
	memcpy(v_wt_ptr, wt_buf, wt_size);
	prev[v] = u;
	if (in_heap[v]){
	  heap_uint32_update(&h, v_wt_ptr, &v);
	}else{
	  heap_uint32_push(&h, v_wt_ptr, &v);
	  in_heap[v] = true;
	}
      }
    }
  }
  heap_uint32_free(&h);
  free(wt_buf);
  free(in_heap);
  wt_buf = NULL;
  in_heap = NULL;
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

/**
   Compares two vertices.
*/
static int cmp_vt_fn(const void *a, const void *b){
  if (*(uint64_t *)a > *(uint64_t *)b){
    return 1;
  }else if (*(uint64_t *)a < *(uint64_t *)b){
    return -1;
  }else{
    return 0;
  }
}

