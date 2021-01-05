/**
   bfs.c

   Functions for running the BFS algorithm on graphs with vertices
   indexed from 0.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "bfs.h"
#include "graph.h"
#include "queue.h"
#include "stack.h"
#include "utilities-mem.h"

static size_t *vt_ptr(const void *vts, size_t i, size_t vt_size);

static const size_t NR = SIZE_MAX; //not reached as index
static const size_t QUEUE_INIT_COUNT = 1;

/**
   Computes and copies to dist the lowest # of edges from start to each 
   reached vertex, and provides the previous vertex in prev, with NR in 
   prev for unreached vertices.
*/
void bfs(const adj_lst_t *a, size_t start, size_t *dist, size_t *prev){
  bool *placed = NULL;
  size_t u, v, vt_size = sizeof(size_t);
  queue_t q;
  if (a->num_vts == 0) return;
  placed = calloc_perror(a->num_vts, sizeof(bool));
  for (size_t i = 0; i < a->num_vts; i++){
    dist[i] = 0;
    prev[i] = NR;
  }
  queue_init(&q, QUEUE_INIT_COUNT, vt_size, NULL);
  prev[start] = start;
  queue_push(&q, &start);
  placed[start] = true;
  while (q.num_elts > 0){
    queue_pop(&q, &u);
    for (size_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i, vt_size);
      if (!placed[v]){
	dist[v] = dist[u] + 1;
	prev[v] = u;
	queue_push(&q, &v);
	placed[v] = true;
      }
    }
  }
  queue_free(&q);
  free(placed);
  placed = NULL;
}

/** Helper functions */

/**
   Computes a pointer to a vertex in an array of vertices.
*/
static size_t *vt_ptr(const void *vts, size_t i, size_t vt_size){
  return (size_t *)((char *)vts + i * vt_size);
}
