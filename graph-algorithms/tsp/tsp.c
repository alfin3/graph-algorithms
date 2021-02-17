/**
   tsp.c

   An exact solution of TSP without vertex revisiting on graphs with generic
   weights, including negative weights, with a hash table parameter.

   Vertices are indexed from 0. Edge weights are of any basic type (e.g.
   char, int, long, float, double), or are custom weights within a
   contiguous block (e.g. pair of 64-bit segments to address the potential
   overflow due to addition).
   
   The algorithm provides O(2^n n^2) assymptotic runtime, where n is the
   number of vertices in a tour, as well as tour existence detection. A bit
   array representation provides time and space efficient set membership and
   union operations over O(2^n) sets.

   The hash table parameter specifies a hash table used for set hashing
   operations, and enables the optimization of the associated space and time
   resources by choice of a hash table and its load factor upper bound.
   If NULL is passed as a hash table parameter value, a default hash table
   is used, which contains an array with a count that is equal to n * 2^n,
   where n is the number of vertices in the graph.   

   If E >> V and V < sizeof(size_t) * 8, a default hash table may provide
   speed advantages by avoiding the computation of hash values. If V is
   larger and the graph is sparse, a non-default hash table may provide space
   advantages.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tsp.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

typedef enum{
  FALSE,
  TRUE,
} boolean_t;

typedef struct{
  size_t num_vts;
} context_t;

typedef struct{
  size_t key_size;
  size_t elt_size;
  size_t num_vts;
  boolean_t *key_present;
  void *elts;
  void (*free_elt)(void *);
} ht_def_t;

typedef struct{
  size_t ix; //index of the set element with a single set bit
  size_t bit; //set element with a single set bit
} ibit_t;

static const size_t SET_ELT_SIZE = sizeof(size_t);
static const size_t SET_ELT_BIT_COUNT = sizeof(size_t) * 8;

//set operations based on a bit array representation
static void set_init(ibit_t *ibit, size_t n);
static size_t *set_member(const ibit_t *ibit, const size_t *set);
static void set_union(const ibit_t *ibit, size_t *set);

//default hash table operations
static void ht_def_init(ht_def_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context);
static void ht_def_insert(ht_def_t *ht, const size_t *key, const void *elt);
static void *ht_def_search(const ht_def_t *ht, const size_t *key);
static void ht_def_remove(ht_def_t *ht, const size_t *key, void *elt);
static void ht_def_free(ht_def_t *ht);

//auxiliary functions
static void build_next(const adj_lst_t *a,
		       stack_t *prev_s,
		       stack_t *next_s,
		       tsp_ht_t *tht,
		       void (*add_wt)(void *, const void *, const void *),
		       int (*cmp_wt)(const void *, const void *));
static size_t pow_two(size_t k);
static void fprintf_stderr_exit(const char *s, int line);

//functions for computing pointers
static size_t *vt_ptr(const size_t *vts, size_t i);
static void *wt_ptr(const void *wts, size_t i, size_t wt_size);
static void *elt_ptr(const void *elts, size_t i, size_t elt_size);

/**
   Copies to the block pointed to by dist the shortest tour length from 
   start to start across all vertices without revisiting, if a tour exists. 
   Returns 0 if a tour exists, otherwise returns 1.
   a           : pointer to an adjacency list with at least one vertex
   start       : start vertex for running the algorithm
   dist        : pointer to a preallocated block of the size of a weight in
                 the adjacency list
   tht         : - NULL pointer, if a default hash table is used for
                 set hashing operations; a default hash table contains an
                 array with a count that is equal to n * 2^n, where n is the
                 number of vertices in the adjacency list; the maximal n
                 in a default hash table is system-dependent and is less
                 than sizeof(size_t) * 8; if the allocation of a default hash
                 table fails, the program terminates with an error message
                 - a pointer to a set of parameters specifying a hash table
                 used for set hashing operations; the size of a hash key is 
                 k * (1 + lowest # k-sized blocks s.t. # bits >= # vertices),
                 where k = sizeof(size_t)
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
int tsp(const adj_lst_t *a,
	size_t start,
	void *dist,
	tsp_ht_t *tht,
	void (*add_wt)(void *, const void *, const void *),
	int (*cmp_wt)(const void *, const void *)){
  size_t wt_size = a->wt_size;
  size_t set_count, set_size;
  size_t u, v;
  size_t *prev_set = NULL;
  void *sum_wt = NULL;
  boolean_t final_dist_updated = FALSE;
  stack_t prev_s, next_s;
  ht_def_t ht_def;
  context_t context;
  tsp_ht_t tht_def, *thtp = tht;
  set_count = a->num_vts / SET_ELT_BIT_COUNT;
  if (a->num_vts % SET_ELT_BIT_COUNT){
    set_count++;
  }
  set_count++; //+ last reached vertex representation
  set_size = set_count * SET_ELT_SIZE;
  prev_set = calloc_perror(set_size, 1);
  sum_wt = malloc_perror(wt_size);
  prev_set[0] = start;
  memset(dist, 0, wt_size);
  stack_init(&prev_s, 1, set_size, NULL);
  stack_push(&prev_s, prev_set);
  if (thtp == NULL){
    context.num_vts = a->num_vts;
    tht_def.ht = &ht_def;
    tht_def.context = &context;
    tht_def.init = (tsp_ht_init)ht_def_init;
    tht_def.insert = (tsp_ht_insert)ht_def_insert;
    tht_def.search = (tsp_ht_search)ht_def_search;
    tht_def.remove = (tsp_ht_remove)ht_def_remove;
    tht_def.free = (tsp_ht_free)ht_def_free;
    thtp = &tht_def;
  }
  thtp->init(thtp->ht, set_size, wt_size, NULL, thtp->context);
  thtp->insert(thtp->ht, prev_set, dist);
  for (size_t i = 0; i < a->num_vts - 1; i++){
    stack_init(&next_s, 1, set_size, NULL);
    build_next(a, &prev_s, &next_s, thtp, add_wt, cmp_wt);
    stack_free(&prev_s);
    prev_s = next_s;
    if (prev_s.num_elts == 0){
      //no progress made
      stack_free(&prev_s);
      thtp->free(thtp->ht);
      free(prev_set);
      free(sum_wt);
      thtp = NULL;
      prev_set = NULL;
      sum_wt = NULL;
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
	add_wt(sum_wt,
	       thtp->search(thtp->ht, prev_set),
	       wt_ptr(a->wts[u]->elts, i, wt_size));
	if (!final_dist_updated){
	  memcpy(dist, sum_wt, wt_size);
	  final_dist_updated = TRUE;
	}else if (cmp_wt(dist, sum_wt) > 0){
	  memcpy(dist, sum_wt, wt_size);
	}
      }
    }
  }
  stack_free(&prev_s);
  thtp->free(thtp->ht);
  free(prev_set);
  free(sum_wt);
  thtp = NULL;
  prev_set = NULL;
  sum_wt = NULL;
  if (!final_dist_updated && a->num_vts > 1) return 1;
  return 0;
}

/**
   Builds reachable sets from previous sets and updates a hash table
   mapping a set to a distance. 
 */
