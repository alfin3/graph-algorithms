/**
   dijkstra.c

   Dijkstra's algorithm on graphs with generic non-negative weights and 
   a hash table parameter.
   
   Vertices are indexed from 0. Edge weights are of any basic type (e.g.
   char, int, long, float, double), or are custom weights within a
   contiguous block (e.g. pair of 64-bit segments to address the potential
   overflow due to addition).

   The hash table parameter specifies a hash table used for in-heap
   operations, and enables the optimization of space and time resources
   associated with heap operations in Dijkstra's algorithm by choice of a
   hash table and its load factor upper bound. If NULL is passed as a hash
   table parameter value, a default hash table is used, which contains an
   index array with a count that is equal to the number of vertices in the
   graph.   

   If E >> V, a default hash table may provide speed advantages by avoiding
   the computation of hash values. If V is large and the graph is sparse,
   a non-default hash table may provide space advantages.

   The implementation does not use stdint.h and is portable under C89/C90.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dijkstra.h"
#include "graph.h"
#include "heap.h"
#include "stack.h"
#include "utilities-mem.h"

typedef enum{FALSE, TRUE} boolean_t;

typedef struct{
  size_t count;
} context_t;

typedef struct{
  size_t key_size;
  size_t elt_size;
  size_t count;
  boolean_t *key_present;
  void *elts;
  void (*free_elt)(void *);
} ht_def_t;

static const size_t C_NREACHED = (size_t)-1; /* not reached as index */

/* default hash table operations */
static void ht_def_init(ht_def_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context);
static void ht_def_insert(ht_def_t *ht, const size_t *key,const void *elt);
static void *ht_def_search(const ht_def_t *ht, const size_t *key);
static void ht_def_remove(ht_def_t *ht, const size_t *key, void *elt);
static void ht_def_free(ht_def_t *ht);

/* functions for computing pointers */
static size_t *vt_ptr(const size_t *vts, size_t i);
static void *wt_ptr(const void *wts, size_t i, size_t wt_size);
static void *elt_ptr(const void *elts, size_t i, size_t elt_size);

/**
   Computes and copies the shortest distances from start to the array
   pointed to by dist, and the previous vertices to the array pointed to by
   prev, with the maximal value of size_t in the prev array for unreached
   vertices.
   a           : pointer to an adjacency list with at least one vertex
   start       : start vertex for running the algorithm
   dist        : pointer to a preallocated array where the count is equal
                 to the number of vertices, and the size of an array entry
                 is equal to the size of a weight in the adjacency list
   prev        : pointer to a preallocated array with a count that is equal
                 to the number of vertices in the adjacency list
   hht         : - NULL pointer, if a default hash table is used for
                 in-heap operations; a default hash table contains an index
                 array with a count that is equal to the number of vertices
                 - a pointer to a set of parameters specifying a hash table
                 used for in-heap operations; a vertex is a hash key in 
                 the hash table
   add_wt      : addition function which copies the sum of the weight values
                 pointed to by the second and third arguments to the
                 preallocated weight block pointed to by the first argument
   cmp_wt      : comparison function which returns a negative integer value
                 if the weight value pointed to by the first argument is
                 less than the weight value pointed to by the second, a
                 positive integer value if the weight value pointed to by
                 the first argument is greater than the weight value 
                 pointed to by the second, and zero integer value if the two
                 weight values are equal
*/
void dijkstra(const adj_lst_t *a,
	      size_t start,
	      void *dist,
	      size_t *prev,
	      const heap_ht_t *hht,
	      void (*add_wt)(void *, const void *, const void *),
	      int (*cmp_wt)(const void *, const void *)){
  size_t wt_size = a->wt_size;
  size_t vt_size = sizeof(size_t);
  size_t init_count = 1;
  size_t u, v;
  size_t i;
  void *u_wt = NULL, *v_wt = NULL, *sum_wt = NULL;
  ht_def_t ht_def;
  context_t context;
  heap_ht_t hht_def;
  heap_t h;
  u_wt = malloc_perror(1, wt_size);
  sum_wt = malloc_perror(1, wt_size);
  memset(dist, 0, a->num_vts * wt_size);
  memset(prev, 0xff, a->num_vts * vt_size); /* initialize to C_NREACHED */
  if (hht == NULL){
    context.count = a->num_vts;
    hht_def.ht = &ht_def;
    hht_def.context = &context;
    hht_def.init = (heap_ht_init)ht_def_init;
    hht_def.insert = (heap_ht_insert)ht_def_insert;
    hht_def.search = (heap_ht_search)ht_def_search;
    hht_def.remove = (heap_ht_remove)ht_def_remove;
    hht_def.free = (heap_ht_free)ht_def_free;
    heap_init(&h, init_count, wt_size, vt_size, &hht_def, cmp_wt, NULL);
  }else{
    heap_init(&h, init_count, wt_size, vt_size, hht, cmp_wt, NULL);
  }
  heap_push(&h, wt_ptr(dist, start, wt_size), &start);
  prev[start] = start;
  while (h.num_elts > 0){
    heap_pop(&h, u_wt, &u);
    for (i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      v_wt = wt_ptr(dist, v, wt_size);
      add_wt(sum_wt, u_wt, wt_ptr(a->wts[u]->elts, i, wt_size));
      if (prev[v] == C_NREACHED){
	memcpy(v_wt, sum_wt, wt_size);
	heap_push(&h, v_wt, &v);
	prev[v] = u;
      }else if (cmp_wt(v_wt, sum_wt) > 0){
	/* must be in the heap */
	memcpy(v_wt, sum_wt, wt_size);
	heap_update(&h, v_wt, &v);
	prev[v] = u;
      }
    }
  }
  heap_free(&h);
  free(u_wt);
  free(sum_wt);
  u_wt = NULL;
  sum_wt = NULL;
}

