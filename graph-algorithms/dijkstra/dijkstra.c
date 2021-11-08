/**
   dijkstra.c

   Dijkstra's algorithm on graphs with generic integer vertices, generic
   non-negative weights and a hash table parameter.

   The hash table parameter specifies a hash table used for in-heap
   operations, and enables the optimization of space and time resources
   associated with heap operations in Dijkstra's algorithm by choice of a
   hash table and its load factor upper bound. If NULL is passed as a hash
   table parameter value, a default hash table is used, which contains an
   integer array with a count that is equal to the number of vertices in the
   graph.   

   If E >> V, a default hash table may provide speed advantages by avoiding
   the computation of hash values. If V is large and the graph is sparse,
   a non-default hash table may provide space advantages.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dijkstra.h"
#include "graph.h"
#include "heap.h"
#include "stack.h"
#include "utilities-mem.h"

static const size_t C_HEAP_INIT_COUNT = 1;

typedef struct{
  size_t absent;
  size_t *elts;
  size_t (*read_vt)(const void *);
} ht_def_t;

static void ht_def_init(void *ht,
			size_t num_vts,
			size_t (*read_vt)(const void *));
static void ht_def_insert(void *ht, const void *vt, const void *ix);
static void *ht_def_search(const void *ht, const void *vt);
static void ht_def_remove(void *ht, const void *vt, void *ix);
static void ht_def_free(void *ht);

static size_t compute_wt_offset(const adj_lst_t *a);
static void *ptr(const void *block, size_t i, size_t size);

/**
   Computes and copies the shortest distances from start to the array
   pointed to by dist, and the previous vertices to the array pointed to by
   prev, with the number of vertices as the special value in the prev array
   for unreached vertices.
   a           : pointer to an adjacency list with at least one vertex
   start       : start vertex for running the algorithm
   dist        : pointer to a preallocated array with the count of elements
                 equal to the number of vertices in the adjacency list; each
                 element is of size wt_size (wt_size block) that equals to
                 the size of a weight in the adjacency list; if the block
                 pointed to by dist has no declared type then dijkstra sets
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
                 declared type then it is guaranteed that dijkstra sets the
                 effective type of every element to the integer type used to
                 represent vertices by writing a value of the type
   zero_wt     : pointer to a block of size wt_size with a zero value of
                 the type used to represent distances
   daht        : - NULL pointer, if a default hash table is used for
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
   add_wt      : addition function which copies the sum of the weight values
                 pointed to by the second and third arguments to the
                 preallocated weight block pointed to by the first argument
*/
void dijkstra(const adj_lst_t *a,
	      size_t start,
	      void *dist,
	      void *prev,
	      const void *wt_zero,
	      const dijkstra_ht_t *daht,
	      size_t (*read_vt)(const void *),
	      void (*write_vt)(void *, size_t),
	      void *(*at_vt)(const void *, const void *),
	      int (*cmp_vt)(const void *, const void *),
	      int (*cmp_wt)(const void *, const void *),
	      void (*add_wt)(void *, const void *, const void *)){
  ht_def_t ht_def;
  heap_ht_t hht;
  heap_t h;
  void *p = NULL, *p_start = NULL, *p_end = NULL;
  void *dp = NULL;
  /* variables in single block for cache-efficiency */
  void * const vars =
    malloc_perror(1, add_sz_perror(compute_wt_offset(a),
				   mul_sz_perror(2, a->wt_size)));
  void * const u = vars;
  void * const nr = (char *)u + a->vt_size;
  void * const du = (char *)u + compute_wt_offset(a);
  void * const s = (char *)du + a->wt_size;
  write_vt(u, start);
  write_vt(nr, a->num_vts);
  memcpy(du, wt_zero, a->wt_size);
  memcpy(s, wt_zero, a->wt_size);
  memcpy(ptr(dist, read_vt(u), a->wt_size), wt_zero, a->wt_size);
  p_start = prev;
  p_end = ptr(prev, a->num_vts, a->vt_size);
  for (p = p_start; p != p_end; p = (char *)p + a->vt_size){
    memcpy(p, nr, a->vt_size);
  }
  memcpy(at_vt(prev, u), u, a->vt_size);
  if (daht == NULL){
    ht_def_init(&ht_def, a->num_vts, read_vt);
    hht.ht = &ht_def;
    hht.alpha_n = 0;
    hht.log_alpha_d = 0;
    hht.init = NULL;
    hht.align = NULL;
    hht.insert = ht_def_insert;
    hht.search = ht_def_search;
    hht.remove = ht_def_remove;
    hht.free = ht_def_free;
  }else{
    hht.ht = daht->ht;
    hht.alpha_n = daht->alpha_n;
    hht.log_alpha_d = daht->log_alpha_d;
    hht.init = daht->init;
    hht.align = daht->align;
    hht.insert = daht->insert;
    hht.search = daht->search;
    hht.remove = daht->remove;
    hht.free = daht->free;
  }
  heap_init(&h, a->wt_size, a->vt_size, C_HEAP_INIT_COUNT, &hht,
	    cmp_wt, cmp_vt, read_vt, NULL);
  heap_push(&h, du, u);
  while (h.num_elts > 0){
    heap_pop(&h, du, u);
    p_start = a->vt_wts[read_vt(u)]->elts;
    p_end = ptr(p_start, a->vt_wts[read_vt(u)]->num_elts, a->pair_size);
    for (p = p_start; p != p_end; p = (char *)p + a->pair_size){
      add_wt(s, du, (char *)p + a->wt_offset);
      dp = ptr(dist, read_vt(p), a->wt_size);
      if (cmp_vt(at_vt(prev, p), nr) == 0){
	memcpy(dp, s, a->wt_size);
	memcpy(at_vt(prev, p), u, a->vt_size);
	heap_push(&h, dp, p);
      }else if (cmp_wt(dp, s) > 0){
	/* must be in the heap */
	memcpy(dp, s, a->wt_size);
	memcpy(at_vt(prev, p), u, a->vt_size);
	heap_update(&h, dp, p);
      }
    }
  }
  heap_free(&h);
  free(vars);
  /* vars cannot be dereferenced after this line */;
}

