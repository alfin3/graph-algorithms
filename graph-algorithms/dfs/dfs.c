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

static const size_t C_STACK_INIT_COUNT = 1;

static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t offset,
		   void *c,
		   const void *nr,
		   const void *ix,
		   void *pre,
		   void *post,
		   size_t (*read_vt)(const void *),
		   void *(*at_vt)(const void *, const void *),
		   int (*cmp_vt)(const void *, const void *),
		   void (*incr_vt)(void *));
static void *ptr(const void *block, size_t i, size_t size);

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
         void (*incr_vt)(void *)){
  size_t v_rem, uval_rem;
  size_t offset, block_size;
  stack_t s;
  /* variables in single block for cache-efficiency */
  void *vars = malloc_perror(6, a->vt_size);
  void * const su = vars;
  void * const c = ptr(vars, 1, a->vt_size);
  void * const nr = ptr(vars, 2, a->vt_size);
  void * const zero = ptr(vars, 3, a->vt_size);
  void * const ix = ptr(vars, 4, a->vt_size);
  void * const end = ptr(vars, 5, a->vt_size);
  write_vt(su, start);
  write_vt(c, 0);
  write_vt(nr, mul_sz_perror(2, a->num_vts));
  write_vt(zero, 0);
  write_vt(ix, 0);
  write_vt(end, a->num_vts);
  while (cmp_vt(ix, end) != 0){
    memcpy(at_vt(pre, ix), nr, a->vt_size);
    incr_vt(ix);
  }
  /* compute the size of v_uval block */
  if (sizeof(void *) <= a->vt_size){
    offset = a->vt_size;
  }else{
    uval_rem = sizeof(void *) % a->vt_size;
    offset = add_sz_perror(sizeof(void *),
			   (uval_rem > 0) * (a->vt_size - uval_rem));
  }
  v_rem = add_sz_perror(offset, a->vt_size) % sizeof(void *);
  block_size = add_sz_perror(offset + a->vt_size,
			     (v_rem > 0) * (sizeof(void *) - v_rem));

  /* run search with recursion emulation on a stack ds */
  stack_init(&s, C_STACK_INIT_COUNT, block_size, NULL);
  memcpy(ix, su, a->vt_size);
  while (cmp_vt(ix, end) != 0){
    if (cmp_vt(at_vt(pre, ix), nr) == 0){
      search(a, &s, offset, c, nr, ix, pre, post,
	     read_vt, at_vt, cmp_vt, incr_vt);
    }
    incr_vt(ix);
  }
  memcpy(end, su, a->vt_size);
  memcpy(ix, zero, a->vt_size);
  while (cmp_vt(ix, end) != 0){
    if (cmp_vt(at_vt(pre, ix), nr) == 0){
      search(a, &s, offset, c, nr, ix, pre, post,
	     read_vt, at_vt, cmp_vt, incr_vt);
    }
    incr_vt(ix);
  }
  stack_free(&s);
  free(vars);
  vars = NULL;
}

/**
   Performs a DFS search of a graph component reachable from an unexplored
   vertex provided by the u parameter by emulating the recursion in DFS on
   a dynamically allocated stack data structure.
*/
static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t offset,
		   void *c,
		   const void *nr,
		   const void *ix,
		   void *pre,
		   void *post,
		   size_t (*read_vt)(const void *),
		   void *(*at_vt)(const void *, const void *),
		   int (*cmp_vt)(const void *, const void *),
		   void (*incr_vt)(void *)){
  const void *v = NULL, *v_end = NULL;
  void *v_uval = malloc_perror(1, s->elt_size);
  const void ** const vp  = v_uval;
  void * const u = ptr(v_uval, 1, offset);
  memcpy(u, ix, a->vt_size);
  *vp = a->vt_wts[read_vt(u)]->elts;
  memcpy(at_vt(pre, u), c, a->vt_size);
  incr_vt(c);
  stack_push(s, v_uval);
  while (s->num_elts > 0){
    stack_pop(s, v_uval);
    v = *vp; /* for performance */
    v_end = ptr(a->vt_wts[read_vt(u)]->elts,
		a->vt_wts[read_vt(u)]->num_elts,
		a->pair_size);
    /* iterate v across the u's stack */
    while (v != v_end && cmp_vt(at_vt(pre, v), nr) != 0){
      v = (char *)v + a->pair_size;
    }
    if (v == v_end){
      memcpy(at_vt(post, u), c, a->vt_size);
      incr_vt(c);
    }else{
      *vp = v;
      stack_push(s, v_uval); /* push the unfinished vertex */
      memcpy(u, *vp, a->vt_size);
      *vp = a->vt_wts[read_vt(u)]->elts;
      memcpy(at_vt(pre, u), c, a->vt_size);
      incr_vt(c);
      stack_push(s, v_uval); /* then push an unexplored vertex */
    }
  }
  free(v_uval);
  v_uval = NULL;
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
