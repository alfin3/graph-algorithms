/**
   dijkstra.c

   Dijkstra's algorithm with a hash table parameter on graphs with generic
   non-negative weights.
    
   Edge weights are of any basic type (e.g. char, int, double).

   The hash table parameter specifies a hash table used for in-heap
   operations, and enables the optimization of space and time resources
   associated with heap operations in Dijkstra's algorithm by choice of a
   hash table and its load factor upper bound. 

   If E >> V, then passing NULL as the hash table parameter, 
   Otherwise,
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "dijkstra.h"
#include "graph.h"
#include "heap.h"
#include "stack.h"
#include "utilities-mem.h"

typedef struct{
  size_t num_vts;
} context_t;

typedef struct{
  size_t key_size;
  size_t elt_size;
  size_t count;
  size_t *keys; //array of vertices indexed from 0
  void *elts;
  int (*cmp_key)(const void *, const void *);
  void (*free_elt)(void *);
} ht_default_t;

static const size_t NR = SIZE_MAX; //not reached as index

//default hash table operations, suitable if V << E
static void ht_default_init(ht_default_t *ht,
			    size_t key_size,
			    size_t elt_size,
			    int (*cmp_key)(const void *, const void *),
			    void (*free_elt)(void *),
			    void *context);
static void ht_default_insert(ht_default_t *ht,
			      const void *key,
			      const void *elt);
static void *ht_default_search(const ht_default_t *ht, const void *key);
static void ht_default_remove(ht_default_t *ht, const void *key, void *elt);
static void ht_default_free(ht_default_t *ht);

//auxiliary functions
static size_t *vt_ptr(const size_t *vts, size_t i);
static void *wt_ptr(const void *wts, size_t i, size_t wt_size);
static void *elt_ptr(const void *elts, size_t i, size_t elt_size);

/**
   Computes and copies the shortest distances from start to dist array and 
   previous vertices to prev array, with nr in prev for unreached vertices.
   Assumes immutability of an adjacency list during execution.
*/
void dijkstra(const adj_lst_t *a,
	      size_t start,
	      void *dist,
	      size_t *prev,
	      const heap_ht_t *ht,
	      void (*add_wt)(void *, const void *, const void *),
	      int (*cmp_wt)(const void *, const void *)){
  size_t vt_size = sizeof(uint64_t);
  size_t wt_size = a->wt_size;
  size_t init_count = 1;
  size_t u, v;
  void *u_wt = NULL, *v_wt = NULL, *sum_wt = NULL;
  context_t context;
  heap_ht_t ht_default;
  heap_t h;
  u_wt = malloc_perror(wt_size);
  sum_wt = malloc_perror(wt_size);
  memset(dist, 0, a->num_vts * wt_size);
  memset(prev, 0xff, a->num_vts * vt_size); //initialize to NR
  if (ht == NULL){
    context.num_vts = a->num_vts;
    ht_default.size = sizeof(ht_default_t);
    ht_default.context = &context;
    ht_default.init = (heap_ht_init)ht_default_init;
    ht_default.insert = (heap_ht_insert)ht_default_insert;
    ht_default.search = (heap_ht_search)ht_default_search;
    ht_default.remove = (heap_ht_remove)ht_default_remove;
    ht_default.free = (heap_ht_free)ht_default_free;
    heap_init(&h, init_count, wt_size, vt_size, &ht_default, cmp_wt, NULL);
  }else{
    heap_init(&h, init_count, wt_size, vt_size, ht, cmp_wt, NULL);
  }
  heap_push(&h, wt_ptr(dist, start, wt_size), &start);
  prev[start] = start;
  while (h.num_elts > 0){
    heap_pop(&h, u_wt, &u);
    for (size_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      v_wt = wt_ptr(dist, v, wt_size);
      add_wt(sum_wt, u_wt, wt_ptr(a->wts[u]->elts, i, wt_size));
      if (prev[v] == NR){
	memcpy(v_wt, sum_wt, wt_size);
	heap_push(&h, v_wt, &v);
	prev[v] = u;
      }else if (cmp_wt(v_wt, sum_wt) > 0){
	//must be in the heap
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

static void ht_default_init(ht_default_t *ht,
			    size_t key_size,
			    size_t elt_size,
			    int (*cmp_key)(const void *, const void *),
			    void (*free_elt)(void *),
			    void *context){
  context_t *c = context;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->count = c->num_vts;
  ht->keys = malloc_perror(c->num_vts * key_size);
  ht->elts = malloc_perror(c->num_vts * elt_size);
  memset(ht->keys, 0xff, c->num_vts * key_size); //initialize to NR
  ht->cmp_key = cmp_key;
  ht->free_elt = free_elt;
}

static void ht_default_insert(ht_default_t *ht,
			      const void *key,
			      const void *elt){
  ht->keys[*(size_t *)key] = *(size_t *)key;
  memcpy(elt_ptr(ht->elts, *(size_t *)key, ht->elt_size),
	 elt,
	 ht->elt_size);
}

static void *ht_default_search(const ht_default_t *ht, const void *key){
  if (ht->keys[*(size_t *)key] == NR){
    return NULL;
  }else{
    return elt_ptr(ht->elts, *(size_t *)key, ht->elt_size);
  }
}

static void ht_default_remove(ht_default_t *ht, const void *key, void *elt){
  ht->keys[*(size_t *)key] = NR;
  memcpy(elt,
	 elt_ptr(ht->elts, *(size_t *)key, ht->elt_size),
	 ht->elt_size);
}

static void ht_default_free(ht_default_t *ht){
  for (size_t i = 0; i < ht->count; i++){
    if (ht->keys[i] != NR){
      ht->free_elt(elt_ptr(ht->elts, i, ht->elt_size));
    }
  }
  free(ht->keys);
  free(ht->elts);
  ht->keys = NULL;
  ht->elts = NULL;
}

/** Helper functions */

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
   Computes a pointer to an entry in the element array of a default hash
   table.
*/
static void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
}