/**
   Default hash table operations, mapping values of the integer type
   used to represent vertices to size_t indices for in-heap operations.
*/

static void ht_def_init(void *ht,
			size_t num_vts,
			size_t (*read_vt)(const void *)){
  size_t i;
  ht_def_t *ht_def = ht;
  ht_def->absent = num_vts;
  ht_def->elts = malloc_perror(num_vts, sizeof(size_t));
  for (i = 0; i < num_vts; i++){
    /* <= num_vts elements in the heap; indices are < num_vts */ 
    ht_def->elts[i] = num_vts;
  }
  ht_def->read_vt = read_vt;
}

static void ht_def_insert(void *ht, const void *vt, const void *ix){
  ht_def_t *ht_def = ht;
  ht_def->elts[ht_def->read_vt(vt)] = *(size_t *)ix; 
}

static void *ht_def_search(const void *ht, const void *vt){
  const ht_def_t *ht_def = ht;
  const size_t *p = ht_def->elts + ht_def->read_vt(vt);
  if (*p != ht_def->absent){
    return (void *)p;
  }else{
    return NULL;
  }
}

static void ht_def_remove(void *ht, const void *vt, void *ix){
  ht_def_t *ht_def = ht;
  size_t *p = ht_def->elts + ht_def->read_vt(vt);
  if (*p != ht_def->absent){
    *(size_t *)ix = *p;
    *p = ht_def->absent;
  }
}

static void ht_def_free(void *ht){
  ht_def_t *ht_def = ht;
  free(ht_def->elts);
  ht_def->elts = NULL;
}

/**
   Computes the wt_offset from malloc's pointer in the vars block
   consisting of two vt_size blocks followed by two wt_size blocks.
*/
static size_t compute_wt_offset(const adj_lst_t *a){
  size_t wt_rem;
  size_t vt_pair_size = mul_sz_perror(2, a->vt_size);
  if (vt_pair_size <= a->wt_size) return a->wt_size;
  wt_rem = vt_pair_size % a->wt_size;
  return add_sz_perror(vt_pair_size, (wt_rem > 0) * (a->wt_size - wt_rem));
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
