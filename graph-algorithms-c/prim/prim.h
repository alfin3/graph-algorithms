/**
   prim.h

   Declarations of accessible functions running Prim's algorithm on an 
   undirected graph with generic weights, including negative weights.

   If there are vertices outside the connected component of s, an mst of 
   the connected component of s is returned.

   Edge weights are of any basic type (e.g. char, int, double, long double).
   Edge weight initialization and operations are defined in init_wt_fn, 
   add_wt_fn, and cmp_wt_fn functions. 
*/

#ifndef PRIM_H  
#define PRIM_H

#include "graph.h"

/**
   Computes and copies weights of an mst to dist array and previous 
   vertices to prev array, with -1 in prev array for unreached vertices.
*/
void prim(adj_lst_t *a,
	  int s,
	  void *dist,
	  int *prev,
	  void (*init_wt_fn)(void *),
	  int (*cmp_wt_fn)(void *, void *));
#endif
