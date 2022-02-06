/**
   tsp.c

   An exact solution of TSP without vertex revisiting on graphs with generic
   integer vertices and generic weights, including negative weights, and a
   hash table parameter.
   
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

   If E >> V and V < width of size_t, a default hash table may provide speed
   advantages by avoiding the computation of hash values. A non-default hash
   table may provide space advantages. A non-default hash may also enable
   computation with V that would exceed the available memory resources with
   the default hash table.

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
#include <limits.h>
#include "tsp.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

static const int C_FALSE = 0;
static const int C_TRUE = 1;

struct ht_def{
  size_t set_size;
  size_t wt_size;
  size_t num_vts;
  unsigned char *present;
  void *elts;
};

struct ibit{
  size_t ix;  /* index of the set element with a single set bit */
  size_t bit; /* set element with a single set bit */
};

static const size_t C_SZ_SIZE = sizeof(size_t);
static const size_t C_SZ_BIT = PRECISION_FROM_ULIMIT((size_t)-1);

/* set comparison; faster than memcmp on sets allocated with calloc */
static int cmp_set(const void *a, const void *b);
static size_t rdc_set(const void *a);

/* set operations based on a bit array representation */
static void ib_init(struct ibit *ib, size_t n);
static size_t *ib_set_member(const struct ibit *ib, const size_t *set);
static void ib_set_union(const struct ibit *ib, size_t *set);

/* default hash table operations */
static void ht_def_init(void *ht,
			size_t set_size,
			size_t wt_size,
			size_t num_vts);
static void ht_def_insert(void *ht, const void *s, const void *wt);
static void *ht_def_search(const void *ht, const void *s);
static void ht_def_remove(void *ht, const void *s, void *wt);
static void ht_def_free(void *ht);

/* auxiliary functions */
static void build_next(const struct adj_lst *a,
		       struct stack *prev_s,
		       struct stack *next_s,
		       const struct tsp_ht *tht,
		       size_t (*read_vt)(const void *),
		       int (*cmp_wt)(const void *, const void *),
		       void (*add_wt)(void *, const void *, const void *));
static void fprintf_stderr_exit(const char *s, int line);
static void *ptr(const void *block, size_t i, size_t size);

