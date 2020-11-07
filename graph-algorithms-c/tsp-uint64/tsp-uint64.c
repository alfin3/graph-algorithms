/**
   tsp-uint64.c

   Functions for running a dynamic programming version of an exact solution 
   of TSP with generic weights, including negative weights, in O(2^n n^2)
   assymptotic runtime, where n is the number of vertices in a tour. 

   A tour without revisiting must exist. In later versions, the non-existence 
   of a tour will be detected.
   
   The number of vertices is > 0 and bounded by 2^32 - 1. Edge weights 
   are of any basic type (e.g. char, int, double).

   Each computation of the next sets from a given set in build_next may be 
   implemented as a thread in a multithreaded version.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "tsp-uint64.h"
#include "graph-uint64.h"
#include "ht-mul-uint64.h"
#include "stack-uint64.h"

static uint64_t *vt_ptr(void *vts, uint64_t i);
static void *wt_ptr(void *wts, uint64_t i, int wt_size);
static void swap(void *a, void *b);;
static int cmp_key_fn(void *a, void *b);
static void rdc_key_fn(void *t, void *s);
static int cmp_vt_fn(const void *a, const void *b);

static const uint64_t nr = 0xffffffffffffffff; //not reached
static const uint64_t l_num_vts = 0xffffffff;

/**
   Determines the shortest tour length from start to start across all 
   vertices without revisiting.
*/
void tsp_uint64(adj_lst_uint64_t *a,
		uint64_t start,
		void *dist,
		void (*add_wt_fn)(void *, void *, void *),
		int (*cmp_wt_fn)(void *, void *)){
  assert(a->num_vts < l_num_vts);
  stack_uint64_t s_a, s_b;
  stack_uint64_t *prev_s = &s_a, *next_s = &s_b;
  ht_mul_uint64_t ht_a, ht_b;
  ht_mul_uint64_t *prev_ht = &ht_a, *next_ht = &ht_b;
  int vt_size = a->vt_size;
  int wt_size = a->wt_size;
  int set_size = 2; //including nr
  uint64_t u, v;
  uint64_t *prev_set = malloc(set_size * vt_size);
  assert(prev_set != NULL);
  void *wt_buf = malloc(wt_size);
  assert(wt_buf != NULL);
  prev_set[0] = start;
  prev_set[1] = nr;
  stack_uint64_init(prev_s, 1, set_size * vt_size, NULL);
  stack_uint64_push(prev_s, prev_set);
  ht_mul_uint64_init(prev_ht,
		     set_size * vt_size,
		     wt_size,
		     1.0,
		     cmp_key_fn,
		     rdc_key_fn,
		     NULL);
  for (uint64_t i = 0; i < a->num_vts - 1; i++){
    set_size++;
    stack_uint64_init(next_s, 1, set_size * vt_size, NULL);
    ht_mul_uint64_init(next_ht,
		       set_size * vt_size,
		       wt_size,
		       1.0,
		       cmp_key_fn, 
		       rdc_key_fn,
		       NULL);
    build_next(a, next_s, next_ht, prev_s, prev_ht, add_wt_fn, cmp_wt_fn);
    stack_uint64_free(prev_s);
    ht_mul_uint64_free(prev_ht);
    swap(&next_s, &prev_s);
    swap(&next_ht, &prev_ht);
  }
  //compute only the sets that allow returning to start
  prev_set = realloc((set_size - 1) * vt_size);
  assert(prev_set != NULL);
  while (prev_s->num_elts > 0){
    stack_uint64_pop(prev_s, prev_set);
    u = prev_set[0];
    for (uint64_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u], i);
      if (v == start){
	add_wt_fn(wt_buf,
		  ht_mul_uint64_search(prev_ht, prev_set),
		  wt_ptr(a->wts[u]->elts, i, wt_size));
	if (cmp_wt_fn(dist, wt_buf) > 0){
	  memcpy(dist, wt_buf, wt_size);
	}
      }
    }
  }
  free(prev_set);
  free(wt_buf);
  prev_set = NULL;
  wt_buf = NULL;
}

