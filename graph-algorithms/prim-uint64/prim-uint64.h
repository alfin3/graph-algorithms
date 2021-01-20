/**
   prim-uint64.h

   Declarations of accessible functions for running Prim's algorithm on 
   an undirected graph with generic weights, including negative weights.

   If there are vertices outside the connected component of start, an mst of 
   the connected component of start is computed.
    
   The positive number of vertices is bounded by 2^32 - 2, as in heap-uint32.
   Edge weights are of any basic type (e.g. char, int, double).
*/

#ifndef PRIM_UINT64_H  
#define PRIM_UINT64_H

#include <stdint.h>
#include "graph-uint64.h"

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
		 int (*cmp_wt_fn)(const void *, const void *));
#endif
