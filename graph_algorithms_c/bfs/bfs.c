/**
   bfs.c

   Functions for running the BFS algorithm. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "bfs.h"
#include "graph.h"
#include "queue.h"
#include "stack.h"

static int *vt_ptr(void *vts, int i);

/**
   Computes and copies to dist the lowest # of edges from s to each reached 
   vertex, and provides the previous vertex in prev, with -1 in prev 
   for unreached vertices.
*/
void bfs(adj_lst_t *a, int s, int *dist, int *prev){
  assert(a->num_vts > 0); //a->num_vts includes s
  queue_t q;
  int q_size = 1;
  int vt_size = sizeof(int);
  int u;
  int v;
  bool *placed = calloc(a->num_vts, sizeof(bool));
  assert(placed != NULL);
  for (int i = 0; i < a->num_vts; i++){
    dist[i] = 0;
    prev[i] = -1;
  }
  queue_init(&q, q_size, vt_size, NULL);
  prev[s] = s;
  queue_push(&q, &s);
  placed[s] = true;
  while (q.num_elts > 0){
    queue_pop(&q, &u);
    for (int i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      //reached for the first time => shortest distance from s
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
   Computes a pointer to an entry in an array of vertices.
*/
static int *vt_ptr(void *vts, int i){
  return (int *)((char *)vts + i * sizeof(int));
}
