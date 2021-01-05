/**
   bfs.c

   Functions for running the BFS algorithm on graphs with vertices
   indexed from 0.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bfs.h"
#include "graph.h"
#include "queue.h"
#include "stack.h"
#include "utilities-mem.h"

static const size_t NR = SIZE_MAX; //not reached as index
static const size_t QUEUE_INIT_COUNT = 1;

/**
   Computes and copies to dist the lowest # of edges from start to each 
   reached vertex, and provides the previous vertex in prev, with SIZE_MAX
   in prev for unreached vertices. Assumes start is valid and there is
   at least one vertex.
*/
void bfs(const adj_lst_t *a, size_t start, size_t *dist, size_t *prev){
  size_t u, *vts = NULL;
  size_t vt_size = sizeof(size_t);
  queue_t q;
  memset(dist, 0, a->num_vts * sizeof(size_t));
  memset(prev, 0xff, a->num_vts * sizeof(size_t)); //initialize to NR
  queue_init(&q, QUEUE_INIT_COUNT, vt_size, NULL);
  prev[start] = start;
  queue_push(&q, &start);
  while (q.num_elts > 0){
    queue_pop(&q, &u);
    vts = a->vts[u]->elts;
    for (size_t i = 0; i < a->vts[u]->num_elts; i++){
      if (prev[vts[i]] == NR){
	dist[vts[i]] = dist[u] + 1;
	prev[vts[i]] = u;
	queue_push(&q, &vts[i]);
      }
    }
  }
  queue_free(&q);
}
