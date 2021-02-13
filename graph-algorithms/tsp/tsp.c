/**
   tsp.c

   Dynamic programming version of an exact solution of TSP without revisiting
   and with generic weights, including negative weights, in O(2^n n^2)
   assymptotic runtime, where n is the number of vertices in a tour.

   A bit array representation provides time and space efficient set
   operations.

   TODO
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

typedef struct{
  size_t ix; //index of the set element with a single set bit
  size_t bit; //set element with a single set bit
} ibit_t;

static const size_t NR = SIZE_MAX; //not reached as index
static const size_t SET_ELT_SIZE = sizeof(size_t);
static const size_t SET_ELT_BIT_COUNT = sizeof(size_t) * 8;

//set operations based on a bit array representation
static void set_init(ibit_t *ibit, size_t n);
static size_t *set_member(const ibit_t *ibit, const size_t * set);
static void set_union(const ibit_t *ibit, size_t * set);

//auxiliary functions
static void build_next(const adj_lst_t *a,
		       ht_mul_uint64_t *ht,
		       stack_t *prev_s,
		       stack_t *next_s,
		       void (*add_wt)(void *, const void *, const void *),
		       int (*cmp_wt)(const void *, const void *));
static size_t *vt_ptr(const size_t *vts, size_t i);
static void *wt_ptr(const void *wts, size_t i, size_t wt_size);
static void rdc_key(void *t, const void *s);

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
  size_t wt_size = a->wt_size;
  size_t set_count, set_size;
  size_t u, v;
  size_t *prev_set = NULL;
  void *wt_buf = NULL;
  stack_t prev_s, next_s;
  ht_mul_uint64_t ht;
  set_count = a->num_vts / SET_ELT_BIT_COUNT;
  if (a->num_vts % SET_ELT_BIT_COUNT){
    set_count++;
  }
  set_count += 2; //+ size_t representation of the last vertex and NR
  set_size = set_count * SET_ELT_SIZE;
  prev_set = calloc_perror(set_size, 1);
  wt_buf = malloc_perror(wt_size);
  prev_set[0] = start;
  prev_set[set_count - 1] = NR;
  stack_init(&prev_s, 1, set_size, NULL);
  stack_push(&prev_s, prev_set);
  ht_mul_uint64_init(&ht, set_size, wt_size, 0.2, rdc_key, NULL);
  memset(dist, 0, wt_size);
  ht_mul_uint64_insert(&ht, prev_set, dist);
  for (size_t i = 0; i < a->num_vts - 1; i++){
    stack_init(&next_s, 1, set_size, NULL);
    build_next(a, &ht, &prev_s, &next_s, add_wt, cmp_wt);
    stack_free(&prev_s);
    prev_s = next_s;
    if (prev_s.num_elts == 0){
      //no progress made
      stack_free(&prev_s);
      ht_mul_uint64_free(&ht);
      free(prev_set);
      free(wt_buf);
      prev_set = NULL;
      wt_buf = NULL;
      return 1;
    }
  }
  //compute the return to start
  while (prev_s.num_elts > 0){
    stack_pop(&prev_s, prev_set);
    u = prev_set[0];
    for (size_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      if (v == start){
	add_wt(wt_buf,
	       ht_mul_uint64_search(&ht, prev_set),
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
  ht_mul_uint64_free(&ht);
  free(prev_set);
  free(wt_buf);
  prev_set = NULL;
  wt_buf = NULL;
  if (!final_dist_updated && a->num_vts > 1) return 1;
  return 0;
}

/**
   Builds reachable sets from previous sets and updates a hash table
   mapping a set to a distance. 
 */
static void build_next(const adj_lst_t *a,
		       ht_mul_uint64_t *ht,
		       stack_t *prev_s,
		       stack_t *next_s,
		       void (*add_wt)(void *, const void *, const void *),
		       int (*cmp_wt)(const void *, const void *)){
  size_t wt_size = a->wt_size;
  size_t set_size = ht->key_size;
  size_t u, v;
  size_t *prev_set = NULL, *next_set = NULL, *set_elt = NULL;
  void *prev_wt = NULL, *next_wt = NULL, *sum_wt = NULL;
  ibit_t ibit;
  prev_set = malloc_perror(set_size);
  next_set = malloc_perror(set_size);
  prev_wt = malloc_perror(wt_size);
  sum_wt = malloc_perror(wt_size);
  while (prev_s->num_elts > 0){
    stack_pop(prev_s, prev_set);
    ht_mul_uint64_remove(ht, prev_set, prev_wt);
    u = prev_set[0];
    for (size_t j = 0; j < a->vts[u]->num_elts; j++){
      v = *vt_ptr(a->vts[u]->elts, j);
      set_init(&ibit, v);
      set_elt = set_member(&ibit, &prev_set[1]);
      if (set_elt == NULL){
	set_init(&ibit, u);
	next_set[0] = v;
	memcpy(&next_set[1], &prev_set[1], set_size - sizeof(size_t));
	set_union(&ibit, &next_set[1]);
	add_wt(sum_wt, prev_wt, wt_ptr(a->wts[u]->elts, j, wt_size));
	next_wt = ht_mul_uint64_search(ht, next_set);
	if (next_wt == NULL){
	  ht_mul_uint64_insert(ht, next_set, sum_wt);
	  stack_push(next_s, next_set);
	}else if (cmp_wt(next_wt, sum_wt) > 0){
	  ht_mul_uint64_insert(ht, next_set, sum_wt);
	}
      }
    }
  }
  free(prev_set);
  free(next_set);
  free(sum_wt);
  free(prev_wt);
  prev_set = NULL;
  next_set = NULL;
  sum_wt = NULL;
  prev_wt = NULL;
}

/** Helper functions */

static void set_init(ibit_t *ibit, size_t n){
  ibit->ix = n / (sizeof(size_t) * 8);
  ibit->bit = 1;
  ibit->bit <<= n % (sizeof(size_t) * 8);
}

static size_t *set_member(const ibit_t *ibit, const size_t * set){
  if (set[ibit->ix] & ibit->bit){
    return (size_t *)(&set[ibit->ix]);
  }else{
    return NULL;
  }
}

static void set_union(const ibit_t *ibit, size_t * set){
  set[ibit->ix] |= ibit->bit;
}

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
