/**
   bfs-uint64.c

   Functions for running the BFS algorithm on graphs with the number of 
   vertices bounded by 1 + (2^64 - 1) / sizeof(uint64_t) and vertices
   indexed from 0. The unused upper values are reserved for special values. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "bfs-uint64.h"
#include "graph-uint64.h"
#include "queue-uint64.h"
#include "stack-uint64.h"

static uint64_t *vt_ptr(void *vts, uint64_t i);

static const uint64_t nr = 0xffffffffffffffff; //not reached
static const uint64_t l_num_vts = 1 + 0xffffffffffffffff / sizeof(uint64_t);

/**
   Computes and copies to dist the lowest # of edges from start to each 
   reached vertex, and provides the previous vertex in prev, with nr in 
   prev for unreached vertices.
*/
void bfs_uint64(adj_lst_uint64_t *a,
		uint64_t start,
		uint64_t *dist,
		uint64_t *prev){
  assert(a->num_vts > 0 && a->num_vts < l_num_vts);
  queue_uint64_t q;
  uint64_t init_queue_size = 1;
  uint64_t vt_size = sizeof(uint64_t);
  uint64_t u, v;
  bool *placed = calloc(a->num_vts, sizeof(bool));
  assert(placed != NULL);
  for (uint64_t i = 0; i < a->num_vts; i++){
    dist[i] = 0;
    prev[i] = nr;
  }
  queue_uint64_init(&q, init_queue_size, vt_size, NULL);
  prev[start] = start;
  queue_uint64_push(&q, &start);
  placed[start] = true;
  while (q.num_elts > 0){
    queue_uint64_pop(&q, &u);
    for (uint64_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      if (!placed[v]){
	dist[v] = dist[u] + 1;
	prev[v] = u;
	queue_uint64_push(&q, &v);
	placed[v] = true;
      }
    }
  }
  queue_uint64_free(&q);
  free(placed);
  placed = NULL;
}

/** Helper functions */

/**
   Computes a pointer to an entry in an array of vertices.
*/
static uint64_t *vt_ptr(void *vts, uint64_t i){
  return (uint64_t *)((char *)vts + i * sizeof(uint64_t));
}
