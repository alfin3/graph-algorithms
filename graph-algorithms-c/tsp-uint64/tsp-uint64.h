/**
   tsp-uint64.h

   Declarations of accessible functions for running a dynamic programming 
   version of an exact solution of TSP with generic weights, including 
   negative weights, in O(2^n n^2) assymptotic runtime, where n is the 
   number of vertices in a tour. 

   A tour without revisiting must exist. In later versions, the non-existence 
   of a tour will be detected.
   
   The number of vertices is > 0 and bounded by 2^32 - 1. Edge weights 
   are of any basic type (e.g. char, int, double).

   Each computation of the next sets from a given set in build_next may be 
   implemented as a thread in a multithreaded version.
*/

#ifndef TSP_UINT64_H  
#define TSP_UINT64_H

#include <stdint.h>
#include "graph-uint64.h"

/**
   Determines the shortest tour length from start to start across all 
   vertices without revisiting.
*/
void tsp_uint64(adj_lst_uint64_t *a,
		uint64_t start,
		void *dist,
		void (*add_wt_fn)(void *, void *, void *),
		int (*cmp_wt_fn)(void *, void *));
#endif
