/**
   heap-test.c

   Tests of a generic (min) heap with a hash table parameter across 
   i) division- and mutliplication-based hash tables, ii) contiguous and
   noncontiguous elements, and iii) priority types.

   The following command line arguments can be used to customize tests:
   heap-test
      [0, # bits in size_t - 1) : i s.t. # inserts = 2^i
      > 0 : a
      > 0 : b s.t. alpha of division hash table = a / b
      > 0 : c
      > 0 : d s.t. alpha of multiplication hash table = c / d < 1.0
      [0, 1] : on/off push pop free division hash table test
      [0, 1] : on/off update search division hash table test
      [0, 1] : on/off push pop free multiplication hash table test
      [0, 1] : on/off update search multiplication hash table test

   usage examples:
   ./heap-test
   ./heap-test 21
   ./heap-test 20 1 10
   ./heap-test 20 1 1
   ./heap-test 20 100 1
   ./heap-test 20 1 1 1 100 0 0
   ./heap-test 20 1 1 1 3 0 0
   ./heap-test 20 1 1 9 10 0 0

   heap-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 with the only requirement that CHAR_BIT * sizeof(size_t) is
   greater or equal to 16 and is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "heap.h"
#include "ht-div.h"
#include "ht-mul.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "heap-test \n"
  "[0, # bits in size_t - 1) : i s.t. # inserts = 2^i \n"
   "> 0 : a \n"
   "> 0 : b s.t. alpha of division hash table = a / b \n"
   "> 0 : c \n"
   "> 0 : d s.t. alpha of multiplication hash table = c / d < 1.0 \n"
   "[0, 1] : on/off push pop free division hash table test \n"
   "[0, 1] : on/off update search division hash table test \n"
   "[0, 1] : on/off push pop free multiplication hash table test \n"
   "[0, 1] : on/off update search multiplication hash table test \n";
const int C_ARGC_MAX = 10;
const size_t C_ARGS_DEF[9] = {14, 1, 1, 1, 3, 1, 1, 1, 1};
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);
const float C_ALPHA_MUL_MAX = 1.0;

/* tests */
const int C_PTY_TYPES_COUNT = 3;
const char *C_PTY_TYPES[3] = {"size_t", "double", "long double"};
const size_t C_PTY_SIZES[3] = {sizeof(size_t),
			       sizeof(double),
			       sizeof(long double)};

int cmp_uint(const void *a, const void *b);
int cmp_double(const void *a, const void *b);
int cmp_long_double(const void *a, const void *b);
void new_uint(void *a, size_t val);
void new_double(void *a, size_t val);
void new_long_double(void *a, size_t val);

int (*const C_CMP_PTY_ARR[3])(const void *, const void *) ={cmp_uint,
							    cmp_double,
							    cmp_long_double};
void (*const C_NEW_PTY_ARR[3])(void *, size_t) = {new_uint,
						  new_double,
						  new_long_double};
const size_t C_H_INIT_COUNT = 1;

void push_pop_free(size_t n,
		   size_t pty_size,
		   size_t elt_size,
		   const heap_ht_t *hht,
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *),
		   void (*new_pty)(void *, size_t),
		   void (*new_elt)(void *, size_t),
		   void (*free_elt)(void *));
