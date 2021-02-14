/**
   tsp.h

   Declarations of accessible functions for running a dynamic programming
   version of an exact solution of TSP without vertex revisiting, with
   generic weights including negative weights, and with a hash table
   parameter.

   Vertices are indexed from 0. Edge weights are of any basic type (e.g.
   char, int, long, float, double), or are custom weights within a
   contiguous block (e.g. pair of 64-bit segments to address the potential
   overflow due to addition).
   
   The algorithm provides O(2^n n^2) assymptotic runtime, where n is the
   number of vertices in a tour, as well as tour existence detection. A bit
   array representation provides time and space efficient set membership and
   union operations over O(2^n) sets.

   The hash table parameter specifies a hash table used for set hashing
   operations, and enables the optimization of the associated space and time
   resources by choice of a hash table and its load factor upper bound.

   TODO
   - default hash table for dense graphs with a small # vertices
*/

#ifndef TSP_H  
#define TSP_H

#include <stdint.h>
#include "graph.h"

typedef void (*tsp_ht_init)(void *,
			    size_t,
			    size_t,
			    void (*)(void *), //free_elt
			    void *); //pointer to context
typedef void (*tsp_ht_insert)(void *, const void *, const void *);
typedef void *(*tsp_ht_search)(const void *, const void *);
typedef void (*tsp_ht_remove)(void *, const void *, void *);
typedef void (*tsp_ht_free)(void *);

typedef struct{
  void *ht; //points to a block of hash table struct size
  void *context; //points to initialization context
  tsp_ht_init init;
  tsp_ht_insert insert;
  tsp_ht_search search;
  tsp_ht_remove remove;
  tsp_ht_free free;
} tsp_ht_t;

/**
   Copies to the block pointed to by dist the shortest tour length from 
   start to start across all vertices without revisiting, if a tour exists. 
   Returns 0 if a tour exists, otherwise returns 1.

   tht:  size of hash key is sizeof(size_t) * 
         (1 + smallest # sizeof(size_t) size blocks s.t. 
         # bits >= number of vertices)
*/
int tsp(const adj_lst_t *a,
	uint64_t start,
	void *dist,
	tsp_ht_t *tht,
	void (*add_wt)(void *, const void *, const void *),
	int (*cmp_wt)(const void *, const void *));
#endif
