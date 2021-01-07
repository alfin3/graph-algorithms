/**
   dijkstra-uint64.h

   Declarations of accessible functions for running Dijkstra's algorithm on 
   a graph with generic non-negative weights.

   The number of vertices is > 0 and bounded by 2^32 - 2, as in heap-uint32.
   Edge weights are of any basic type (e.g. char, int, double).
*/

#ifndef DIJKSTRA_UINT64_H  
#define DIJKSTRA_UINT64_H

#include <stdint.h>
#include "graph.h"

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
		     int (*cmp_wt_fn)(void *, void *));
#endif