static void build_next(const adj_lst_t *a,
		       stack_t *prev_s,
		       stack_t *next_s,
		       tsp_ht_t *tht,
		       void (*add_wt)(void *, const void *, const void *),
		       int (*cmp_wt)(const void *, const void *)){
  size_t wt_size = a->wt_size;
  size_t set_size = prev_s->elt_size;
  size_t u, v;
  size_t *prev_set = NULL, *next_set = NULL;
  void *prev_wt = NULL, *next_wt = NULL, *sum_wt = NULL;
  ibit_t ibit;
  prev_set = malloc_perror(set_size);
  next_set = malloc_perror(set_size);
  prev_wt = malloc_perror(wt_size);
  sum_wt = malloc_perror(wt_size);
  while (prev_s->num_elts > 0){
    stack_pop(prev_s, prev_set);
    tht->remove(tht->ht, prev_set, prev_wt);
    u = prev_set[0];
    for (size_t i = 0; i < a->vts[u]->num_elts; i++){
      v = *vt_ptr(a->vts[u]->elts, i);
      set_init(&ibit, v);
      if (set_member(&ibit, &prev_set[1]) == NULL){
	memcpy(next_set, prev_set, set_size);
        next_set[0] = v;
        set_init(&ibit, u);
	set_union(&ibit, &next_set[1]);
	add_wt(sum_wt,
	       prev_wt,
	       wt_ptr(a->wts[u]->elts, i, wt_size));
	next_wt = tht->search(tht->ht, next_set);
	if (next_wt == NULL){
	  tht->insert(tht->ht, next_set, sum_wt);
	  stack_push(next_s, next_set);
	}else if (cmp_wt(next_wt, sum_wt) > 0){
	  tht->insert(tht->ht, next_set, sum_wt);
	}
      }
    }
  }
  free(prev_set);
  free(next_set);
  free(prev_wt);
  free(sum_wt);
  prev_set = NULL;
  next_set = NULL;
  prev_wt = NULL;
  sum_wt = NULL;
}

