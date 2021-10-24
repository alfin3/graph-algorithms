/**
   dfs.h

   Declarations of accessible functions for running the DFS algorithm on 
   graphs with generic integer vertices indexed from 0.

   A graph may be unweighted or weighted. In the latter case the weights of
   the graph are ignored.

   The implementation introduces two parameters (cmpat_vt and incr_vt)
   that are designed to inform a compiler to perform optimizations to
   match the performance of the generic DFS to the corresponding non-generic
   version with a fixed integer type of vertices.

   The recursion in DFS is emulated on a dynamically allocated stack data
   structure to avoid an overflow of the memory stack.

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
   testing of explored and unexplored vertices decreased performance in tests
   and is not included in the implementation.
*/

#ifndef DFS_H  
#define DFS_H

#include <stddef.h>
#include "graph.h"

/**
   Computes and copies to the arrays pointed to by pre and post the previsit
   and postvisit values of a DFS search from a start vertex. Assumes start
   is valid and there is at least one vertex.
   a           : pointer to an adjacency list with at least one and at most
                 2**(P - 1) - 1 vertices, where P is the precision of the
                 integer type used to represent vertices
   start       : a start vertex for running dfs
   pre         : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list; each element is
                 of the integer type used to represent vertices and the
                 value of every element is set by the algorithm; if the
                 pointed block has no declared type then dfs sets the
                 effective type of every element to the integer type of
                 vertices
   post        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list; each element is
                 of the integer type used to represent vertices and the
                 value of every element is set by the algorithm; if the
                 pointed block has no declared type then dfs sets the
                 effective type of every element to the integer type of
                 vertices
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
void dfs(const adj_lst_t *a,
	 size_t start,
	 void *pre,
	 void *post,
         size_t (*read_vt)(const void *),
         void (*write_vt)(void *, size_t),
         void *(*at_vt)(const void *, const void *),
         int (*cmp_vt)(const void *, const void *),
         void (*incr_vt)(void *));

#endif
