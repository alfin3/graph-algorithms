/**
   bfs.c

   Functions for running the BFS algorithm on graphs with generic integer
   vertices indexed from 0.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfs.h"
#include "graph.h"
#include "queue.h"
#include "stack.h"
#include "utilities-mem.h"

static const size_t QUEUE_INIT_COUNT = 1;

static void *ptr(const void *block, size_t i, size_t size);

int bfs_cmpat_ushort(const void *a, const void *i, const void *v){
  return ((const unsigned short *)a)[*(const unsigned short *)i] !=
    *(const unsigned short *)v;
}

int bfs_cmpat_uint(const void *a, const void *i, const void *v){
  return ((const unsigned int *)a)[*(const unsigned int *)i] !=
    *(const unsigned int *)v;
}

int bfs_cmpat_ulong(const void *a, const void *i, const void *v){
  return ((const unsigned long *)a)[*(const unsigned long *)i] !=
    *(const unsigned long *)v;
}

int bfs_cmpat_sz(const void *a, const void *i, const void *v){
  return ((const size_t *)a)[*(const size_t *)i] != *(const size_t *)v;
}

void bfs_incr_ushort(void *a){
  (*(unsigned short *)a)++;
}

void bfs_incr_uint(void *a){
  (*(unsigned int *)a)++;
}

void bfs_incr_ulong(void *a){
  (*(unsigned long *)a)++;
}

void bfs_incr_sz(void *a){
  (*(size_t *)a)++;
}

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
                 element is of the integer type used to represent vertices
                 in the adjacency list; if the pointed block has no declared
                 type then bfs sets the effective type of each element
                 corresponding to a reached vertex to the integer type of
                 vertices; if the block was allocated with calloc then
                 under C99 and C11 each element corresponding to an
                 unreached vertex, can be safely read as an integer of the
                 type used to represent vertices and will represent 0 value
   prev        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list; each element is
                 of the integer type used to represent vertices and the
                 value of every element is set by the algorithm; if the
                 pointed block has no declared type then bfs sets the
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
void bfs(const adj_lst_t *a,
	 size_t start,
	 void *dist,
	 void *prev,
	 int (*cmpat_vt)(const void *, const void *, const void *),
	 void (*incr_vt)(void *)){
  char *dp = NULL, *pp = NULL;
  const char *du = NULL;
  const char *p = NULL, *p_start = NULL, *p_end = NULL;
  void *unr = NULL; /* vertex, not reached vertex value */
  void *u = NULL, *nr = NULL;
  queue_t q;
  /* single block for cache-efficiency; same type */
  unr = malloc_perror(2, a->vt_size);
  u = unr;
  nr = ptr(unr, 1, a->vt_size);
  a->write_vt(u, start);
  a->write_vt(nr, a->num_vts);
  a->write_vt(ptr(dist, start, a->vt_size), 0);
  for (pp = prev; pp != ptr(prev, a->num_vts, a->vt_size); pp += a->vt_size){
    memcpy(pp, nr, a->vt_size);
  }
  queue_init(&q, QUEUE_INIT_COUNT, a->vt_size, NULL);
  memcpy(ptr(prev, a->read_vt(u), a->vt_size), u, a->vt_size);
  queue_push(&q, u);
  while (q.num_elts > 0){
    queue_pop(&q, u);
    du = ptr(dist, a->read_vt(u), a->vt_size);
    p_start = a->vt_wts[a->read_vt(u)]->elts;
    p_end = p_start + a->vt_wts[a->read_vt(u)]->num_elts * a->pair_size;
    for (p = p_start; p != p_end; p += a->pair_size){
      if (cmpat_vt(prev, p, nr) == 0){
        dp = ptr(dist, a->read_vt(p), a->vt_size);
        pp = ptr(prev, a->read_vt(p), a->vt_size);
	memcpy(dp, du, a->vt_size);
        incr_vt(dp);
	memcpy(pp, u, a->vt_size);
	queue_push(&q, p);
      }
    }
  }
  queue_free(&q);
  free(unr);
  unr = NULL;
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}
