/**
   heap-test.c

   Tests of a generic dynamically allocated (min) heap with a hash table
   parameter across i) division- and mutliplication-based hash tables,
   ii) contiguous and noncontiguous elements, and iii) priority types.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "heap.h"
#include "ht-div.h"
#include "ht-mul.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

const int C_PTY_TYPES_COUNT = 3;
const char *C_PTY_TYPES[3] = {
  "size_t", "double", "long double"};
const size_t C_PTY_SIZES[3] = {
  sizeof(size_t), sizeof(double), sizeof(long double)};

int cmp_uint(const void *a, const void *b);
int cmp_double(const void *a, const void *b);
int cmp_long_double(const void *a, const void *b);
void new_uint(void *a, size_t val);
void new_double(void *a, size_t val);
void new_long_double(void *a, size_t val);

int (* const C_CMP_PTY_ARR[3])(const void *, const void *) ={
  cmp_uint, cmp_double, cmp_long_double};
void (* const C_NEW_PTY_ARR[3])(void *, size_t) = {
  new_uint, new_double, new_long_double};

float C_ALPHA_DIV = 1.0;
float C_ALPHA_MUL = 0.4;

void push_pop_free_pty_types(size_t n,
			     size_t pty_size,
			     size_t elt_size,
			     const heap_ht_t *ht,
			     int (*cmp_pty)(const void *, const void *),
			     int (*cmp_elt)(const void *, const void *),
			     void (*new_pty)(void *, size_t),
			     void (*new_elt)(void *, size_t),
			     void (*free_elt)(void *));
void update_search_pty_types(size_t n,
			     size_t pty_size,
			     size_t elt_size,
			     const heap_ht_t *ht,
			     int (*cmp_pty)(const void *, const void *),
			     int (*cmp_elt)(const void *, const void *),
			     void (*new_pty)(void *, size_t),
			     void (*new_elt)(void *, size_t),
			     void (*free_elt)(void *));
void print_test_result(int res);

/**
   Run heap_{push, pop, free} and heap_{update, search} tests with division-
   and mutliplication-based hash tables on size_t elements across
   priority types. A pointer to an element is passed as elt in heap_push
   and the element is fully copied into a heap. NULL as free_elt is
   sufficient to free the heap.
*/

int cmp_uint(const void *a, const void *b){
  if (*(size_t *)a > *(size_t *)b){
    return 1;
  }else if(*(size_t *)a < *(size_t *)b){
    return -1;
  }else{
    return 0;
  }
}

int cmp_double(const void *a, const void *b){
  if (*(double *)a > *(double *)b){
    return 1;
  }else if(*(double *)a < *(double *)b){
    return -1;
  }else{
    return 0;
  }
}

int cmp_long_double(const void *a, const void *b){
  if (*(long double *)a > *(long double *)b){
    return 1;
  }else if(*(long double *)a < *(long double *)b){
    return -1;
  }else{
    return 0;
  }
}

void new_uint(void *a, size_t val){
  size_t *s = a;
  *s = val;
  s = NULL;
}

void new_double(void *a, size_t val){
  double *s = a;
  *s = val;
  s = NULL;
}

void new_long_double(void *a, size_t val){
  long double *s = a;
  *s = val;
  s = NULL;
}

typedef struct{
  float alpha;
} ht_div_context_t;

typedef struct{
  float alpha;
  size_t (*rdc_key)(const void *, size_t);
} ht_mul_context_t;

void ht_div_init_helper(ht_div_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context){
  ht_div_context_t *c = context;
  ht_div_init(ht, key_size, elt_size, c->alpha, free_elt);
}

void ht_mul_init_helper(ht_mul_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context){
  ht_mul_context_t * c = context;
  ht_mul_init(ht, key_size, elt_size, c->alpha, c->rdc_key, free_elt);
}

