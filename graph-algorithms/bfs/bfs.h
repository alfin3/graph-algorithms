/**
   bfs.h

   Declarations of accessible functions for running the BFS algorithm on
   graphs with generic integer vertices indexed from 0. A graph may be
   unweighted or weighted. In the latter case the weights of the graph are
   ignored.

   The effective type of every element in the prev array is of the integer
   type used to represent vertices. The value of every element is set
   by the algorithm to the value of the previous vertex. If the block
   pointed to by prev has no declared type then the algorithm sets the
   effective type of every element to the integer type used to represent
   vertices by writing a value of the type.

   A distance value in the dist array is only set if the corresponding
   vertex was reached, as indicated by the prev array, in which case it
   is guaranteed that the distance object representation is not a trap
   representation. An element corresponding to a not reached vertex, as
   indicated by the prev array, may be a trap representation. However,
   if the dist array is allocated with calloc, then for any integer type
   the representation with all zero bits is 0 integer value under C99 and
   C11 (6.2.6.2), and it is safe to read such a representation even if the
   value was not set by the algorithm.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation
   provides an error message and an exit is executed if an integer
   overflow is attempted or an allocation is not completed due to
   insufficient resources. The behavior outside the specified parameter
   ranges is undefined.

   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.

   Note: A bit array for cache-efficient set membership testing is
   not included due to an overhead that decreased the performance in tests.
*/

#ifndef BFS_H  
#define BFS_H

#include <stddef.h>
#include "graph.h"

/**
   Computes and copies to an array pointed to by dist the lowest # of edges
   from start to each reached vertex, and provides the previous vertex in
   the array pointed to by prev, with the number of vertices in a graph as
   the special value in prev for unreached vertices. Assumes start is valid
   and there is at least one vertex.
   a           : pointer to an adjacency list with at least one vertex
   start       : a start vertex for running bfs
   dist        : pointer to a preallocated array with the count of elements
                 equal to the number of vertices in the adjacency list; each
                 element is of size vt_size (vt_size block) that equals to
                 the size of the integer type used to represent vertices in
                 the adjacency list; if the block pointed to by dist has no
                 declared type then bfs sets the effective type of each
                 element corresponding to a reached vertex to the integer
                 type of vertices by writing a value of the type; if the
                 block was allocated with calloc then under C99 and C11 each
                 element corresponding to an unreached vertex, can be safely
                 read as an integer of the type used to represent vertices
                 and will represent 0 value
   prev        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list; each element
                 is of size vt_size (vt_size block) that equals to the
                 size of the integer type used to represent vertices in the
                 adjacency list; if the block pointed to by prev has no
                 declared type then it is guaranteed that bfs sets the
                 effective type of every element to the integer type used to
                 represent vertices by writing a value of the type
   read_vt     : reads the integer value of the type used to represent
                 vertices from the vt_size block pointed to by the argument
                 and returns a size_t value
   write_vt    : writes the integer value of the second argument to
                 the vt_size block pointed to by the first argument
                 as a value of the integer type used to represent vertices
   at_vt       : returns a pointer to the element in the array pointed to by
                 the first argument at the index pointed to by the second
                 argument; the first argument points to the integer type
                 used to represent vertices and is not dereferenced; the
                 second argument points to a value of the integer type used
                 to represent vertices and is dereferenced
   cmp_vt      : returns 0 iff the element pointed to by the first
                 argument is equal to the element pointed to by the second
                 argument; each argument points to a value of the integer
                 type used to represent vertices
   incr_vt     : increments a value of the integer type used to represent
                 vertices
*/
void bfs(const struct adj_lst *a,
	 size_t start,
	 void *dist,
	 void *prev,
         size_t (*read_vt)(const void *),
         void (*write_vt)(void *, size_t),
         void *(*at_vt)(const void *, const void *),
         int (*cmp_vt)(const void *, const void *),
         void (*incr_vt)(void *));

#endif