/**
   Builds reachable sets of size n + 1 from sets of size n. A set is an
   array of uint64_t vertices, where the first element is the last reached 
   vertex, and other elements are the previously reached sorted vertices. 
   A hashtable, maps a set to a distance. 
 */
static void build_next(adj_lst_uint64_t *a,
		       stack_uint64_t *next_s,
		       ht_mul_uint64_t *next_ht,
		       stack_uint64_t *prev_s,
		       ht_mul_uint64_t *prev_ht,
		       void (*add_wt_fn)(void *, void *, void *),
		       int (*cmp_wt_fn)(void *, void *)){
  int vt_size = a->vt_size;
  int wt_size = a->wt_size;
  int set_size = prev_ht->key_size / vt_size;
  uint64_t u, v;
  uint64_t *prev_set = malloc(set_size * vt_size);
  assert(prev_set != NULL);
  uint64_t *next_set = malloc((set_size + 1) * vt_size);
  assert(next_set != NULL);
  void *wt_buf = malloc(wt_size);
  assert(wt_buf != NULL);
  void *next_wt = NULL;
  while (prev_s->num_elts > 0){
    stack_uint64_pop(prev_s, prev_set);
    u = prev_set[0];
    for (uint64_t j = 0; j < a->vts[u]->num_elts; j++){
      v = *vt_ptr(a->vts[u], j);
      if (bsearch(&v, prev_set, set_size, vt_size, cmp_vt_fn) == NULL){
	next_set[0] = v;
	memcpy(next_set[1], prev_set, set_size * vt_size);
	qsort(next_set[1], set_size, vt_size, cmp_vt_fn);
	add_wt_fn(wt_buf,
		  ht_mul_uint64_search(prev_ht, prev_set),
		  wt_ptr(a->wts[u]->elts, j, wt_size));
	next_wt = ht_mul_uint64_search(next_ht, next_set);
	if (next_wt == NULL){
	  ht_mul_uint64_insert(next_ht, next_set, next_wt);
	  stack_uint64_push(next_s, next_set);
	}else if (cmp_wt_fn(next_wt, wt_buf) > 0){
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
static uint64_t *vt_ptr(void *vts, uint64_t i){
  return (uint64_t *)((char *)vts + i * sizeof(uint64_t));
}

/**
   Computes a pointer to an entry in an array of weights.
*/
static void *wt_ptr(void *wts, uint64_t i, int wt_size){
  return (void *)((char *)wts + i * wt_size);
}

/**
   Swaps pointers.
*/
static void swap(void *a, void *b){
  void *temp = *(void **)a;
  *(void **)a = *(void **)b;
  *(void **)b = temp;
}

/**
   Compares two nr terminated uint64_t arrays of the same size.
*/
static int cmp_key_fn(void *a, void *b){
  uint64_t *a_arr = (uint64_t *)a;
  uint64_t *b_arr = (uint64_t *)b;
  uint64_t i = 0;
  //slower than memcmp at this time
  while (a_arr[i] != nr){
    if (a_arr[i] > b_arr[i]){
      return 1;
    }else if (a_arr[i] < b_arr[i]){
      return -1;
    }
  }
  return 0;
}

/**
   Reduces size of an nr terminated array to uint64_t.
*/
static void rdc_key_fn(void *t, void *s){
  uint64_t r = 0;
  uint64_t i = 0;
  uint64_t n = 17858760364399553281U;
  uint64_t *s_arr = (uint64_t *)s;
  while (s_arr[i] != nr){
    r = sum_mod_uint64(r, s_arr[i], n);
    i++;
  }
  *(uint64_t *)t = r;
}


/**
   Compares two uint64_t vertices.
*/
static int cmp_vt_fn(const void *a, const void *b){
  uint64_t a_val = *(uint64_t *)a;
  uint64_t b_val = *(uint64_t *)b;
  if (a_val > b_val){
    return 1;
  }else if (a_val < b_val){
    return -1;
  }else{
    return 0;
  }
}