/**
   Copies to the block pointed to by dist the shortest tour length from 
   start to start across all vertices without revisiting, if a tour exists. 
   Returns 0 if a tour exists, otherwise returns 1.
   a           : pointer to an adjacency list with at least one vertex
   start       : start vertex for running the algorithm
   dist        : pointer to a preallocated block of size wt_size (wt_size
                 block) that equals to the size of a weight in the adjacency
                 list; the value is written into the block if tsp returns 0;
                 if the block pointed to by dist has no declared type and
                 tsp returns 0, then tsp sets the effective type of the
                 block to the type of a weight in the adjacency list by
                 writing a value of the type
   zero_wt     : pointer to a block of size wt_size with a zero value of
                 the type used to represent a distance
   tht         : - NULL pointer, if a default hash table is used for
                 set hashing operations; a default hash table contains an
                 array with a count that is equal to n * 2^n, where n is the
                 number of vertices in the adjacency list; the maximal n
                 in a default hash table is system-dependent and is less
                 than the width of size_t; if the allocation of a default
                 hash table fails for a given adjacency list, the program
                 terminates with an error message
                 - a pointer to a set of parameters specifying a hash table
                 used for set hashing operations
   read_vt     : reads the integer value of the type used to represent
                 vertices from the vt_size block pointed to by the argument
                 and returns a size_t value; unsigned char provides an often
                 sufficient and cache-efficient representation for vertices
   cmp_wt      : comparison function which returns a negative integer value
                 if the weight value pointed to by the first argument is
                 less than the weight value pointed to by the second, a
                 positive integer value if the weight value pointed to by
                 the first argument is greater than the weight value 
                 pointed to by the second, and zero integer value if the two
                 weight values are equal
   add_wt      : addition function which copies the sum of the weight values
                 pointed to by the second and third arguments to the
                 preallocated wt_size block pointed to by the first argument;
                 if the distribution of weights can result in an overflow,
                 the user may include an overflow test in the function or
                 use a provided _perror-suffixed function
*/
int tsp(const struct adj_lst *a,
	size_t start,
	void *dist,
	const void *wt_zero,
	const struct tsp_ht *tht,
	size_t (*read_vt)(const void *),
	int (*cmp_wt)(const void *, const void *),
	void (*add_wt)(void *, const void *, const void *)){
  int final_dist_updated = C_FALSE;
  size_t set_count = add_sz_perror(a->num_vts / C_SZ_BIT,
				   (a->num_vts % C_SZ_BIT > 0) + 2);
  size_t set_size = mul_sz_perror(set_count, C_SZ_SIZE);
  size_t u, v;
  size_t i;
  struct stack prev_s, next_s;
  struct ht_def ht_def;
  struct tsp_ht tht_def;
  const struct tsp_ht *thtp = NULL;
  const void *p = NULL, *p_start = NULL, *p_end = NULL;
  size_t * const prev_set = calloc_perror(1, set_size);
  void * const sum_wt = malloc_perror(1, a->wt_size);
  prev_set[0] = set_count;
  prev_set[1] = start;
  memcpy(dist, wt_zero, a->wt_size);
  stack_init(&prev_s, 1, set_size, NULL);
  stack_push(&prev_s, prev_set);
  if (tht == NULL){
    ht_def_init(&ht_def, set_size, a->wt_size, a->num_vts);
    tht_def.ht = &ht_def;
    tht_def.alpha_n = 0;
    tht_def.log_alpha_d = 0;
    tht_def.init = NULL;
    tht_def.align = NULL;
    tht_def.insert = ht_def_insert;
    tht_def.search = ht_def_search;
    tht_def.remove = ht_def_remove;
    tht_def.free = ht_def_free;
    thtp = &tht_def;
  }else{
    tht->init(tht->ht, set_size, wt_size,
	      0, tht->alpha_n, tht->log_alpha_d,
	      cmp_set, rdc_set, NULL, NULL);
    tht->align(tht->ht, wt_size);
    thtp = tht;
  }
  thtp->insert(thtp->ht, prev_set, dist);
  for (i = 0; i < a->num_vts - 1; i++){
    stack_init(&next_s, 1, set_size, NULL);
    build_next(a, set_size, wt_size, &prev_s, &next_s, thtp, add_wt, cmp_wt);
    stack_free(&prev_s);
    prev_s = next_s;
    if (prev_s.num_elts == 0){
      /* no progress made ; TODO add a goto since the same as the end*/
      stack_free(&prev_s);
      thtp->free(thtp->ht);
      free(prev_set);
      free(sum_wt);
      thtp = NULL;
      /* prev_set adn sum_vt cannot be dereferenced */
      return 1;
    }
  }
  /* compute the return to start */
  while (prev_s.num_elts > 0){
    stack_pop(&prev_s, prev_set);
    u = prev_set[1];
    p_start = a->vt_wts[u]->elts;
    p_end = (char *)p_start + a->vt_wts[u]->num_elts * a->pair_size;
    for (p = p_start; p != p_end; p = (char *)p + a->pair_size){
      v = read_vt(p);
      if (v == start){
	add_wt(sum_wt,
	       thtp->search(thtp->ht, prev_set),
	       (char *)p + a->offset);
	if (!final_dist_updated){
	  memcpy(dist, sum_wt, wt_size);
	  final_dist_updated = C_TRUE;
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
  /* prev_set and sum_wt cannot be dereferenced */
  thtp = NULL;
  if (!final_dist_updated && a->num_vts > 1) return 1;
  return 0;
}

/**
   Builds reachable sets from previous sets and updates a hash table
   mapping a set to a distance. 
 */
static void build_next(const struct adj_lst *a,
		       size_t set_size,
		       size_t wt_size,
		       struct stack *prev_s,
		       struct stack *next_s,
		       const struct tsp_ht *tht,
		       size_t (*read_vt)(const void *),
		       int (*cmp_wt)(const void *, const void *),
		       void (*add_wt)(void *, const void *, const void *)){
  size_t u, v;
  struct ibit ib;
  const void *p = NULL, *p_start = NULL, *p_end = NULL;
  const void *next_wt = NULL;
  /* in single blocks for cache-efficiency, TODO move to the caller */
  size_t * const prev_set = calloc_perror(2, set_size);
  size_t * const next_set = ptr(prev_set, 1, set_size);
  void * const prev_wt = malloc_perror(2, wt_size);
  void * const sum_wt = ptr(prev_wt, 1, wt_size);
  while (prev_s->num_elts > 0){
    stack_pop(prev_s, prev_set);
    tht->remove(tht->ht, prev_set, prev_wt);
    u = prev_set[1];
    p_start = a->vt_wts[u]->elts;
    p_end = (char *)p_start + a->vt_wts[u]->num_elts * a->pair_size;
    for (p = p_start; p != p_end; p = (char *)p + a->pair_size){
      v = read_vt(p); 
      ib_init(&ib, v);
      if (ib_set_member(&ib, &prev_set[1]) == NULL){
	memcpy(next_set, prev_set, set_size);
        next_set[1] = v;
        ib_init(&ib, u);
	ib_set_union(&ib, &next_set[1]);
	add_wt(sum_wt,
	       prev_wt,
	       (char *)p + a->offset);
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
  free(prev_wt);
  /* {prev, next}_set, {prev, sum}_wt, cannot be dereferenced */
}

/**
   Set comparison. Each set has two size_t values (count of size_t values,
   and last reached vertex) followed by a bit array representation of
   previously reached vertices.
*/

static int cmp_set(const void *a, const void *b){
  size_t i = 1; /* s[0] >= 2 for each set */
  size_t *sa = a;
  size_t *sb = b;
  while (i < sa[0] && sa[i] == sb[i]) i++;
  return (i != sa[0]);  
}

static size_t rdc_set(const void *a){
  size_t i;
  size_t ret = 0;
  size_t *sa = a;
  for (i = 1; i < sa[0]; i++) ret += sa[i]; /* with wrapping around */
  return ret;
}

/**
   Set operations based on a bit array representation.
*/

static void ib_init(struct ibit *ib, size_t n){
  ib->ix = n / C_SZ_BIT;
  ib->bit = 1;
  ib->bit <<= n % C_SZ_BIT;
}

static size_t *ib_set_member(const struct ibit *ib, const size_t *set){
  if (set[ib->ix] & ib->bit){
    return (size_t *)(&set[ib->ix]);
  }
  return NULL;
}

static void ib_set_union(const struct ibit *ib, size_t *set){
  set[ib->ix] |= ib->bit;
}

/**
   Default hash table operations.
*/

static void ht_def_init(void *ht,
			size_t set_size,
			size_t wt_size,
			size_t num_vts){
  struct ht_def *ht_def = ht;
  if (c->num_vts >= C_SZ_BIT){
    fprintf_stderr_exit("default hash table allocation failed", __LINE__);
  }
  ht->set_size = set_size;
  ht->wt_size = wt_size;
  ht->num_vts = num_vts;
  ht->present =
    calloc_perror(mul_sz_perror(num_vts, pow_two_perror(num_vts)),
		  sizeof(unsigned char));
  ht->wts =
    malloc_perror(mul_sz_perror(num_vts, pow_two_perror(num_vts)),
		  wt_size);
}

static void ht_def_insert(void *ht, const void *s, const void *wt){
  struct ht_def *ht_def = ht;
  const size_t *set = s;
  size_t ix = set[0] + ht->num_vts * set[1];
  ht->present[ix] = C_TRUE; /* convert to unsigned char from int */
  memcpy(ptr(ht->wts, ix, ht->wt_size),
	 wt,
	 ht->wt_size);
}

static void *ht_def_search(const void *ht, const void *s){
  struct ht_def *ht_def = ht;
  const size_t *set = s;
  size_t ix = set[0] + ht->num_vts * set[1];
  if (ht->present[ix]){
    return ptr(ht->wts, ix, ht->wt_size);
  }else{
    return NULL;
  }
}

static void ht_def_remove(void *ht, const void *s, void *wt){
  struct ht_def *ht_def = ht;
  const size_t *set = s;
  size_t ix = set[0] + ht->num_vts * set[1];
  ht->present[ix] = C_FALSE;
  memcpy(wt,
	 ptr(ht->wts, ix, ht->wt_size),
	 ht->wt_size);
}

static void ht_def_free(void *ht){
  struct ht_def *ht_def = ht;
  free(ht->present);
  free(ht->wts);
  ht->present = NULL;
  ht->wts = NULL;
}

/**
   Prints an error message and exits.
*/
static void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
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
