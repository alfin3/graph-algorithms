/**
   heap-main.c

   Tests of a generic dynamically allocated (min) heap with a hash table
   parameter across i) division- and mutliplication-based hash tables,
   ii) contiguous and noncontiguous elements, and iii) priority types.

   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "heap.h"
#include "ht-div-uint64.h"
#include "ht-mul-uint64.h"
#include "utilities-mem.h"

void push_pop_free_pty_types(uint64_t n,
			     uint64_t pty_size,
			     uint64_t elt_size,
			     const heap_ht_t *ht,
			     int (*cmp_pty)(const void *, const void *),
			     int (*cmp_elt)(const void *, const void *),
			     void (*new_pty)(void *, uint64_t),
			     void (*new_elt)(void *, uint64_t),
			     void (*free_elt)(void *));
void update_search_pty_types(uint64_t n,
			     uint64_t pty_size,
			     uint64_t elt_size,
			     const heap_ht_t *ht,
			     int (*cmp_pty)(const void *, const void *),
			     int (*cmp_elt)(const void *, const void *),
			     void (*new_pty)(void *, uint64_t),
			     void (*new_elt)(void *, uint64_t),
			     void (*free_elt)(void *));
void print_test_result(int res);

/**
   Run heap_{push, pop, free} and heap_{update, search} tests with division-
   and mutliplication-based hash tables on uint64_t elements across
   priority types. A pointer to an element is passed as elt in heap_push
   and the element is fully copied into a heap. NULL as free_elt is
   sufficient to free the heap.
*/

int cmp_uint64(const void *a, const void *b){
  if (*(uint64_t *)a >  *(uint64_t *)b){
    return 1;
  }else if(*(uint64_t *)a < *(uint64_t *)b){
    return -1;
  }else{
    return 0;
  }
}

int cmp_double(const void *a, const void *b){
  if (*(double *)a >  *(double *)b){
    return 1;
  }else if(*(double *)a < *(double *)b){
    return -1;
  }else{
    return 0;
  }
}

int cmp_long_double(const void *a, const void *b){
  if (*(long double *)a >  *(long double *)b){
    return 1;
  }else if(*(long double *)a < *(long double *)b){
    return -1;
  }else{
    return 0;
  }
}

void new_uint64(void *a, uint64_t val){
  uint64_t *s = a;
  *s = val;
  s = NULL;
}

void new_double(void *a, uint64_t val){
  double *s = a;
  *s = val;
  s = NULL;
}

void new_long_double(void *a, uint64_t val){
  long double *s = a;
  *s = val;
  s = NULL;
}

typedef struct{
  float alpha;
} ht_div_context_t;

typedef struct{
  float alpha;
  void (*rdc_key)(void *, const void *);
} ht_mul_context_t;

void ht_div_uint64_init_helper(ht_div_uint64_t *ht,
			       size_t key_size,
			       size_t elt_size,
			       void (*free_elt)(void *),
			       void *context){
  ht_div_context_t *c = context;
  ht_div_uint64_init(ht,
		     key_size,
		     elt_size,
		     c->alpha,
		     free_elt);
}

void ht_mul_uint64_init_helper(ht_mul_uint64_t *ht,
			       size_t key_size,
			       size_t elt_size,
			       void (*free_elt)(void *),
			       void *context){
  ht_mul_context_t * c = context;
  ht_mul_uint64_init(ht,
		     key_size,
		     elt_size,
		     c->alpha,
		     c->rdc_key,
		     free_elt);
}

/**
   Runs a heap_{push, pop, free} test with a ht_div_uint64_t hash table on
   uint64_t elements across priority types.
*/
void run_push_pop_free_div_uint64_test(){
  int i, pty_types_count = 3;
  const char *pty_types[3] = {"uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[3] = {sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  uint64_t n = 1000000;
  float alpha = 1.0;
  ht_div_uint64_t ht_div;
  ht_div_context_t context;
  heap_ht_t ht;
  int (*cmp_pty_arr[3])(const void *, const void *) = {cmp_uint64,
						       cmp_double,
						       cmp_long_double};
  void (*new_pty_arr[3])(void *, uint64_t) = {new_uint64,
					      new_double,
					      new_long_double};
  context.alpha = alpha;
  ht.ht = &ht_div;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_div_uint64_insert;
  ht.search = (heap_ht_search)ht_div_uint64_search;
  ht.remove = (heap_ht_remove)ht_div_uint64_remove;
  ht.free = (heap_ht_free)ht_div_uint64_free;
  printf("Run a heap_{push, pop, free} test with a ht_div_uint64_t "
	 "hash table on uint64_t elements\n");
  for (i = 0; i < pty_types_count; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   n, pty_types[i]);
    push_pop_free_pty_types(n,
			    pty_sizes[i],
			    sizeof(uint64_t),
			    &ht,
			    cmp_pty_arr[i],
			    cmp_uint64,
			    new_pty_arr[i],
			    new_uint64,
			    NULL);
  }
}

