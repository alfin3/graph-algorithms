/**
   prim.h

   Declarations of accessible functions for running Prim's algorithm on
   undirected graphs with generic integer vertices, generic weights (incl.
   negative) and a hash table parameter.

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

   The effective type of every element in the prev array is of the integer
   type used to represent vertices. The value of every element is set
   by the algorithm to the value of the previous vertex. If the block
   pointed to by prev has no declared type then the algorithm sets the
   effective type of every element to the integer type used to represent
   vertices by writing a value of the type, including a special value
   for unreached vertices.

   A distance value in the dist array is only set if the corresponding
   vertex was reached, as indicated by the prev array, in which case it
   is guaranteed that the distance object representation is not a trap
   representation. An element corresponding to a not reached vertex, as
   indicated by the prev array, may be a trap representation. However,
   if distances are of an integer type and the dist array is allocated with
   calloc, then for any integer type the representation with all zero
   bits is 0 integer value under C99 and C11 (6.2.6.2), and it is safe
   to read such a representation even if the value was not set by the
   algorithm.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation
   provides an error message and an exit is executed if an integer
   overflow is attempted or an allocation is not completed due to
   insufficient resources. The behavior outside the specified parameter
   ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99.
*/

#ifndef PRIM_H  
#define PRIM_H

#include <stddef.h>
#include "graph.h"

/**
   Prim hash table parameter struct, pointing to the hash table op
   helpers, pre-defined in each hash table.
*/
typedef struct{
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
} prim_ht_t;

/**
   Computes and copies the edge weights of an mst of the connected component
   of a start vertex to the array pointed to by dist, and the previous
   vertices to the array pointed to by prev, with the number of vertices as
   the special value in the prev array for unreached vertices.
   a           : pointer to an adjacency list with at least one vertex
   start       : start vertex for running the algorithm
   dist        : pointer to a preallocated array with the count of elements
                 equal to the number of vertices in the adjacency list; each
                 element is of size wt_size (wt_size block) that equals to
                 the size of a weight in the adjacency list; if the block
                 pointed to by dist has no declared type then prim sets
                 the effective type of each element corresponding to a
                 reached vertex to the type of a weight in the adjacency list
                 by writing a value of the type; if distances are of an 
                 integer type and the block was allocated with calloc then
                 under C99 and C11 each element corresponding to an unreached
                 vertex can be safely read as the integer type and will
                 represent 0 value
   prev        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list; each element
                 is of size vt_size (vt_size block) that equals to the
                 size of the integer type used to represent vertices in the
                 adjacency list; if the block pointed to by prev has no
                 declared type then it is guaranteed that prim sets the
                 effective type of every element to the integer type used to
                 represent vertices by writing a value of the type
   zero_wt     : pointer to a block of size wt_size with a zero value of
                 the type used to represent weights; this value is copied
                 to the element in dist corresponding to the start vertex
   pmht        : - NULL pointer, if a default hash table is used for
                 in-heap operations; a default hash table contains an index
                 array with a count that is equal to the number of vertices
                 - a pointer to a set of parameters specifying a hash table
                 used for in-heap operations
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
	  void *prev,
	  const void *wt_zero,
	  const prim_ht_t *pmht,
	  size_t (*read_vt)(const void *),
	  void (*write_vt)(void *, size_t),
	  void *(*at_vt)(const void *, const void *),
	  int (*cmp_vt)(const void *, const void *),
	  int (*cmp_wt)(const void *, const void *));
#endif
