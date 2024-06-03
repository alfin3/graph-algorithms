/**
   bfs.c

   Functions for running the BFS algorithm on graphs with generic integer
   vertices indexed from 0. A graph may be unweighted or weighted. In the
   latter case the weights of the graph are ignored.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfs.h"
#include "graph.h"
#include "queue.h"
#include "stack.h"
#include "utilities-mem.h"

static const size_t C_QUEUE_INIT_COUNT = 1;

static void *ptr(const void *block, size_t i, size_t size);

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
         void (*incr_vt)(void *)){
  const void *p = NULL, *p_start = NULL, *p_end = NULL;
  struct queue q;
  /* variables in single block for cache-efficiency */
  void * const vars = malloc_perror(5, a->vt_size);
  void * const u = vars;
  void * const nr = ptr(vars, 1, a->vt_size);
  void * const zero = ptr(vars, 2, a->vt_size);
  void * const ix = ptr(vars, 3, a->vt_size);
  void * const d = ptr(vars, 4, a->vt_size);
  write_vt(u, start);
  write_vt(nr, a->num_vts);
  write_vt(zero, 0);
  write_vt(ix, 0);
  write_vt(d, 0);
  write_vt(at_vt(dist, u), 0);
  while (cmp_vt(ix, nr) != 0){
    memcpy(at_vt(prev, ix), nr, a->vt_size);
    incr_vt(ix);
  }
  memcpy(at_vt(prev, u), u, a->vt_size);
  queue_init(&q, a->vt_size, NULL);
  queue_bound(&q, C_QUEUE_INIT_COUNT, a->num_vts);
  queue_push(&q, u);
  while (q.num_elts > 0){
    queue_pop(&q, u);
    memcpy(d, at_vt(dist, u), a->vt_size);
    incr_vt(d);
    p_start = a->vt_wts[read_vt(u)]->elts;
    p_end = (char *)p_start + a->vt_wts[read_vt(u)]->num_elts * a->pair_size;
    for (p = p_start; p != p_end; p = (char *)p + a->pair_size){
      if (cmp_vt(at_vt(prev, p), nr) == 0){
        memcpy(at_vt(dist, p), d, a->vt_size);
        memcpy(at_vt(prev, p), u, a->vt_size);
        queue_push(&q, p);
      }
    }
  }
  queue_free(&q);
  free(vars);
  /* after this line vars cannot be dereferenced */
}

/**
   Computes a pointer to the ith element in the block of elements.

   According to C89 (draft):

   "It is guaranteed, however, that a pointer to an object of a given
   alignment may be converted to a pointer to an object of the same
   alignment or a less strict alignment and back again; the result shall
   compare equal to the original pointer. (An object that has character
   type has the least strict alignment.)"

   "A pointer to void may be converted to or from a pointer to any
   incomplete or object type. A pointer to any incomplete or object type
   may be converted to a pointer to void and back again; the result shall
   compare equal to the original pointer."

   "A pointer to void shall have the same representation and alignment
   requirements as a pointer to a character type."
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}