/**
   Runs a heap_{update, search} test with a ht_div_uint64_t hash table on
   uint64_t elements across priority types.
*/
void run_update_search_div_uint64_test(){
  int i, pty_types_count = 3;
  const char *pty_types[3] = {"uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[3] = {sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  uint64_t n = 1000000;
  float alpha = 1.0;
  ht_div_uint64_t ht_div;
  ht_div_context_t context;
  heap_ht_t ht;
  int (*cmp_pty_arr[3])(const void *, const void *) = {cmp_uint64,
						       cmp_double,
						       cmp_long_double};
  void (*new_pty_arr[3])(void *, uint64_t) = {new_uint64,
					      new_double,
					      new_long_double};
  context.alpha = alpha;
  ht.ht = &ht_div;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_div_uint64_insert;
  ht.search = (heap_ht_search)ht_div_uint64_search;
  ht.remove = (heap_ht_remove)ht_div_uint64_remove;
  ht.free = (heap_ht_free)ht_div_uint64_free;
  printf("Run a heap_{update, search} test with a ht_div_uint64_t "
	 "hash table on uint64_t elements\n");
  for (i = 0; i < pty_types_count; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   n, pty_types[i]);
    update_search_pty_types(n,
			    pty_sizes[i],
			    sizeof(uint64_t),
			    &ht,
			    cmp_pty_arr[i],
			    cmp_uint64,
			    new_pty_arr[i],
			    new_uint64,
			    NULL);
  }
}

/**
   Runs a heap_{push, pop, free} test with a ht_mul_uint64_t hash table on
   uint64_t elements across priority types.
*/
void run_push_pop_free_mul_uint64_test(){
  int i, pty_types_count = 3;
  const char *pty_types[3] = {"uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[3] = {sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  uint64_t n = 1000000;
  float alpha = 0.4;
  ht_mul_uint64_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t ht;
  int (*cmp_pty_arr[3])(const void *, const void *) = {cmp_uint64,
						       cmp_double,
						       cmp_long_double};
  void (*new_pty_arr[3])(void *, uint64_t) = {new_uint64,
					      new_double,
					      new_long_double};
  context.alpha = alpha;
  context.rdc_key = NULL; /* elt_size <= 8 bytes */
  ht.ht = &ht_mul;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_uint64_insert;
  ht.search = (heap_ht_search)ht_mul_uint64_search;
  ht.remove = (heap_ht_remove)ht_mul_uint64_remove;
  ht.free = (heap_ht_free)ht_mul_uint64_free;
  printf("Run a heap_{push, pop, free} test with a ht_mul_uint64_t "
	 "hash table on uint64_t elements\n");
  for (i = 0; i < pty_types_count; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   n, pty_types[i]);
    push_pop_free_pty_types(n,
			    pty_sizes[i],
			    sizeof(uint64_t),
			    &ht,
			    cmp_pty_arr[i],
			    cmp_uint64,
			    new_pty_arr[i],
			    new_uint64,
			    NULL);
  }
}

