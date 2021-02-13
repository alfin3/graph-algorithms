/**
   tsp.h

   Declarations of accessible functions for running a dynamic programming
   version of an exact solution of TSP without revisiting
   and with generic weights, including negative weights, in O(2^n n^2)
   assymptotic runtime, where n is the number of vertices in a tour.

   A bit array representation provides time and space efficient set
   operations.

   TODO
   - hash table parameter
*/

#ifndef TSP_H  
#define TSP_H

#include <stdint.h>
#include "graph.h"

/**
   Copies to the block pointed to by dist the shortest tour length from 
   start to start across all vertices without revisiting, if a tour exists. 
   Returns 0 if a tour exists, otherwise returns 1.
*/
int tsp(const adj_lst_t *a,
	uint64_t start,
	void *dist,
	void (*add_wt)(void *, const void *, const void *),
	int (*cmp_wt)(const void *, const void *));
#endif
