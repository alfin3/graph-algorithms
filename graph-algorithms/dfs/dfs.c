/**
   dfs.c

   Functions for running the DFS algorithm on graphs with generic integer
   vertices indexed from 0.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

typedef struct{
  size_t u;
  const char *vp; /* vp is pointer to v in u's stack in an adj. list */
} uvp_t;

static const size_t STACK_INIT_COUNT = 1;

static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t u,
		   void *c,
		   void *pre,
		   void *post,
		   const void *nr,
                   int (*cmpat_vt)(const void *, const void *, const void *),
		   void (*incr_vt)(void *));
static void *ptr(const void *block, size_t i, size_t size);

int dfs_cmpat_ushort(const void *a, const void *i, const void *v){
  return ((const unsigned short *)a)[*(const unsigned short *)i] !=
    *(const unsigned short *)v;
}

int dfs_cmpat_uint(const void *a, const void *i, const void *v){
  return ((const unsigned int *)a)[*(const unsigned int *)i] !=
    *(const unsigned int *)v;
}

int dfs_cmpat_ulong(const void *a, const void *i, const void *v){
  return ((const unsigned long *)a)[*(const unsigned long *)i] !=
    *(const unsigned long *)v;
}

int dfs_cmpat_sz(const void *a, const void *i, const void *v){
  return ((const size_t *)a)[*(const size_t *)i] != *(const size_t *)v;
}

void dfs_incr_ushort(void *a){
  (*(unsigned short *)a)++;
}

void dfs_incr_uint(void *a){
  (*(unsigned int *)a)++;
}

void dfs_incr_ulong(void *a){
  (*(unsigned long *)a)++;
}

void dfs_incr_sz(void *a){
  (*(size_t *)a)++;
}

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
	 int (*cmpat_vt)(const void *, const void *, const void *),
	 void (*incr_vt)(void *)){
  size_t u;
  char *p = NULL;
  void *cnri = NULL; /* counter, not reached vertex value, index */
  void *c = NULL, *nr = NULL, *i = NULL;
  stack_t s;
  /* single block for cache-efficiency; same type */
  cnri = malloc_perror(3, a->vt_size);
  c = cnri;
  nr = ptr(cnri, 1, a->vt_size);
  i = ptr(cnri, 2, a->vt_size);
  a->write_vt(c, 0);
  a->write_vt(nr, mul_sz_perror(2, a->num_vts));
  a->write_vt(i, start);
  for (p = pre; p != ptr(pre, a->num_vts, a->vt_size); p += a->vt_size){
    memcpy(p, nr, a->vt_size);
  }
  stack_init(&s, STACK_INIT_COUNT, sizeof(uvp_t), NULL);
  for (u = start; u < a->num_vts; u++){
    if (cmpat_vt(pre, i, nr) == 0){
      search(a, &s, u, c, pre, post, nr, cmpat_vt, incr_vt);
    }
    incr_vt(i);
  }
  a->write_vt(i, 0);
  for (u = 0; u < start; u++){
    if (cmpat_vt(pre, i, nr) == 0){
      search(a, &s, u, c, pre, post, nr, cmpat_vt, incr_vt);
    }
    incr_vt(i);
  }
  stack_free(&s);
  free(cnri);
  cnri = NULL;
}

/**
   Performs a DFS search of a graph component reachable from an unexplored
   vertex provided by the u parameter by emulating the recursion in DFS on
   a dynamically allocated stack data structure.
*/
static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t u,
		   void *c,
		   void *pre,
		   void *post,
		   const void *nr,
                   int (*cmpat_vt)(const void *, const void *, const void *),
		   void (*incr_vt)(void *)){
  const char *p = NULL, *p_end = NULL;
  uvp_t uvp;
  uvp.u = u;
  uvp.vp = a->vt_wts[u]->elts;
  memcpy(ptr(pre, u, a->vt_size), c, a->vt_size);
  incr_vt(c);
  stack_push(s, &uvp);
  while (s->num_elts > 0){
    stack_pop(s, &uvp);
    p = uvp.vp;
    p_end = ptr(a->vt_wts[uvp.u]->elts,
		a->vt_wts[uvp.u]->num_elts,
		a->pair_size);
    while (p != p_end && cmpat_vt(pre, p, nr) != 0){
      p += a->pair_size;
    }
    if (p == p_end){
      memcpy(ptr(post, uvp.u, a->vt_size), c, a->vt_size);
      incr_vt(c);
    }else{
      uvp.vp = p;
      stack_push(s, &uvp); /* push the unfinished vertex */
      uvp.u = a->read_vt(p);
      uvp.vp = a->vt_wts[uvp.u]->elts;
      memcpy(ptr(pre, uvp.u, a->vt_size), c, a->vt_size);
      incr_vt(c);
      stack_push(s, &uvp); /* then push an unexplored vertex */
    }
  }
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}