void update_search(size_t n,
		   size_t pty_size,
		   size_t elt_size,
		   const heap_ht_t *hht,
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
void run_push_pop_free_div_uint_test(int log_ins, float alpha){
  int i;
  size_t n;
  ht_div_t ht_div;
  ht_div_context_t context;
  heap_ht_t hht;
  n = pow_two_perror(log_ins);
  context.alpha = alpha;
  hht.ht = &ht_div;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_div_init_helper;
  hht.insert = (heap_ht_insert)ht_div_insert;
  hht.search = (heap_ht_search)ht_div_search;
  hht.remove = (heap_ht_remove)ht_div_remove;
  hht.free = (heap_ht_free)ht_div_free;
  printf("Run a heap_{push, pop, free} test with a ht_div_t "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n), alpha, C_PTY_TYPES[i]);
    push_pop_free(n,
		  C_PTY_SIZES[i],
		  sizeof(size_t),
		  &hht,
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
void run_update_search_div_uint_test(int log_ins, float alpha){
  int i;
  size_t n;
  ht_div_t ht_div;
  ht_div_context_t context;
  heap_ht_t hht;
  n = pow_two_perror(log_ins);
  context.alpha = alpha;
  hht.ht = &ht_div;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_div_init_helper;
  hht.insert = (heap_ht_insert)ht_div_insert;
  hht.search = (heap_ht_search)ht_div_search;
  hht.remove = (heap_ht_remove)ht_div_remove;
  hht.free = (heap_ht_free)ht_div_free;
  printf("Run a heap_{update, search} test with a ht_div_t "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n), alpha, C_PTY_TYPES[i]);
    update_search(n,
		  C_PTY_SIZES[i],
		  sizeof(size_t),
		  &hht,
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
void run_push_pop_free_mul_uint_test(int log_ins, float alpha){
  int i;
  size_t n;
  ht_mul_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t hht;
  n = pow_two_perror(log_ins);
  context.alpha = alpha;
  context.rdc_key = NULL;
  hht.ht = &ht_mul;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_mul_init_helper;
  hht.insert = (heap_ht_insert)ht_mul_insert;
  hht.search = (heap_ht_search)ht_mul_search;
  hht.remove = (heap_ht_remove)ht_mul_remove;
  hht.free = (heap_ht_free)ht_mul_free;
  printf("Run a heap_{push, pop, free} test with a ht_mul_t "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n), alpha, C_PTY_TYPES[i]);
    push_pop_free(n,
		  C_PTY_SIZES[i],
		  sizeof(size_t),
		  &hht,
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
void run_update_search_mul_uint_test(int log_ins, float alpha){
  int i;
  size_t n;
  ht_mul_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t hht;
  n = pow_two_perror(log_ins);
  context.alpha = alpha;
  context.rdc_key = NULL;
  hht.ht = &ht_mul;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_mul_init_helper;
  hht.insert = (heap_ht_insert)ht_mul_insert;
  hht.search = (heap_ht_search)ht_mul_search;
  hht.remove = (heap_ht_remove)ht_mul_remove;
  hht.free = (heap_ht_free)ht_mul_free;
  printf("Run a heap_{update, search} test with a ht_mul_t "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n), alpha, C_PTY_TYPES[i]);
    update_search(n,
		  C_PTY_SIZES[i],
		  sizeof(size_t),
		  &hht,
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
void run_push_pop_free_div_uint_ptr_test(int log_ins, float alpha){
  int i;
  size_t n;
  ht_div_t ht_div;
  ht_div_context_t context;
  heap_ht_t hht;
  n = pow_two_perror(log_ins);
  context.alpha = alpha;
  hht.ht = &ht_div;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_div_init_helper;
  hht.insert = (heap_ht_insert)ht_div_insert;
  hht.search = (heap_ht_search)ht_div_search;
  hht.remove = (heap_ht_remove)ht_div_remove;
  hht.free = (heap_ht_free)ht_div_free;
  printf("Run a heap_{push, pop, free} test with a ht_div_t "
	 "hash table on noncontiguous uint_ptr_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n), alpha, C_PTY_TYPES[i]);
    push_pop_free(n,
		  C_PTY_SIZES[i],
		  sizeof(uint_ptr_t *),
		  &hht,
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
void run_update_search_div_uint_ptr_test(int log_ins, float alpha){
  int i;
  size_t n;
  ht_div_t ht_div;
  ht_div_context_t context;
  heap_ht_t hht;
  n = pow_two_perror(log_ins);
  context.alpha = alpha;
  hht.ht = &ht_div;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_div_init_helper;
  hht.insert = (heap_ht_insert)ht_div_insert;
  hht.search = (heap_ht_search)ht_div_search;
  hht.remove = (heap_ht_remove)ht_div_remove;
  hht.free = (heap_ht_free)ht_div_free;
  printf("Run a heap_{update, search} test with a ht_div_t "
	 "hash table on noncontiguous uint_ptr_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n), alpha, C_PTY_TYPES[i]);
    update_search(n,
		  C_PTY_SIZES[i],
		  sizeof(uint_ptr_t *),
		  &hht,
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
void run_push_pop_free_mul_uint_ptr_test(int log_ins, float alpha){
  int i;
  size_t n;
  ht_mul_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t hht;
  n = pow_two_perror(log_ins);
  context.alpha = alpha;
  context.rdc_key = NULL;
  hht.ht = &ht_mul;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_mul_init_helper;
  hht.insert = (heap_ht_insert)ht_mul_insert;
  hht.search = (heap_ht_search)ht_mul_search;
  hht.remove = (heap_ht_remove)ht_mul_remove;
  hht.free = (heap_ht_free)ht_mul_free;
  printf("Run a heap_{push, pop, free} test with a ht_mul_t "
	 "hash table on noncontiguous uint_ptr_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n), alpha, C_PTY_TYPES[i]);
    push_pop_free(n,
		  C_PTY_SIZES[i],
		  sizeof(uint_ptr_t *),
		  &hht,
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
void run_update_search_mul_uint_ptr_test(int log_ins, float alpha){
  int i;
  size_t n;
  ht_mul_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t hht;
  n = pow_two_perror(log_ins);
  context.alpha = alpha;
  context.rdc_key = NULL;
  hht.ht = &ht_mul;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_mul_init_helper;
  hht.insert = (heap_ht_insert)ht_mul_insert;
  hht.search = (heap_ht_search)ht_mul_search;
  hht.remove = (heap_ht_remove)ht_mul_remove;
  hht.free = (heap_ht_free)ht_mul_free;
  printf("Run a heap_{update, search} test with a ht_mul_t "
	 "hash table on noncontiguous uint_ptr_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n), alpha, C_PTY_TYPES[i]);
    update_search(n,
		  C_PTY_SIZES[i],
		  sizeof(uint_ptr_t *),
		  &hht,
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

void push_ptys_elts(heap_t *h,
		    int *res,
		    void **ptys,
		    void **elts,
		    size_t count){
  size_t i, half_count;
  size_t n = h->num_elts;
  clock_t t_first, t_second;
  half_count = count / 2;
  t_first = clock();
  for (i = 0; i < half_count; i++){
    heap_push(h, ptys[i], elts[i]);
  }
  t_first = clock() - t_first;
  *res *= (h->num_elts == n + half_count);
  t_second = clock();
  for (i = half_count; i < count; i++){
    heap_push(h, ptys[i], elts[i]);
  }
  t_second = clock() - t_second;
  printf("\t\tpush 1/2 elements:                           "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tpush residual elements:                      "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + count);
}

void push_rev_ptys_elts(heap_t *h,
			int *res,
			void **ptys,
			void **elts,
			size_t count){
  size_t i, half_count;
  size_t n = h->num_elts;
  clock_t t_first, t_second;
  half_count = count / 2;
  t_first = clock();
  for (i = 0; i < half_count; i++){
    heap_push(h, ptys[count - i - 1], elts[count - i - 1]);
  }
  t_first = clock() - t_first;
  *res *= (h->num_elts == n + half_count);
  t_second = clock();
  for (i = half_count; i < count; i++){
    heap_push(h, ptys[count - i - 1], elts[count - i - 1]);
  }
  t_second = clock() - t_second;
  printf("\t\tpush 1/2 elements, rev. pty order:           "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tpush residual elements, rev. pty order:      "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + count);
}

void pop_ptys_elts(heap_t *h,
		   int *res,
		   size_t pty_size,
		   size_t elt_size,
		   void **elts,
		   size_t count,
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *)){
  size_t i, half_count;
  size_t n = h->num_elts;
  void **pop_ptys = NULL, **pop_elts = NULL;
  clock_t t_first, t_second;
  half_count = count / 2;
  pop_ptys = malloc_perror(count, sizeof(void *));
  pop_elts = malloc_perror(count, sizeof(void *));
  for (i = 0; i < count; i++){
    pop_ptys[i] = malloc_perror(1, pty_size);
    pop_elts[i] = malloc_perror(1, elt_size);
  }
  t_first = clock();
  for (i = 0; i < half_count; i++){
    heap_pop(h, pop_ptys[i], pop_elts[i]);
  }
  t_first = clock() - t_first;
  *res *= (h->num_elts == n - half_count);
  t_second = clock();
  for (i = half_count; i < count; i++){
    heap_pop(h, pop_ptys[i], pop_elts[i]);
  }
  t_second = clock() - t_second;
  *res *= (h->num_elts == n - count);
  for (i = 0; i < count; i++){
    if (i == 0){
      *res *= (cmp_elt(pop_elts[i], elts[i]) == 0);
    }else{
      *res *= (cmp_pty(pop_ptys[i], pop_ptys[i - 1]) >= 0);
      *res *= (cmp_elt(pop_elts[i], elts[i]) == 0);
    }
  }
  printf("\t\tpop 1/2 elements:                            "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tpop residual elements:                       "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  for (i = 0; i < count; i++){
    free(pop_ptys[i]);
    free(pop_elts[i]); /* a non-contiguous element accessible from elts */ 
  }
  free(pop_ptys);
  free(pop_elts);
  pop_ptys = NULL;
  pop_elts = NULL;
}

void free_heap(heap_t *h){
  clock_t t;
  t = clock();
  heap_free(h);
  t = clock() - t;
  printf("\t\tfree time:                                   "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/** 
   Helper functions for heap_{update, search} tests.
*/

void push_ptys_rev_elts(heap_t *h,
			int *res,
			void **ptys,
			void **elts,
			size_t count){
  size_t i, half_count;
  size_t n = h->num_elts;
  clock_t t_first, t_second;
  half_count = count / 2;
  t_first = clock();
  for (i = 0; i < half_count; i++){
    heap_push(h, ptys[i], elts[count - i - 1]);
  }
  t_first = clock() - t_first;
  *res *= (h->num_elts == n + half_count);
  t_second = clock();
  for (i = half_count; i < count; i++){
    heap_push(h, ptys[i], elts[count - i - 1]);
  }
  t_second = clock() - t_second;
  printf("\t\tpush 1/2 elements, rev. elt order:           "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tpush residual elements, rev. elt order:      "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + count);
}

void update_ptys_elts(heap_t *h,
		      int *res,
		      void **ptys,
		      void **elts,
		      size_t count){
  size_t i, half_count = count / 2;
  size_t n = h->num_elts;
  clock_t t_first, t_second;
  t_first = clock();
  for (i = 0; i < half_count; i++){
    heap_update(h, ptys[i], elts[i]);
  }
  t_first = clock() - t_first;
  *res *= (h->num_elts == n);
  t_second = clock();
  for (i = half_count; i < count; i++){
    heap_update(h, ptys[i], elts[i]);
  }
  t_second = clock() - t_second;
  printf("\t\tupdate 1/2 elements:                         "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tupdate residual elements:                    "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n);
}

void search_ptys_elts(heap_t *h,
		      int *res,
		      void **elts,
		      void **not_heap_elts,
		      size_t count){
  size_t n = h->num_elts;
  size_t i;
  void *ptr = NULL;
  clock_t t_heap, t_not_heap;
  t_heap = clock();
  for (i = 0; i < count; i++){
    ptr = heap_search(h, elts[i]);
  }
  t_heap = clock() - t_heap;
  for (i = 0; i < count; i++){
    ptr = heap_search(h, elts[i]);
    *res *= (ptr != NULL);
  }
  *res *= (h->num_elts == n);
  t_not_heap = clock();
  for (i = 0; i < count; i++){
    ptr = heap_search(h, not_heap_elts[i]);
  }
  t_not_heap = clock() - t_not_heap;
  for (i = 0; i < count; i++){
    ptr = heap_search(h, not_heap_elts[i]);
    *res *= (ptr == NULL);
  }
  printf("\t\tin heap search:                              "
	 "%.4f seconds\n", (float)t_heap / CLOCKS_PER_SEC);
  printf("\t\tnot in heap search:                          "
	 "%.4f seconds\n", (float)t_not_heap / CLOCKS_PER_SEC);
}

/** 
   Helper functions for heap_{push, pop, free} and heap_{update, search} 
   tests that are used in the upper-level test routines.
*/

void push_pop_free(size_t n,
		   size_t pty_size,
		   size_t elt_size,
		   const heap_ht_t *hht,
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *),
		   void (*new_pty)(void *, size_t),
		   void (*new_elt)(void *, size_t),
		   void (*free_elt)(void *)){
  int res = 1;
  size_t i;
  void **ptys = NULL, **elts = NULL;
  heap_t h;
  ptys = malloc_perror(n, sizeof(void *));
  elts = malloc_perror(n, sizeof(void *));
  for (i = 0; i < n; i++){
    ptys[i] = malloc_perror(1, pty_size);
    elts[i] = malloc_perror(1, elt_size);
    new_pty(ptys[i], i);  /* priority must not decrease in value with i */
    new_elt(elts[i], i);
  }
  heap_init(&h, C_H_INIT_COUNT, pty_size, elt_size, hht, cmp_pty, free_elt);
  push_ptys_elts(&h, &res, ptys, elts, n);
  pop_ptys_elts(&h, &res, pty_size, elt_size, elts, n, cmp_pty, cmp_elt);
  push_rev_ptys_elts(&h, &res, ptys, elts, n);
  pop_ptys_elts(&h, &res, pty_size, elt_size, elts, n, cmp_pty, cmp_elt);
  push_ptys_elts(&h, &res, ptys, elts, n);
  free_heap(&h);
  printf("\t\torder correctness:                           ");
  print_test_result(res);
  for (i = 0; i < n; i++){
    free(ptys[i]);
    free(elts[i]); /* non-contiguous elements freed by free_heap */
  }
  free(ptys);
  free(elts);
  ptys = NULL;
  elts = NULL;
}

void update_search(size_t n,
		   size_t pty_size,
		   size_t elt_size,
		   const heap_ht_t *hht,
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *),
		   void (*new_pty)(void *, size_t),
		   void (*new_elt)(void *, size_t),
		   void (*free_elt)(void *)){
  int res = 1;
  size_t i;
  void **ptys = NULL, **elts = NULL, **not_heap_elts = NULL;
  heap_t h;
  ptys = malloc_perror(n, sizeof(void *));
  elts = malloc_perror(n, sizeof(void *));
  not_heap_elts = malloc_perror(n, sizeof(void *));
  for (i = 0; i < n; i++){
    ptys[i] = malloc_perror(1, pty_size);
    elts[i] = malloc_perror(1, elt_size);
    not_heap_elts[i] = malloc_perror(1, elt_size);
    new_pty(ptys[i], i);  /* priority must not decrease in value with i */
    new_elt(elts[i], i);
    new_elt(not_heap_elts[i], n + i);
  }
  heap_init(&h, C_H_INIT_COUNT, pty_size, elt_size, hht, cmp_pty, free_elt);
  push_ptys_rev_elts(&h, &res, ptys, elts, n);
  update_ptys_elts(&h, &res, ptys, elts, n);
  search_ptys_elts(&h, &res, elts, not_heap_elts, n);
  pop_ptys_elts(&h, &res, pty_size, elt_size, elts, n, cmp_pty, cmp_elt);
  free_heap(&h);
  printf("\t\torder correctness:                           ");
  print_test_result(res);
  for (i = 0; i < n; i++){
    free(ptys[i]);
    if (free_elt != NULL){
      free_elt(elts[i]);
      free_elt(not_heap_elts[i]);
    }
    free(elts[i]);
    free(not_heap_elts[i]);
  }
  free(ptys);
  free(elts);
  free(not_heap_elts);
  ptys = NULL;
  elts = NULL;
  not_heap_elts = NULL;
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

int main(int argc, char *argv[]){
  int i;
  size_t *args = NULL;
  float alpha_div, alpha_mul;
  if (argc > C_ARGC_MAX){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_MAX - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_MAX - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] > C_FULL_BIT - 2 ||
      args[1] < 1 ||
      args[2] < 1 ||
      args[3] < 1 ||
      args[4] < 1 ||
      args[5] > 1 ||
      args[6] > 1 ||
      args[7] > 1 ||
      args[8] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  alpha_div = (float)args[1] / args[2];
  alpha_mul = (float)args[3] / args[4];
  if (alpha_mul >= C_ALPHA_MUL_MAX){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[5]){
    run_push_pop_free_div_uint_test(args[0], alpha_div);
    run_push_pop_free_div_uint_ptr_test(args[0], alpha_div);
  }
  if (args[6]){
    run_update_search_div_uint_test(args[0], alpha_div);
    run_update_search_div_uint_ptr_test(args[0], alpha_div);
  }
  if (args[7]){
    run_push_pop_free_mul_uint_test(args[0], alpha_mul);
    run_push_pop_free_mul_uint_ptr_test(args[0], alpha_mul);
  }
  if (args[8]){
    run_update_search_mul_uint_test(args[0], alpha_mul);
    run_update_search_mul_uint_ptr_test(args[0], alpha_mul);
  }
  free(args);
  args = NULL;
  return 0;
}