/**
   Runs a heap_{update, search} test with a ht_mul_uint64_t hash table on
   uint64_t elements across priority types.
*/
void run_update_search_mul_uint64_test(){
  int i, pty_types_count = 3;
  const char *pty_types[3] = {"uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[3] = {sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  uint64_t n = 1000000;
  float alpha = 0.4;
  ht_mul_uint64_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t ht;
  int (*cmp_pty_arr[3])(const void *, const void *) = {cmp_uint64,
						       cmp_double,
						       cmp_long_double};
  void (*new_pty_arr[3])(void *, uint64_t) = {new_uint64,
					      new_double,
					      new_long_double};
  context.alpha = alpha;
  context.rdc_key = NULL; /* elt_size <= 8 bytes */
  ht.ht = &ht_mul;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_uint64_insert;
  ht.search = (heap_ht_search)ht_mul_uint64_search;
  ht.remove = (heap_ht_remove)ht_mul_uint64_remove;
  ht.free = (heap_ht_free)ht_mul_uint64_free;
  printf("Run a heap_{update, search} test with a ht_mul_uint64_t "
	 "hash table on uint64_t elements\n");
  for (i = 0; i < pty_types_count; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   n, pty_types[i]);
    update_search_pty_types(n,
			    pty_sizes[i],
			    sizeof(uint64_t),
			    &ht,
			    cmp_pty_arr[i],
			    cmp_uint64,
			    new_pty_arr[i],
			    new_uint64,
			    NULL);
  }
}

/**
   Run heap_{push, pop, free} and heap_{update, search} tests with division-
   and mutliplication-based hash tables on noncontiguous uint64_ptr_t
   elements across priority types. A pointer to a pointer to an element is
   passed as elt in heap_push, and the pointer to the element is copied into
   a heap. An element-specific free_elt is necessary to free the heap.
*/

typedef struct{
  uint64_t *val;
} uint64_ptr_t;

int cmp_uint64_ptr(const void *a, const void *b){
  if (*((*(uint64_ptr_t **)a)->val) > *((*(uint64_ptr_t **)b)->val)){
    return 1;
  }else if (*((*(uint64_ptr_t **)a)->val) < *((*(uint64_ptr_t **)b)->val)){
    return -1;
  }else{
    return 0;
  }
}

void new_uint64_ptr(void *a, uint64_t val){
  uint64_ptr_t **s = a;
  *s = malloc_perror(sizeof(uint64_ptr_t));
  (*s)->val = malloc_perror(sizeof(uint64_t));
  *((*s)->val) = val;
  s = NULL;
}

