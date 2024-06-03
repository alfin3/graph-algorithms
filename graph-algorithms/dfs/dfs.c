/**
   dfs.c

   Functions for running the DFS algorithm on graphs with generic integer
   vertices indexed from 0. A graph may be unweighted or weighted. In the
   latter case the weights of the graph are ignored.

   The recursion in DFS is emulated on a dynamically allocated stack data
   structure to avoid an overflow of the memory stack. An option to
   optimally align the elements in the stack is provided and may increase
   the cache efficiency and reduce the space requirement*, depending on
   the system and the choice of the integer type for representing vertices.

   The effective type of every element in the previsit and postvisit arrays
   is of the integer type used to represent vertices. The value of every
   element is set by the algorithm. If the block pointed to by pre or post
   has no declared type then the algorithm sets the effective type of every
   element to the integer type used to represent vertices by writing a value
   of the type.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation
   provides an error message and an exit is executed if an integer
   overflow is attempted or an allocation is not completed due to
   insufficient resources. The behavior outside the specified parameter
   ranges is undefined.

   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.

   * A bit array for cache-efficient set membership testing is
   not included due to an overhead that decreased the performance in tests.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

static const size_t C_STACK_INIT_COUNT = 1;

static void dfs_helper(const struct adj_lst *a,
                       size_t start,
                       size_t vt_alignment,
                       size_t vdptr_alignment,
                       void *pre,
                       void *post,
                       size_t (*read_vt)(const void *),
                       void (*write_vt)(void *, size_t),
                       void *(*at_vt)(const void *, const void *),
                       int (*cmp_vt)(const void *, const void *),
                       void (*incr_vt)(void *));
static void search(const struct adj_lst *a,
                   struct stack *s,
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
                 number of vertices in the adjacency list; each element
                 is of size vt_size (vt_size block) that equals to the
                 size of the integer type used to represent vertices in the
                 adjacency list; if the block pointed to by pre has no
                 declared type then it is guaranteed that dfs sets the
                 effective type of every element to the integer type used to
                 represent vertices by writing a value of the type
   post        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list; each element
                 is of size vt_size (vt_size block) that equals to the
                 size of the integer type used to represent vertices in the
                 adjacency list; if the block pointed to by post has no
                 declared type then it is guaranteed that dfs sets the
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
void dfs(const struct adj_lst *a,
         size_t start,
         void *pre,
         void *post,
         size_t (*read_vt)(const void *),
         void (*write_vt)(void *, size_t),
         void *(*at_vt)(const void *, const void *),
         int (*cmp_vt)(const void *, const void *),
         void (*incr_vt)(void *)){
  dfs_helper(a,
             start,
             a->vt_size,
             sizeof(void *),
             pre,
             post,
             read_vt,
             write_vt,
             at_vt,
             cmp_vt,
             incr_vt);
}

/**
   Computes and copies to the arrays pointed to by pre and post the previsit
   and postvisit values of a DFS search from a start vertex. Assumes start
   is valid and there is at least one vertex. Sets the alignment of void *
   vertex value pairs in the dynamically allocated stack used to emulate
   the dfs recursion. If the alignment requirement of only one type is known,
   then the size of the other type can be used as a value of the other
   alignment parameter because size of type >= alignment requirement of type
   (due to structure of arrays). The call to this operation may result in an
   increase in cache efficiency and a reduction of the space requirements as
   compared to dfs. Also see the parameter specification in dfs.
   vt_alignment : alignment requirement or size of the integer type used
                  to represent vertices; the size is equal to vt_size, which
                  accounts for padding according to sizeof
   vdp_alignment: alignment requirement of void * (i.e. alignof(void *)) or
                  size of void * according to sizeof
*/
void dfs_align(const struct adj_lst *a,
               size_t start,
               size_t vt_alignment,
               size_t vdp_alignment,
               void *pre,
               void *post,
               size_t (*read_vt)(const void *),
               void (*write_vt)(void *, size_t),
               void *(*at_vt)(const void *, const void *),
               int (*cmp_vt)(const void *, const void *),
               void (*incr_vt)(void *)){
  dfs_helper(a,
             start,
             vt_alignment,
             vdp_alignment,
             pre,
             post,
             read_vt,
             write_vt,
             at_vt,
             cmp_vt,
             incr_vt);
}

static void dfs_helper(const struct adj_lst *a,
                       size_t start,
                       size_t vt_alignment,
                       size_t vdp_alignment,
                       void *pre,
                       void *post,
                       size_t (*read_vt)(const void *),
                       void (*write_vt)(void *, size_t),
                       void *(*at_vt)(const void *, const void *),
                       int (*cmp_vt)(const void *, const void *),
                       void (*incr_vt)(void *)){
  size_t vt_rem, vdp_rem;
  size_t vt_offset, pair_size;
  struct stack s;
  /* variables in single block for cache-efficiency */
  void * const vars = malloc_perror(6, a->vt_size);
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
  /* align v_uval block relative to a malloc's pointer in stack */
  if (sizeof(void *) <= vt_alignment){
    vt_offset = vt_alignment;
  }else{
    vt_rem = sizeof(void *) % vt_alignment;
    vt_offset = add_sz_perror(sizeof(void *),
                              (vt_rem > 0) * (vt_alignment - vt_rem));
  }
  vdp_rem = add_sz_perror(vt_offset, a->vt_size) % vdp_alignment;
  pair_size = add_sz_perror(vt_offset + a->vt_size,
                            (vdp_rem > 0) * (vdp_alignment - vdp_rem));
  /* run search with recursion emulation on a stack ds */
  stack_init(&s, pair_size, NULL);
  stack_bound(&s, C_STACK_INIT_COUNT, a->num_vts);
  memcpy(ix, su, a->vt_size);
  while (cmp_vt(ix, end) != 0){
    if (cmp_vt(at_vt(pre, ix), nr) == 0){
      search(a, &s, vt_offset, c, nr, ix, pre, post,
             read_vt, at_vt, cmp_vt, incr_vt);
    }
    incr_vt(ix);
  }
  memcpy(end, su, a->vt_size);
  memcpy(ix, zero, a->vt_size);
  while (cmp_vt(ix, end) != 0){
    if (cmp_vt(at_vt(pre, ix), nr) == 0){
      search(a, &s, vt_offset, c, nr, ix, pre, post,
             read_vt, at_vt, cmp_vt, incr_vt);
    }
    incr_vt(ix);
  }
  stack_free(&s);
  free(vars);
  /* after this line vars cannot be dereferenced */
}

/**
   Performs a DFS search of a graph component reachable from an unexplored
   vertex pointed to by the ix parameter by emulating the recursion in DFS
   on a dynamically allocated stack data structure.
*/
static void search(const struct adj_lst *a,
                   struct stack *s,
                   size_t vt_offset,
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
  void * const v_uval = malloc_perror(1, s->elt_size);
  const void ** const vp  = v_uval;
  void * const u = ptr(v_uval, 1, vt_offset);
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
  /* after this line v_uval cannot be dereferenced */
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
