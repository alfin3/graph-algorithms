/**
   bfs.h

   Declarations of accessible functions for running the BFS algorithm on
   graphs with generic integer vertices indexed from 0.

   A graph may be unweighted or weighted. In the latter case the weights of
   the graph are ignored.

   The implementation introduces two parameters (cmpat_vt and incr_vt)
   that are designed to inform a compiler to perform optimizations to
   match or nearly match the performance of the generic BFS to the
   corresponding non-generic version with a fixed integer type of vertices.

   A distance value in the dist array is only set if the corresponding
   vertex was reached, in which case it is guaranteed that the distance
   object representation is not a trap representation. If the dist array is
   allocated with calloc, then for any integer type the representation
   with all zero bits is 0 integer value under C99 and C11 (6.2.6.2),
   and it is safe to read such a representation even if the value
   was not set by the algorithm.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.

   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.

   Optimization notes:

   -  The overhead of a bit array for cache-efficient set membership
   testing of reached and unreached vertices decreased performance in tests
   and is not included in the implementation.
*/

#ifndef BFS_H  
#define BFS_H

#include <stddef.h>
#include "graph.h"

int bfs_cmpat_ushort(const void *a, const void *i, const void *v);
int bfs_cmpat_uint(const void *a, const void *i, const void *v);
int bfs_cmpat_ulong(const void *a, const void *i, const void *v);
int bfs_cmpat_sz(const void *a, const void *i, const void *v);

void bfs_incr_ushort(void *a);
void bfs_incr_uint(void *a);
void bfs_incr_ulong(void *a);
void bfs_incr_sz(void *a);

/**
   Computes and copies to an array pointed to by dist the lowest # of edges
   from start to each reached vertex, and provides the previous vertex in
   the array pointed to by prev, with the number of vertices in a graph as
   the special value in prev for unreached vertices. Assumes start is valid
   and there is at least one vertex.
   a           : pointer to an adjacency list with at least one vertex
   start       : a start vertex for running bfs
   dist        : pointer to a preallocated array with the count of elements
                 equal to the number of vertices in the adjacency list; the
                 size of each element is equal to the size of the integer
                 type used to represent vertices; if the pointed block has
                 no declared type then bfs sets the effective type of the
                 block to the integer type of vertices
   prev        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list; the size of each
                 element is equal to the size of the integer type used to
                 represent vertices; if the pointed block has no declared
                 type then bfs sets the effective type of the block to the
                 integer type of vertices
   cmpat_vt    : non-NULL pointer to a function for comparing the element in
                 the array pointed to by the first argument at the index
                 pointed to by the second argument, to the value pointed to
                 by the third argument; each argument points to a value of
                 the integer type used to represent vertices; the function
                 pointer may point to one of the provided functions
   incr_vt     : non-NULL pointer to a function incrementing an integer of
                 the type used to represent vertices; the function pointer
                 may point to one of the provided functions
*/
void bfs(const adj_lst_t *a,
	 size_t start,
	 void *dist,
	 void *prev,
	 int (*cmpat_vt)(const void *, const void *, const void *),
	 void (*incr_vt)(void *));

#endif