/**
   Runs a heap_{push, pop, free} test with a ht_div_t hash table on
   size_t elements across priority types.
*/
void run_push_pop_free_div_uint_test(int pow_ins){
  int i;
  size_t n;
  ht_div_t ht_div;
  ht_div_context_t context;
  heap_ht_t ht;
  n = pow_two(pow_ins);
  context.alpha = C_ALPHA_DIV;
  ht.ht = &ht_div;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_init_helper;
  ht.insert = (heap_ht_insert)ht_div_insert;
  ht.search = (heap_ht_search)ht_div_search;
  ht.remove = (heap_ht_remove)ht_div_remove;
  ht.free = (heap_ht_free)ht_div_free;
  printf("Run a heap_{push, pop, free} test with a ht_div_t "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   TOLU(n), C_PTY_TYPES[i]);
    push_pop_free_pty_types(n,
			    C_PTY_SIZES[i],
			    sizeof(size_t),
			    &ht,
			    C_CMP_PTY_ARR[i],
			    cmp_uint,
			    C_NEW_PTY_ARR[i],
			    new_uint,
			    NULL);
  }
}

/**
   Runs a heap_{update, search} test with a ht_div_t hash table on
   size_t elements across priority types.
*/
void run_update_search_div_uint_test(int pow_ins){
  int i;
  size_t n;
  ht_div_t ht_div;
  ht_div_context_t context;
  heap_ht_t ht;
  n = pow_two(pow_ins);
  context.alpha = C_ALPHA_DIV;
  ht.ht = &ht_div;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_init_helper;
  ht.insert = (heap_ht_insert)ht_div_insert;
  ht.search = (heap_ht_search)ht_div_search;
  ht.remove = (heap_ht_remove)ht_div_remove;
  ht.free = (heap_ht_free)ht_div_free;
  printf("Run a heap_{update, search} test with a ht_div_t "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   TOLU(n), C_PTY_TYPES[i]);
    update_search_pty_types(n,
			    C_PTY_SIZES[i],
			    sizeof(size_t),
			    &ht,
			    C_CMP_PTY_ARR[i],
			    cmp_uint,
			    C_NEW_PTY_ARR[i],
			    new_uint,
			    NULL);
  }
}

/**
   Runs a heap_{push, pop, free} test with a ht_mul_t hash table on
   size_t elements across priority types.
*/
void run_push_pop_free_mul_uint_test(int pow_ins){
  int i;
  size_t n;
  ht_mul_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t ht;
  n = pow_two(pow_ins);
  context.alpha = C_ALPHA_MUL;
  context.rdc_key = NULL;
  ht.ht = &ht_mul;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_insert;
  ht.search = (heap_ht_search)ht_mul_search;
  ht.remove = (heap_ht_remove)ht_mul_remove;
  ht.free = (heap_ht_free)ht_mul_free;
  printf("Run a heap_{push, pop, free} test with a ht_mul_t "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   TOLU(n), C_PTY_TYPES[i]);
    push_pop_free_pty_types(n,
			    C_PTY_SIZES[i],
			    sizeof(size_t),
			    &ht,
			    C_CMP_PTY_ARR[i],
			    cmp_uint,
			    C_NEW_PTY_ARR[i],
			    new_uint,
			    NULL);
  }
}

/**
   Runs a heap_{update, search} test with a ht_mul_t hash table on
   size_t elements across priority types.
*/
void run_update_search_mul_uint_test(int pow_ins){
  int i;
  size_t n;
  ht_mul_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t ht;
  n = pow_two(pow_ins);
  context.alpha = C_ALPHA_MUL;
  context.rdc_key = NULL;
  ht.ht = &ht_mul;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_insert;
  ht.search = (heap_ht_search)ht_mul_search;
  ht.remove = (heap_ht_remove)ht_mul_remove;
  ht.free = (heap_ht_free)ht_mul_free;
  printf("Run a heap_{update, search} test with a ht_mul_t "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   TOLU(n), C_PTY_TYPES[i]);
    update_search_pty_types(n,
			    C_PTY_SIZES[i],
			    sizeof(size_t),
			    &ht,
			    C_CMP_PTY_ARR[i],
			    cmp_uint,
			    C_NEW_PTY_ARR[i],
			    new_uint,
			    NULL);
  }
}

