/**
   tsp-uint64.c

   Functions for running a dynamic programming version of an exact solution 
   to TSP with generic weights, including negative weights, in O(2^n n^2)
   assymptotic runtime, where n is the number of vertices in a tour. 

   It is assumed that a tour exists without revisiting.
   
   The number of vertices is > 0 and bounded by 2^32 - 1. Edge weights 
   are of any basic type (e.g. char, int, double).
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
static int cmp_vt_fn(const void *a, const void *b);
static int cmp_key_fn(void *a, void *b);
static void rdc_key_fn(void *t, void *s);


static const uint64_t nr = 0xffffffffffffffff; //not reached
static const uint64_t l_num_vts = 0xffffffff;

/**
   Determines the shortest tour length from start across all vertices, 
   without revisiting.
*/
void tsp_uint64(adj_lst_uint64_t *a,
		uint64_t start,
		void *dist,
		void (*init_wt_fn)(void *),
		void (*add_wt_fn)(void *, void *, void *),
		int (*cmp_wt_fn)(void *, void *)){
  stack_uint64_t *temp_s = NULL;
  ht_mul_uint64_t *temp_ht = NULL;
  stack_uint64_t *next_s = malloc(sizeof(stack_uint64_t));
  assert(next_s != NULL);
  stack_uint64_t *prev_s = malloc(sizeof(stack_uint64_t));
  assert(prev_s != NULL);
  ht_mul_uint64_t *next_ht = malloc(sizeof(ht_mul_uint64_t));
  assert(next_ht != NULL);
  ht_mul_uint64_t *prev_ht = malloc(sizeof(ht_mul_uint64_t));
  assert(prev_ht != NULL);
  uint64_t key_size = 2 * sizeof(uint64_t);
  stack_uint64_init(prev_s, 1, key_size, NULL);
  ht_mul_uint64_init(prev_ht,
		     key_size,
		     a->wt_size,
		     1.0,
		     cmp_key_fn,
		     rdc_key_fn,
		     NULL);
  for (uint64_t i = 0; i < a->num_vts - 1; i++){
    key_size += sizeof(uint64_t);
    stack_uint64_init(next_s, 1, key_size, NULL);
    ht_mul_uint64_init(next_ht,
		       key_size,
		       a->wt_size,
		       1.0,
		       cmp_key_fn, //while looped until nr
		       rdc_key_fn, //while looped until nr
		       NULL);
     build_next(a, next_s, next_ht, prev_s, prev_ht, add_wt_fn, cmp_wt_fn);
     stack_uint64_free(prev_s);
     ht_mul_uint64_free(prev_ht);
     temp_s = prev_s;
     temp_ht = prev_ht;
     prev_s = next_s;
     prev_ht = next_ht;
     next_s = temp_s;
     next_ht = temp_ht;
  }
}

/**
   Builds reachable sets of size n + 1 from sets of size n. A set is an
   array of uint64_t vertices, where the first element is the last reached 
   vertex, and other elements are the previously reached vertices. 
   A hashtable, maps a set to a distance.
 */
static void build_next(adj_lst_uint64_t *a,
		       stack_uint64_t *next_s,
		       ht_mul_uint64_t *next_ht,
		       stack_uint64_t *prev_s,
		       ht_mul_uint64_t *prev_ht,
		       void (*add_wt_fn)(void *, void *, void *),
		       int (*cmp_wt_fn)(void *, void *)){
  int vt_size = sizeof(uint64_t);
  int wt_size = prev_ht->elt_size;
  int prev_set_size = prev_ht->key_size / vt_size;
  int next_set_size = next_ht->key_size / vt_size;
  uint64_t *prev_set = malloc(prev_set_size * vt_size);
  assert(prev_set != NULL);
  uint64_t *next_set = malloc(next_set_size * vt_size);
  assert(next_set != NULL);
  uint64_t u, v;
  void *prev_wt = NULL;
  void *next_wt = malloc(wt_size);
  assert(next_wt != NULL);
  while (prev_s->num_elts > 0){
    stack_uint64_pop(prev_s, prev_set);
    u = prev_set[0];
    for (uint64_t j = 0; j < a->vts[u]->num_elts; j++){
      v = *vt_ptr(a->vts[u], j);
      if (bsearch(&v, prev_set, prev_set_size, vt_size, cmp_vt_fn) == NULL){
	next_set[0] = v;
	memcpy(next_set[1], prev_set, prev_set_size * vt_size);
	qsort(next_set[1], prev_set_size, vt_size, cmp_vt_fn);
	prev_wt = ht_mul_uint64_search(prev_ht, prev_set);
	add_wt_fn(next_wt, prev_wt, wt_ptr(a->wts[u]->elts, j, wt_size));
	if (ht_mul_uint64_search(next_ht, next_set) == NULL){
	  ht_mul_uint64_insert(next_ht, next_set, next_wt);
	  stack_uint64_push(next_s, next_set);
	}else if (*ht_mul_uint64_search(next_ht, next_set) > next_wt){
	  ht_mul_uint64_insert(next_ht, next_set, next_wt);
	}
      }
    }
  }
  free(prev_set);
  free(next_set);
  free(next_wt);
  prev_set = NULL;
  next_set = NULL;
  next_wt = NULL;
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
   Compare two nr terminated arrays of the same size of uint64_t vertices.
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
   Reduce size of a nr terminated arrays to uint64_t.
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
   Compare two uint64_t vertices.
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
 
