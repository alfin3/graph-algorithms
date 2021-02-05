/**
   dijkstra.h

   Declarations of accessible functions for running Dijkstra's algorithm on 
   a graph with generic non-negative weights and a hash table parameter.
*/

#ifndef DIJKSTRA_H  
#define DIJKSTRA_H

#include <stdint.h>
#include "graph.h"
#include "heap.h"

/**
   Computes and copies the shortest distances from start to dist array and 
   previous vertices to prev array, with NR in prev for unreached vertices.
*/
void dijkstra(const adj_lst_t *a,
	      size_t start,
	      void *dist,
	      size_t *prev,
	      const heap_ht_t *ht,
	      void (*add_wt)(void *, const void *, const void *),
	      int (*cmp_wt)(const void *, const void *));
#endif