/**
   Run heap_{push, pop, free} and heap_{update, search} tests with division-
   and mutliplication-based hash tables on noncontiguous uint_ptr_t
   elements across priority types. A pointer to a pointer to an element is
   passed as elt in heap_push, and the pointer to the element is copied into
   a heap. An element-specific free_elt is necessary to free the heap.
*/

typedef struct{
  size_t *val;
} uint_ptr_t;

int cmp_uint_ptr(const void *a, const void *b){
  if (*((*(uint_ptr_t **)a)->val) > *((*(uint_ptr_t **)b)->val)){
    return 1;
  }else if (*((*(uint_ptr_t **)a)->val) < *((*(uint_ptr_t **)b)->val)){
    return -1;
  }else{
    return 0;
  }
}

void new_uint_ptr(void *a, size_t val){
  uint_ptr_t **s = a;
  *s = malloc_perror(1, sizeof(uint_ptr_t));
  (*s)->val = malloc_perror(1, sizeof(size_t));
  *((*s)->val) = val;
  s = NULL;
}

void free_uint_ptr(void *a){
  uint_ptr_t **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

/**
   Runs a heap_{push, pop, free} test with a ht_div_t hash table on
   noncontiguous uint_ptr_t elements across priority types.
*/
void run_push_pop_free_div_uint_ptr_test(int pow_ins){
  int i;
  size_t n;
  ht_div_t ht_div;
  ht_div_context_t context;
  heap_ht_t ht;
  n = pow_two(pow_ins);
  context.alpha = C_ALPHA_DIV;
  ht.ht = &ht_div;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_init_helper;
  ht.insert = (heap_ht_insert)ht_div_insert;
  ht.search = (heap_ht_search)ht_div_search;
  ht.remove = (heap_ht_remove)ht_div_remove;
  ht.free = (heap_ht_free)ht_div_free;
  printf("Run a heap_{push, pop, free} test with a ht_div_t "
	 "hash table on noncontiguous uint_ptr_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   TOLU(n), C_PTY_TYPES[i]);
    push_pop_free_pty_types(n,
			    C_PTY_SIZES[i],
			    sizeof(uint_ptr_t *),
			    &ht,
			    C_CMP_PTY_ARR[i],
			    cmp_uint_ptr,
			    C_NEW_PTY_ARR[i],
			    new_uint_ptr,
			    free_uint_ptr);
  }
}

/**
   Runs a heap_{update, search} test with a ht_div_t hash table on
   noncontiguous uint_ptr_t elements across priority types.
*/
void run_update_search_div_uint_ptr_test(int pow_ins){
  int i;
  size_t n;
  ht_div_t ht_div;
  ht_div_context_t context;
  heap_ht_t ht;
  n = pow_two(pow_ins);
  context.alpha = C_ALPHA_DIV;
  ht.ht = &ht_div;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_init_helper;
  ht.insert = (heap_ht_insert)ht_div_insert;
  ht.search = (heap_ht_search)ht_div_search;
  ht.remove = (heap_ht_remove)ht_div_remove;
  ht.free = (heap_ht_free)ht_div_free;
  printf("Run a heap_{update, search} test with a ht_div_t "
	 "hash table on noncontiguous uint_ptr_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   TOLU(n), C_PTY_TYPES[i]);
    update_search_pty_types(n,
			    C_PTY_SIZES[i],
			    sizeof(uint_ptr_t *),
			    &ht,
			    C_CMP_PTY_ARR[i],
			    cmp_uint_ptr,
			    C_NEW_PTY_ARR[i],
			    new_uint_ptr,
			    free_uint_ptr);
  }
}

/**
   Runs a heap_{push, pop, free} test with a ht_mul_t hash table on
   noncontiguous uint_ptr_t elements across priority types.
*/
void run_push_pop_free_mul_uint_ptr_test(int pow_ins){
  int i;
  size_t n;
  ht_mul_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t ht;
  n = pow_two(pow_ins);
  context.alpha = C_ALPHA_MUL;
  context.rdc_key = NULL;
  ht.ht = &ht_mul;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_insert;
  ht.search = (heap_ht_search)ht_mul_search;
  ht.remove = (heap_ht_remove)ht_mul_remove;
  ht.free = (heap_ht_free)ht_mul_free;
  printf("Run a heap_{push, pop, free} test with a ht_mul_t "
	 "hash table on noncontiguous uint_ptr_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   TOLU(n), C_PTY_TYPES[i]);
    push_pop_free_pty_types(n,
			    C_PTY_SIZES[i],
			    sizeof(uint_ptr_t *),
			    &ht,
			    C_CMP_PTY_ARR[i],
			    cmp_uint_ptr,
			    C_NEW_PTY_ARR[i],
			    new_uint_ptr,
			    free_uint_ptr);
  }
}

/**
   Runs a heap_{update, search} test with a ht_mul_t hash table on
   noncontiguous uint_ptr_t elements across priority types.
*/
void run_update_search_mul_uint_ptr_test(int pow_ins){
  int i;
  size_t n;
  ht_mul_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t ht;
  n = pow_two(pow_ins);
  context.alpha = C_ALPHA_MUL;
  context.rdc_key = NULL;
  ht.ht = &ht_mul;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_insert;
  ht.search = (heap_ht_search)ht_mul_search;
  ht.remove = (heap_ht_remove)ht_mul_remove;
  ht.free = (heap_ht_free)ht_mul_free;
  printf("Run a heap_{update, search} test with a ht_mul_t "
	 "hash table on noncontiguous uint_ptr_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   TOLU(n), C_PTY_TYPES[i]);
    update_search_pty_types(n,
			    C_PTY_SIZES[i],
			    sizeof(uint_ptr_t *),
			    &ht,
			    C_CMP_PTY_ARR[i],
			    cmp_uint_ptr,
			    C_NEW_PTY_ARR[i],
			    new_uint_ptr,
			    free_uint_ptr);
  }
}

/** 
   Helper functions for heap_{push, pop, free} tests.
*/

void push_incr_ptys_elts(heap_t *h,
			 int *res,
			 size_t pty_size,
			 void **elts,
			 size_t count,
			 void (*new_pty)(void *, size_t)){
  size_t i, first_half_count = count / 2;
  size_t n = h->num_elts;
  void *pty = NULL;
  clock_t t;
  pty = malloc_perror(1, pty_size);
  t = clock();
  for (i = 0; i < first_half_count; i++){
    new_pty(pty, i);
    heap_push(h, pty, elts[i]);
  }
  t = clock() - t;
  printf("\t\tpush 1/2 elements, incr. priorities:           "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + first_half_count);
  t = clock();
  for (i = first_half_count; i < count; i++){
    new_pty(pty, i);
    heap_push(h, pty, elts[i]);
  }
  t = clock() - t;
  printf("\t\tpush residual elements, incr. priorities:      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + count);
  free(pty);
  pty = NULL;
}

void push_decr_ptys_elts(heap_t *h,
			 int *res,
			 size_t pty_size,
			 void **elts,
			 size_t count,
			 void (*new_pty)(void *, size_t)){
  size_t i, first_half_count = count / 2;
  size_t n = h->num_elts;
  void *pty = NULL;
  clock_t t;
  pty = malloc_perror(1, pty_size);
  t = clock();
  for (i = 0; i < first_half_count; i++){
    new_pty(pty, count - i - 1);
    heap_push(h, pty, elts[count - i - 1]);
  }
  t = clock() - t;
  printf("\t\tpush 1/2 elements, decr. priorities:           "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + first_half_count);
  t = clock();
  for (i = first_half_count; i < count; i++){
    new_pty(pty, count - i - 1);
    heap_push(h, pty, elts[count - i - 1]);
  }
  t = clock() - t;
  printf("\t\tpush residual elements, decr. priorities:      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + count);
  free(pty);
  pty = NULL;
}

void pop_ptys_elts(heap_t *h,
		   int *res,
		   size_t pty_size,
		   size_t elt_size,
		   void **elts,
		   size_t count,
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *)){
  size_t i, first_half_count = count / 2;
  size_t n = h->num_elts;
  void *pty_prev = NULL, *pty_cur = NULL, *elt = NULL;
  clock_t t;
  pty_prev = malloc_perror(1, pty_size);
  pty_cur = malloc_perror(1, pty_size);
  elt = malloc_perror(1, elt_size);
  t = clock();
  for (i = 0; i < first_half_count; i++){
    if (i == 0){
      heap_pop(h, pty_cur, elt);
      *res *= (cmp_elt(elt, elts[i]) == 0);
    }else{
      heap_pop(h, pty_cur, elt);
      *res *= (cmp_pty(pty_prev, pty_cur) <= 0);
      *res *= (cmp_elt(elt, elts[i]) == 0);
    }
    memcpy(pty_prev, pty_cur, pty_size); 
  }
  t = clock() - t;
  printf("\t\tpop 1/2 elements:                              "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n - first_half_count);
  t = clock();
  for (i = first_half_count; i < count; i++){
    if (i == 0){
      heap_pop(h, pty_cur, elt);
      *res *= (cmp_elt(elt, elts[i]) == 0);
    }else{
      heap_pop(h, pty_cur, elt);
      *res *= (cmp_pty(pty_prev, pty_cur) <= 0);
      *res *= (cmp_elt(elt, elts[i]) == 0);
    }
    memcpy(pty_prev, pty_cur, pty_size); 
  }
  t = clock() - t;
  printf("\t\tpop residual elements:                         "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n - count);
  free(pty_prev);
  free(pty_cur);
  free(elt);
  pty_prev = NULL;
  pty_cur = NULL;
  elt = NULL;
}

void free_heap(heap_t *h){
  clock_t t;
  t = clock();
  heap_free(h);
  t = clock() - t;
  printf("\t\tfree time:                                     "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

void push_pop_free_pty_types(size_t n,
			     size_t pty_size,
			     size_t elt_size,
			     const heap_ht_t *ht,
			     int (*cmp_pty)(const void *, const void *),
			     int (*cmp_elt)(const void *, const void *),
			     void (*new_pty)(void *, size_t),
			     void (*new_elt)(void *, size_t),
			     void (*free_elt)(void *)){
  int res = 1;
  size_t init_count = 1;
  size_t i;
  void **elts = NULL;
  heap_t h;
  elts = malloc_perror(n, sizeof(void *));
  for (i = 0; i < n; i++){
    elts[i] = malloc_perror(1, elt_size);
    new_elt(elts[i], i);
  }
  heap_init(&h, init_count, pty_size, elt_size, ht, cmp_pty, free_elt);
  push_incr_ptys_elts(&h, &res, pty_size, elts, n, new_pty);
  pop_ptys_elts(&h, &res, pty_size, elt_size, elts, n, cmp_pty, cmp_elt);
  push_decr_ptys_elts(&h, &res, pty_size, elts, n, new_pty);
  pop_ptys_elts(&h, &res, pty_size, elt_size, elts, n, cmp_pty, cmp_elt);
  push_incr_ptys_elts(&h, &res, pty_size, elts, n, new_pty);
  free_heap(&h);
  printf("\t\torder correctness:                             ");
  print_test_result(res);
  for (i = 0; i < n; i++){
    free(elts[i]);
  }
  free(elts);
  elts = NULL;
}

/** 
   Helper functions for heap_{update, search} tests.
*/

void push_rev_incr_ptys_elts(heap_t *h,
			     int *res,
			     size_t pty_size,
			     void **elts,
			     size_t count,
			     void (*new_pty)(void *, size_t)){
  size_t i, first_half_count = count / 2;
  size_t n = h->num_elts;
  void *pty = NULL;
  clock_t t;
  pty = malloc_perror(1, pty_size);
  t = clock();
  for (i = 0; i < first_half_count; i++){
    new_pty(pty, i);
    heap_push(h, pty, elts[count - i - 1]);
  }
  t = clock() - t;
  printf("\t\tpush 1/2 elements, rev. incr. priorities:      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + first_half_count);
  t = clock();
  for (i = first_half_count; i < count; i++){
    new_pty(pty, i);
    heap_push(h, pty, elts[count - i - 1]);
  }
  t = clock() - t;
  printf("\t\tpush residual elements, rev. incr. priorities: "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + count);
  free(pty);
  pty = NULL;
}

void update_rev_ptys_elts(heap_t *h,
			  int *res,
			  size_t pty_size,
			  void **elts,
			  size_t count,
			  void (*new_pty)(void *, size_t)){
  size_t i, first_half_count = count / 2;
  size_t n = h->num_elts;
  void *pty = malloc_perror(1, pty_size);
  clock_t t;
  t = clock();
  for (i = 0; i < first_half_count; i++){
    new_pty(pty, i);
    heap_update(h, pty, elts[i]);
  }
  t = clock() - t;
  printf("\t\tupdate 1/2 elements:                           "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n);
  t = clock();
  for (i = first_half_count; i < count; i++){
    new_pty(pty, i);
    heap_update(h, pty, elts[i]);
  }
  t = clock() - t;
  printf("\t\tupdate residual elements:                      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n);
  free(pty);
  pty = NULL;
}

void search_ptys_elts(heap_t *h,
		      int *res,
		      size_t pty_size,
		      size_t elt_size,
		      void **elts,
		      size_t count){
  size_t n = h->num_elts;
  size_t i;
  void *pty = NULL, *elt = NULL, *ptr = NULL;
  clock_t t;
  pty = malloc_perror(1, pty_size);
  elt = malloc_perror(1, elt_size);
  t = clock();
  for (i = 0; i < count; i++){
    ptr = heap_search(h, elts[i]);
    *res *= (ptr != NULL);
  }
  t = clock() - t;
  printf("\t\tin heap search:                                "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n);
  heap_pop(h, pty, elt);
  t = clock();
  for (i = 0; i < count; i++){
    ptr = heap_search(h, elt);
    *res *= (ptr == NULL);
  }
  t = clock() - t;
  printf("\t\tnot in heap search:                            "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n - 1);
  heap_push(h, pty, elt);
  *res *= (h->num_elts == n);
  free(pty);
  free(elt);
  pty = NULL;
  elt = NULL;
}

void update_search_pty_types(size_t n,
			     size_t pty_size,
			     size_t elt_size,
			     const heap_ht_t *ht,
			     int (*cmp_pty)(const void *, const void *),
			     int (*cmp_elt)(const void *, const void *),
			     void (*new_pty)(void *, size_t),
			     void (*new_elt)(void *, size_t),
			     void (*free_elt)(void *)){
  int res = 1;
  size_t init_count = 1;
  size_t i;
  void **elts = NULL;
  heap_t h;
  elts = malloc_perror(n, sizeof(void *));
  for (i = 0; i < n; i++){
    elts[i] = malloc_perror(1, elt_size);
    new_elt(elts[i], i);
  }
  heap_init(&h, init_count, pty_size, elt_size, ht, cmp_pty, free_elt);
  push_rev_incr_ptys_elts(&h, &res, pty_size, elts, n, new_pty);
  update_rev_ptys_elts(&h, &res, pty_size, elts, n, new_pty);
  search_ptys_elts(&h, &res, pty_size, elt_size, elts, n);
  pop_ptys_elts(&h, &res, pty_size, elt_size, elts, n, cmp_pty, cmp_elt);
  free_heap(&h);
  printf("\t\torder correctness:                             ");
  print_test_result(res);
  for (i = 0; i < n; i++){
    free(elts[i]);
  }
  free(elts);
  elts = NULL;
}

/**
   Prints a test result.
*/
void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  /* ht_div_t hash table */
  run_push_pop_free_div_uint_test(18);
  run_push_pop_free_div_uint_ptr_test(18);
  run_update_search_div_uint_test(18);
  run_update_search_div_uint_ptr_test(18);

  /* ht_mul_t hash table */
  run_push_pop_free_mul_uint_test(18);
  run_push_pop_free_mul_uint_ptr_test(18);
  run_update_search_mul_uint_test(18);
  run_update_search_mul_uint_ptr_test(18);
  return 0;
}
