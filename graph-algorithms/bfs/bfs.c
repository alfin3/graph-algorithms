/**
   bfs.c

   Functions for running the BFS algorithm on graphs with vertices
   indexed from 0.

   Optimization notes: 
   - using a bit array for testing if a vertex was visited, instead of
   prev[vts[i]] == NR, in order to decrease cache misses did not result
   in a speed up in tests even when the index computation was performed
   with a right bit shift on 64-bit words.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfs.h"
#include "graph.h"
#include "queue.h"
#include "stack.h"
#include "utilities-mem.h"

static const size_t NR = (size_t)-1; /* not reached as index */
static const size_t QUEUE_INIT_COUNT = 1;

/**
   Computes and copies to an array pointed to by dist the lowest # of edges
   from start to each reached vertex, and provides the previous vertex in the
   array pointed to by prev, with the maximal value of size_t in the prev
   array for unreached vertices. Assumes start is valid and there is at least
   one vertex.
   a           : pointer to an adjacency list with at least one vertex
   start       : a start vertex for running bfs
   dist        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list
   prev        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list
*/
void bfs(const adj_lst_t *a, size_t start, size_t *dist, size_t *prev){
  size_t u, *vts = NULL;
  size_t vt_size = sizeof(size_t);
  size_t i;
  queue_t q;
  memset(dist, 0, a->num_vts * vt_size);
  memset(prev, 0xff, a->num_vts * vt_size); /* initialize to NR */
  queue_init(&q, QUEUE_INIT_COUNT, vt_size, NULL);
  prev[start] = start;
  queue_push(&q, &start);
  while (q.num_elts > 0){
    queue_pop(&q, &u);
    vts = a->vts[u]->elts;
    for (i = 0; i < a->vts[u]->num_elts; i++){
      if (prev[vts[i]] == NR){
	dist[vts[i]] = dist[u] + 1;
	prev[vts[i]] = u;
	queue_push(&q, &vts[i]);
      }
    }
  }
  queue_free(&q);
}
