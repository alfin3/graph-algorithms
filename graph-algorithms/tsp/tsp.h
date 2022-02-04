/**
   tsp.h

   Declarations of accessible functions for running an exact solution of TSP
   without vertex revisiting on graphs with generic weights, including
   negative weights, with a hash table parameter.

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
   If NULL is passed as a hash table parameter value, a default hash table
   is used, which contains an array with a count that is equal to n * 2^n,
   where n is the number of vertices in the graph.   

   If E >> V and V < sizeof(size_t) * CHAR_BIT, a default hash table may
   provide speed advantages by avoiding the computation of hash values. If V
   is larger and the graph is sparse, a non-default hash table may provide
   space advantages.
*/

#ifndef TSP_H  
#define TSP_H

#include <stddef.h>
#include "graph.h"

/**
   TSP hash table parameter struct. The function pointers point to the
   hash table op helpers, pre-defined in each hash table. ht points
   to a preallocated hash table struct.
*/
struct tsp_ht{
  void *ht;
  size_t alpha_n;
  size_t log_alpha_d;
  void (*init)(void *,
	       size_t,
	       size_t,
	       size_t,
	       size_t,
	       size_t,
	       int (*)(const void *, const void *),
	       size_t (*)(const void *),
	       void (*)(void *),
	       void (*)(void *));
  void (*align)(void *, size_t);
  void (*insert)(void *, const void *, const void *);
  void *(*search)(const void *, const void *);
  void (*remove)(void *, const void *, void *);
  void (*free)(void *);
};

/**
   Copies to the block pointed to by dist the shortest tour length from 
   start to start across all vertices without revisiting, if a tour exists. 
   Returns 0 if a tour exists, otherwise returns 1.
   a           : pointer to an adjacency list with at least one vertex
   start       : start vertex for running the algorithm
   dist        : pointer to a preallocated block of the size of a weight in
                 the adjacency list
   tht         : - NULL pointer, if a default hash table is used for
                 set hashing operations; a default hash table contains an
                 array with a count that is equal to n * 2^n, where n is the
                 number of vertices in the adjacency list; the maximal n
                 in a default hash table is system-dependent and is less
                 than sizeof(size_t) * CHAR_BIT; if the allocation of a
                 default hash table fails, the program terminates with an
                 error message
                 - a pointer to a set of parameters specifying a hash table
                 used for set hashing operations; the size of a hash key is 
                 k * (1 + lowest # k-sized blocks s.t. # bits >= # vertices),
                 where k = sizeof(size_t)
   add_wt      : addition function which copies the sum of the weight values
                 pointed to by the second and third arguments to the
                 preallocated weight block pointed to by the first argument
   cmp_wt      : comparison function which returns a negative integer value
                 if the weight value pointed to by the first argument is
                 less than the weight value pointed to by the second, a
                 positive integer value if the weight value pointed to by
                 the first argument is greater than the weight value 
                 pointed to by the second, and zero integer value if the two
                 weight values are equal
*/
int tsp(const adj_lst_t *a,
	size_t start,
	void *dist,
	const tsp_ht_t *tht,
	void (*add_wt)(void *, const void *, const void *),
	int (*cmp_wt)(const void *, const void *));
#endif