void free_uint64_ptr(void *a){
  uint64_ptr_t **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

/**
   Runs a heap_{push, pop, free} test with a ht_div_uint64_t hash table on
   noncontiguous uint64_ptr_t elements across priority types.
*/
void run_push_pop_free_div_uint64_ptr_test(){
  int i, pty_types_count = 3;
  const char *pty_types[3] = {"uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[3] = {sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  uint64_t n = 1000000;
  float alpha = 1.0;
  ht_div_uint64_t ht_div;
  ht_div_context_t context;
  heap_ht_t ht;
  int (*cmp_pty_arr[3])(const void *, const void *) = {cmp_uint64,
						       cmp_double,
						       cmp_long_double};
  void (*new_pty_arr[3])(void *, uint64_t) = {new_uint64,
					      new_double,
					      new_long_double};
  context.alpha = alpha;
  ht.ht = &ht_div;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_div_uint64_insert;
  ht.search = (heap_ht_search)ht_div_uint64_search;
  ht.remove = (heap_ht_remove)ht_div_uint64_remove;
  ht.free = (heap_ht_free)ht_div_uint64_free;
  printf("Run a heap_{push, pop, free} test with a ht_div_uint64_t "
	 "hash table on noncontiguous uint64_ptr_t elements\n");
  for (i = 0; i < pty_types_count; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   n, pty_types[i]);
    push_pop_free_pty_types(n,
			    pty_sizes[i],
			    sizeof(uint64_ptr_t *),
			    &ht,
			    cmp_pty_arr[i],
			    cmp_uint64_ptr,
			    new_pty_arr[i],
			    new_uint64_ptr,
			    free_uint64_ptr);
  }
}

/**
   Runs a heap_{update, search} test with a ht_div_uint64_t hash table on
   noncontiguous uint64_ptr_t elements across priority types.
*/
void run_update_search_div_uint64_ptr_test(){
  int i, pty_types_count = 3;
  const char *pty_types[3] = {"uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[3] = {sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  uint64_t n = 1000000;
  float alpha = 1.0;
  ht_div_uint64_t ht_div;
  ht_div_context_t context;
  heap_ht_t ht;
  int (*cmp_pty_arr[3])(const void *, const void *) = {cmp_uint64,
						       cmp_double,
						       cmp_long_double};
  void (*new_pty_arr[3])(void *, uint64_t) = {new_uint64,
					      new_double,
					      new_long_double};
  context.alpha = alpha;
  ht.ht = &ht_div;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_div_uint64_insert;
  ht.search = (heap_ht_search)ht_div_uint64_search;
  ht.remove = (heap_ht_remove)ht_div_uint64_remove;
  ht.free = (heap_ht_free)ht_div_uint64_free;
  printf("Run a heap_{update, search} test with a ht_div_uint64_t "
	 "hash table on noncontiguous uint64_ptr_t elements\n");
  for (i = 0; i < pty_types_count; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   n, pty_types[i]);
    update_search_pty_types(n,
			    pty_sizes[i],
			    sizeof(uint64_ptr_t *),
			    &ht,
			    cmp_pty_arr[i],
			    cmp_uint64_ptr,
			    new_pty_arr[i],
			    new_uint64_ptr,
			    free_uint64_ptr);
  }
}

/**
   Runs a heap_{push, pop, free} test with a ht_mul_uint64_t hash table on
   noncontiguous uint64_ptr_t elements across priority types.
*/
void run_push_pop_free_mul_uint64_ptr_test(){
  int i, pty_types_count = 3;
  const char *pty_types[3] = {"uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[3] = {sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  uint64_t n = 1000000;
  float alpha = 0.4;
  ht_mul_uint64_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t ht;
  int (*cmp_pty_arr[3])(const void *, const void *) = {cmp_uint64,
						       cmp_double,
						       cmp_long_double};
  void (*new_pty_arr[3])(void *, uint64_t) = {new_uint64,
					      new_double,
					      new_long_double};
  context.alpha = alpha;
  context.rdc_key = NULL; /* elt_size <= 8 bytes */
  ht.ht = &ht_mul;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_uint64_insert;
  ht.search = (heap_ht_search)ht_mul_uint64_search;
  ht.remove = (heap_ht_remove)ht_mul_uint64_remove;
  ht.free = (heap_ht_free)ht_mul_uint64_free;
  printf("Run a heap_{push, pop, free} test with a ht_mul_uint64_t "
	 "hash table on noncontiguous uint64_ptr_t elements\n");
  for (i = 0; i < pty_types_count; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   n, pty_types[i]);
    push_pop_free_pty_types(n,
			    pty_sizes[i],
			    sizeof(uint64_ptr_t *),
			    &ht,
			    cmp_pty_arr[i],
			    cmp_uint64_ptr,
			    new_pty_arr[i],
			    new_uint64_ptr,
			    free_uint64_ptr);
  }
}

/**
   Runs a heap_{update, search} test with a ht_mul_uint64_t hash table on
   noncontiguous uint64_ptr_t elements across priority types.
*/
void run_update_search_mul_uint64_ptr_test(){
  int i, pty_types_count = 3;
  const char *pty_types[3] = {"uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[3] = {sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  uint64_t n = 1000000;
  float alpha = 0.4;
  ht_mul_uint64_t ht_mul;
  ht_mul_context_t context;
  heap_ht_t ht;
  int (*cmp_pty_arr[3])(const void *, const void *) = {cmp_uint64,
						       cmp_double,
						       cmp_long_double};
  void (*new_pty_arr[3])(void *, uint64_t) = {new_uint64,
					      new_double,
					      new_long_double};
  context.alpha = alpha;
  context.rdc_key = NULL; /* elt_size <= 8 bytes */
  ht.ht = &ht_mul;
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_uint64_insert;
  ht.search = (heap_ht_search)ht_mul_uint64_search;
  ht.remove = (heap_ht_remove)ht_mul_uint64_remove;
  ht.free = (heap_ht_free)ht_mul_uint64_free;
  printf("Run a heap_{update, search} test with a ht_mul_uint64_t "
	 "hash table on noncontiguous uint64_ptr_t elements\n");
  for (i = 0; i < pty_types_count; i++){
    printf("\tnumber of elements: %lu, priority type: %s\n",
	   n, pty_types[i]);
    update_search_pty_types(n,
			    pty_sizes[i],
			    sizeof(uint64_ptr_t *),
			    &ht,
			    cmp_pty_arr[i],
			    cmp_uint64_ptr,
			    new_pty_arr[i],
			    new_uint64_ptr,
			    free_uint64_ptr);
  }
}

/** 
   Helper functions for heap_{push, pop, free} tests.
*/

void push_incr_ptys_elts(heap_t *h,
			 int *res,
			 uint64_t pty_size,
			 void **elts,
			 uint64_t count,
			 void (*new_pty)(void *, uint64_t)){
  uint64_t i, first_half_count = count / 2;
  uint64_t n = h->num_elts;
  void *pty = NULL;
  clock_t t;
  pty = malloc_perror(pty_size);
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
			 uint64_t pty_size,
			 void **elts,
			 uint64_t count,
			 void (*new_pty)(void *, uint64_t)){
  uint64_t i, first_half_count = count / 2;
  uint64_t n = h->num_elts;
  void *pty = NULL;
  clock_t t;
  pty = malloc_perror(pty_size);
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
		   uint64_t pty_size,
		   uint64_t elt_size,
		   void **elts,
		   uint64_t count,
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *)){
  uint64_t i, first_half_count = count / 2;
  uint64_t n = h->num_elts;
  void *pty_prev = NULL, *pty_cur = NULL, *elt = NULL;
  clock_t t;
  pty_prev = malloc_perror(pty_size);
  pty_cur = malloc_perror(pty_size);
  elt = malloc_perror(elt_size);
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

void push_pop_free_pty_types(uint64_t n,
			     uint64_t pty_size,
			     uint64_t elt_size,
			     const heap_ht_t *ht,
			     int (*cmp_pty)(const void *, const void *),
			     int (*cmp_elt)(const void *, const void *),
			     void (*new_pty)(void *, uint64_t),
			     void (*new_elt)(void *, uint64_t),
			     void (*free_elt)(void *)){
  int res = 1;
  uint64_t init_count = 1;
  uint64_t i;
  void **elts = NULL;
  heap_t h;
  elts = malloc_perror(n * sizeof(void *));
  for (i = 0; i < n; i++){
    elts[i] = malloc_perror(elt_size);
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
			     uint64_t pty_size,
			     void **elts,
			     uint64_t count,
			     void (*new_pty)(void *, uint64_t)){
  uint64_t i, first_half_count = count / 2;
  uint64_t n = h->num_elts;
  void *pty = NULL;
  clock_t t;
  pty = malloc_perror(pty_size);
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
			  uint64_t pty_size,
			  void **elts,
			  uint64_t count,
			  void (*new_pty)(void *, uint64_t)){
  uint64_t i, first_half_count = count / 2;
  uint64_t n = h->num_elts;
  void *pty = malloc_perror(pty_size);
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
		      uint64_t pty_size,
		      uint64_t elt_size,
		      void **elts,
		      uint64_t count){
  uint64_t n = h->num_elts;
  uint64_t i;
  void *pty = NULL, *elt = NULL, *ptr = NULL;
  clock_t t;
  pty = malloc_perror(pty_size);
  elt = malloc_perror(elt_size);
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

void update_search_pty_types(uint64_t n,
			     uint64_t pty_size,
			     uint64_t elt_size,
			     const heap_ht_t *ht,
			     int (*cmp_pty)(const void *, const void *),
			     int (*cmp_elt)(const void *, const void *),
			     void (*new_pty)(void *, uint64_t),
			     void (*new_elt)(void *, uint64_t),
			     void (*free_elt)(void *)){
  int res = 1;
  uint64_t init_count = 1;
  uint64_t i;
  void **elts = NULL;
  heap_t h;
  elts = malloc_perror(n * sizeof(void *));
  for (i = 0; i < n; i++){
    elts[i] = malloc_perror(elt_size);
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
  /* ht_div_uint64_t hash table */
  run_push_pop_free_div_uint64_test();
  run_push_pop_free_div_uint64_ptr_test();
  run_update_search_div_uint64_test();
  run_update_search_div_uint64_ptr_test();
  
  /* ht_mul_uint64_t hash table */
  run_push_pop_free_mul_uint64_test();
  run_push_pop_free_mul_uint64_ptr_test();
  run_update_search_mul_uint64_test();
  run_update_search_mul_uint64_ptr_test();
  return 0;
}
