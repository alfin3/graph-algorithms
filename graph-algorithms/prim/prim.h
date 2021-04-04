/**
   prim.h

   Declarations of accessible functions for running Prim's algorithm on
   undirected graphs with generic weights and a hash table parameter.

   If there are vertices outside the connected component of start, an mst of 
   the connected component of start is computed.

   Vertices are indexed from 0. Edge weights may include negative weights,
   and are of any basic type (e.g. char, int, long, float, double), or are
   custom weights within a contiguous block.

   The hash table parameter specifies a hash table used for in-heap
   operations, and enables the optimization of space and time resources
   associated with heap operations in Prim's algorithm by choice of a
   hash table and its load factor upper bound. If NULL is passed as a hash
   table parameter value, a default hash table is used, which contains an
   index array with a count that is equal to the number of vertices in the
   graph.   

   If E >> V, a default hash table may provide speed advantages by avoiding
   the computation of hash values. If V is large and the graph is sparse,
   a non-default hash table may provide space advantages.

   The implementation does not use stdint.h and is portable under C89/C90.
*/

#ifndef PRIM_H  
#define PRIM_H

#include <stddef.h>
#include "graph.h"
#include "heap.h"

/**
   Computes and copies the edge weights of an mst of the connected component
   of a start vertex to the array pointed to by dist, and the previous
   vertices to the array pointed to by prev, with the maximal value of size_t
   in the prev array for unreached vertices.
   a           : pointer to an adjacency list with at least one vertex
   start       : start vertex
   dist        : pointer to a preallocated array where the count is equal
                 to the number of vertices, and the size of an array entry
                 is equal to the size of a weight in the adjacency list
   prev        : pointer to a preallocated array with a count that is equal
                 to the number of vertices in the adjacency list
   hht         : - NULL pointer, if a default hash table is used for
                 in-heap operations; a default hash table contains an index
                 array with a count that is equal to the number of vertices
                 - a pointer to a set of parameters specifying a hash table
                 used for in-heap operations; a vertex is a hash key in 
                 the hash table
   cmp_wt      : comparison function which returns a negative integer value
                 if the weight value pointed to by the first argument is
                 less than the weight value pointed to by the second, a
                 positive integer value if the weight value pointed to by
                 the first argument is greater than the weight value 
                 pointed to by the second, and zero integer value if the two
                 weight values are equal
*/
void prim(const adj_lst_t *a,
	  size_t start,
	  void *dist,
	  size_t *prev,
	  const heap_ht_t *hht,
	  int (*cmp_wt)(const void *, const void *));
#endif
