/**
   tsp.c

   Dynamic programming version of an exact solution of TSP with
   generic weights, including negative weights, in O(2^n n^2) assymptotic
   runtime, where n is the number of vertices in a tour.

   TODO
   - bit array representation
   - hash table parameter
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tsp.h"
#include "graph.h"
#include "ht-mul-uint64.h"
#include "stack.h"
#include "utilities-mem.h"
#include "utilities-rand-mod.h"

static const size_t NR = SIZE_MAX; //not reached as index

static void build_next(const adj_lst_t *a,
		       const ht_mul_uint64_t *prev_ht,
		       ht_mul_uint64_t *next_ht,
		       stack_t *prev_s,
		       stack_t *next_s,
		       void (*add_wt)(void *, const void *, const void *),
		       int (*cmp_wt)(const void *, const void *));
static size_t *vt_ptr(const size_t *vts, size_t i);
static void *wt_ptr(const void *wts, size_t i, size_t wt_size);
static void rdc_key(void *t, const void *s);
static int cmp_vt(const void *a, const void *b);

/**
   Copies to the block pointed to by dist the shortest tour length from 
   start to start across all vertices without revisiting, if a tour exists. 
   Returns 0 if a tour exists, otherwise returns 1.
*/
int tsp(const adj_lst_t *a,
	size_t start,
	void *dist,
	void (*add_wt)(void *, const void *, const void *),
	int (*cmp_wt)(const void *, const void *)){
  int final_dist_updated = 0;
  size_t vt_size = sizeof(size_t);
  size_t wt_size = a->wt_size;
  size_t set_count = 2; //including NR
  size_t u, v;
  size_t *prev_set = NULL;
  void *wt_buf = NULL;
  stack_t prev_s, next_s;
  ht_mul_uint64_t prev_ht, next_ht;
  prev_set = malloc_perror(set_count * vt_size);
  wt_buf = malloc_perror(wt_size);
  prev_set[0] = start;
  prev_set[1] = NR;
  stack_init(&prev_s, 1, set_count * vt_size, NULL);
  stack_push(&prev_s, prev_set);
  ht_mul_uint64_init(&prev_ht,
		     set_count * vt_size,
		     wt_size,
		     0.2,
		     rdc_key,
		     NULL);
  memset(dist, 0, wt_size);
  ht_mul_uint64_insert(&prev_ht, prev_set, dist);
  for (size_t i = 0; i < a->num_vts - 1; i++){
    set_count++;
    stack_init(&next_s, 1, set_count * vt_size, NULL);
    ht_mul_uint64_init(&next_ht,
		       set_count * vt_size,
		       wt_size,
		       0.2,
		       rdc_key,
		       NULL);
    build_next(a, &prev_ht, &next_ht, &prev_s, &next_s, add_wt, cmp_wt);
    stack_free(&prev_s);
    ht_mul_uint64_free(&prev_ht);
    prev_s = next_s;
    prev_ht = next_ht;
    if (prev_s.num_elts == 0){
      //no progress made
      stack_free(&prev_s);
      ht_mul_uint64_free(&prev_ht);
      free(prev_set);
      free(wt_buf);
      prev_set = NULL;
      wt_buf = NULL;
      return 1;
    }
  }
  //compute the return to start
  prev_set = realloc_perror(prev_set, (a->num_vts + 1) * vt_size);
  while (prev_s.num_elts > 0){
    stack_pop(&prev_s, prev_set);
    u = prev_set[0];
    for (size_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      if (v == start){
	add_wt(wt_buf,
	       ht_mul_uint64_search(&prev_ht, prev_set),
	       wt_ptr(a->wts[u]->elts, i, wt_size));
	if (!final_dist_updated){
	  memcpy(dist, wt_buf, wt_size);
	  final_dist_updated = 1;
	}else if (cmp_wt(dist, wt_buf) > 0){
	  memcpy(dist, wt_buf, wt_size);
	}
      }
    }
  }
  stack_free(&prev_s);
  ht_mul_uint64_free(&prev_ht);
  free(prev_set);
  free(wt_buf);
  prev_set = NULL;
  wt_buf = NULL;
  if (!final_dist_updated && a->num_vts > 1) return 1;
  return 0;
}

/**
   Builds reachable sets of count n + 1 from sets of count n. A set is an
   array of size_t vertices, where the first element is the last reached 
   vertex, and other elements are the previously reached sorted vertices. 
   A hashtable, maps a set to a distance. 
 */
static void build_next(const adj_lst_t *a,
		       const ht_mul_uint64_t *prev_ht,
		       ht_mul_uint64_t *next_ht,
		       stack_t *prev_s,
		       stack_t *next_s,
		       void (*add_wt)(void *, const void *, const void *),
		       int (*cmp_wt)(const void *, const void *)){
  size_t vt_size = sizeof(size_t);
  size_t wt_size = a->wt_size;
  size_t set_count = prev_ht->key_size / vt_size;
  size_t u, v;
  size_t *prev_set = NULL, *next_set = NULL;
  void *wt_buf = NULL, *next_wt = NULL;
  prev_set = malloc_perror(set_count * vt_size);
  next_set = malloc_perror((set_count + 1) * vt_size);
  wt_buf = malloc_perror(wt_size);
  while (prev_s->num_elts > 0){
    stack_pop(prev_s, prev_set);
    u = prev_set[0];
    for (size_t j = 0; j < a->vts[u]->num_elts; j++){
      v = *vt_ptr(a->vts[u]->elts, j);
      if (bsearch(&v, prev_set, set_count, vt_size, cmp_vt) == NULL){
	next_set[0] = v;
	memcpy(&next_set[1], prev_set, set_count * vt_size);
	qsort(&next_set[1], set_count, vt_size, cmp_vt);
	add_wt(wt_buf,
	       ht_mul_uint64_search(prev_ht, prev_set),
	       wt_ptr(a->wts[u]->elts, j, wt_size));
	next_wt = ht_mul_uint64_search(next_ht, next_set);
	if (next_wt == NULL){
	  ht_mul_uint64_insert(next_ht, next_set, wt_buf);
	  stack_push(next_s, next_set);
	}else if (cmp_wt(next_wt, wt_buf) > 0){
	  ht_mul_uint64_insert(next_ht, next_set, wt_buf);
	}
      }
    }
  }
  free(prev_set);
  free(next_set);
  free(wt_buf);
  prev_set = NULL;
  next_set = NULL;
  wt_buf = NULL;
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
   Reduces size of an NR terminated array to size_t.
   //TODO abstract out the 64-bit dependence via the hash table parameter
*/
static void rdc_key(void *t, const void *s){
  size_t r = 0;
  size_t i = 0;
  size_t n = SIZE_MAX;
  size_t *s_arr = (size_t *)s;
  while (s_arr[i] != NR){
    r = sum_mod_uint64(r, s_arr[i], n);
    i++;
  }
  *(size_t *)t = r;
}

/**
   Compares two size_t vertices.
*/
static int cmp_vt(const void *a, const void *b){
  if (*(size_t *)a > *(size_t *)b){
    return 1;
  }else if (*(size_t *)a < *(size_t *)b){
    return -1;
  }else{
    return 0;
  }
}
