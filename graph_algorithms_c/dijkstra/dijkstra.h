/**
   dijkstra.h

   Declarations of accessible functions for running Dijkstra's algorithm on 
   a graph with generic non-negative weights.

   Edge weights are of any basic type (e.g. char, int, double, long double).
   Edge weight initialization and operations are defined in init_wt_fn, 
   add_wt_fn, and cmp_wt_fn functions. 
*/

#ifndef DIJKSTRA_H  
#define DIJKSTRA_H

#include "graph.h"

/**
   Computes and copies the shortest distances from s to dist array and 
   previous vertex to prev array, with -1 in prev for unreached vertices.
*/
void dijkstra(adj_lst_t *a,
	      int s,
	      void *dist,
	      int *prev,
	      void (*init_wt_fn)(void *),
	      void (*add_wt_fn)(void *, void *, void *),
	      int (*cmp_wt_fn)(void *, void *));

#endif