/**
   Set operations based on a bit array representation.
*/

static void set_init(ibit_t *ibit, size_t n){
  ibit->ix = n / SET_ELT_BIT_COUNT;
  ibit->bit = 1;
  ibit->bit <<= n % SET_ELT_BIT_COUNT;
}

static size_t *set_member(const ibit_t *ibit, const size_t *set){
  if (set[ibit->ix] & ibit->bit){
    return (size_t *)(&set[ibit->ix]);
  }else{
    return NULL;
  }
}

static void set_union(const ibit_t *ibit, size_t *set){
  set[ibit->ix] |= ibit->bit;
}

/**
   Default hash table operations.
*/

static void ht_def_init(ht_def_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context){
  context_t *c = context;
  if (c->num_vts >= SET_ELT_BIT_COUNT ||
      pow_two(c->num_vts) > SIZE_MAX / c->num_vts / elt_size){
    fprintf_stderr_exit("default hash table allocation failed", __LINE__);
  }
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->num_vts = c->num_vts;
  ht->key_present = calloc_perror(c->num_vts * pow_two(c->num_vts),
				  sizeof(boolean_t));
  ht->elts = malloc_perror(c->num_vts * pow_two(c->num_vts) * elt_size);
  ht->free_elt = free_elt;
}

static void ht_def_insert(ht_def_t *ht, const size_t *key, const void *elt){
  size_t ix = key[0] + ht->num_vts * key[1];
  ht->key_present[ix] = TRUE;
  memcpy(elt_ptr(ht->elts, ix, ht->elt_size),
	 elt,
	 ht->elt_size);
}

static void *ht_def_search(const ht_def_t *ht, const size_t *key){
  size_t ix = key[0] + ht->num_vts * key[1];
  if (ht->key_present[ix]){
    return elt_ptr(ht->elts, ix, ht->elt_size);
  }else{
    return NULL;
  }
}

static void ht_def_remove(ht_def_t *ht, const size_t *key, void *elt){
  size_t ix = key[0] + ht->num_vts * key[1];
  ht->key_present[ix] = FALSE;
  memcpy(elt,
	 elt_ptr(ht->elts, ix, ht->elt_size),
	 ht->elt_size);
}

static void ht_def_free(ht_def_t *ht){
  if (ht->free_elt != NULL){
    for (size_t i = 0; i < ht->num_vts * pow_two(ht->num_vts); i++){
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

/**
   Returns the kth power of 2, where 0 <= k < SET_ELT_BIT_COUNT.
*/
static size_t pow_two(size_t k){
  size_t ret = 1;
  return ret << k;
}

/**
   Prints an error message and exits.
*/
static void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
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
   Computes a pointer to an entry in the array of elements in a default hash
   table.
*/
static void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
}