/**
   Default hash table operations. In the main algorithm routine, the
   elt_size block pointed to by elt in heap_push is a vertex (index).
   Therefore, a vertex is a hash key in the hash table of the heap, as per
   the specification of the hash table parameter of the heap.
*/

static void ht_def_init(ht_def_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context){
  context_t *c = context;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->count = c->count;
  ht->key_present = calloc_perror(c->count, sizeof(boolean_t));
  ht->elts = malloc_perror(c->count, elt_size);
  ht->free_elt = free_elt;
}

static void ht_def_insert(ht_def_t *ht, const size_t *key, const void *elt){
  ht->key_present[*key] = TRUE;
  memcpy(elt_ptr(ht->elts, *key, ht->elt_size),
	 elt,
	 ht->elt_size);
}

static void *ht_def_search(const ht_def_t *ht, const size_t *key){
  if (ht->key_present[*key]){
    return elt_ptr(ht->elts, *key, ht->elt_size);
  }else{
    return NULL;
  }
}

static void ht_def_remove(ht_def_t *ht, const size_t *key, void *elt){
  ht->key_present[*key] = FALSE;
  memcpy(elt,
	 elt_ptr(ht->elts, *key, ht->elt_size),
	 ht->elt_size);
}

static void ht_def_free(ht_def_t *ht){
  size_t i;
  if (ht->free_elt != NULL){
    for (i = 0; i < ht->count; i++){
      if (ht->key_present[i]){
	ht->free_elt(elt_ptr(ht->elts, i, ht->elt_size));
      }
    }
  }
  free(ht->key_present);
  free(ht->elts);
  ht->key_present = NULL;
  ht->elts = NULL;
}

/** Functions for computing pointers */

/**
   Computes a pointer to an entry in an array of vertices.
*/
static size_t *vt_ptr(const size_t *vts, size_t i){
  return (size_t *)vts + i;
}

/**
   Computes a pointer to an entry in an array of weights.
*/
static void *wt_ptr(const void *wts, size_t i, size_t wt_size){
  return (void *)((char *)wts + i * wt_size);
}
 
/**
   Computes a pointer to an entry in the array of elements in a default hash
   table.
*/
static void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
}
