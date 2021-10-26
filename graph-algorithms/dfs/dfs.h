/**
   dfs.h

   Declarations of accessible functions for running the DFS algorithm on 
   graphs with generic integer vertices indexed from 0.  A graph may be
   unweighted or weighted. In the latter case the weights of the graph are
   ignored.

   The recursion in DFS is emulated on a dynamically allocated stack data
   structure to avoid an overflow of the memory stack. An option to
   optimally align the elements in the stack is provided and may increase
   the cache efficiency and reduce the space requirement*, depending on
   the system and the choice of the integer type for representing vertices.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation
   provides an error message and an exit is executed if an integer
   overflow is attempted or an allocation is not completed due to
   insufficient resources. The behavior outside the specified parameter
   ranges is undefined.

   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.

   * The overhead of a bit array for cache-efficient set membership testing
   is excluded due to decreased performance in tests.
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
   pre         : pointer to a preallocated array of vt_size elements with
                 the count equal to the number of vertices in the adjacency
                 list; each element is of the integer type used to represent
                 vertices and the value of every element is set by the
                 algorithm; if the pointed block has no declared type then
                 dfs sets the effective type of every element to the integer
                 type used to represent vertices
   post        : pointer to a preallocated array of vt_size elements with
                 the count equal to the number of vertices in the adjacency
                 list; each element is of the integer type used to represent
                 vertices and the value of every element is set by the
                 algorithm; if the pointed block has no declared type then
                 dfs sets the effective type of every element to the integer
                 type used to represent vertices
   read_vt     : reads the integer value of the type used to represent
                 vertices from the vt_size block pointed to by the argument
                 and returns a size_t value
   write_vt    : writes the integer value of the second argument to
                 the vt_size block pointed to by the first argument
                 as a value of the integer type used to represent vertices
   at_vt       : returns a pointer to the element in the array pointed to by
                 the first argument at the index pointed to by the second
                 argument; each argument points to a value of the integer
                 type used to represent vertices
   cmp_vt      : returns 0 iff the the element pointed to by the first
                 argument is equal to the element pointed to by the second
                 argument; each argument points to a value of the integer
                 type used to represent vertices
   incr_vt     : increments a value of the integer type used to represent
                 vertices
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

/**
   Computes and copies to the arrays pointed to by pre and post the previsit
   and postvisit values of a DFS search from a start vertex. Assumes start
   is valid and there is at least one vertex. Sets the alignment of void *
   and vertex value pairs in the dynamically allocated stack used to emulate
   the dfs recursion. If the alignment requirement of only one type is known,
   then the size of the other type can be used as a value of the other
   alignment parameter because size of type >= alignment requirement of type
   (due to structure of arrays). The call to this operation may result in an
   increase in cache efficiency and a reduction of the space requirements as
   compared to dfs. Also see the parameter specification in dfs.
   vt_alignment : alignment requirement or size of the type of the vt_size
                  block of a vertex, which is the representation of an
                  integer type value; if size, must account for padding
                  according to sizeof
   vdp_alignment: alignment requirement according to void * (i.e.
                  alignof(void *)) or size of void * according to
                  sizeof
*/
void dfs_align(const adj_lst_t *a,
	       size_t start,
	       size_t vt_alignment,
	       size_t vdp_alignment,
	       void *pre,
	       void *post,
	       size_t (*read_vt)(const void *),
	       void (*write_vt)(void *, size_t),
	       void *(*at_vt)(const void *, const void *),
	       int (*cmp_vt)(const void *, const void *),
	       void (*incr_vt)(void *));

#endif
