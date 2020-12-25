/**
   heap-uint32-main.c

   Examples of a generic dynamically allocated (min) heap with upto 2^32 - 2
   elements.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "heap-uint32.h"
#include "utilities-rand-mod.h"

void print_test_result(int result);
static void push_pop_free_pty_type(uint32_t n,
				   int pty_size,
				   int elt_size,
				   int (*cmp_pty_fn)(void *, void *),
				   int (*cmp_elt_val_fn)(void *, void *),
				   int (*cmp_elt_size_fn)(void *, void *),
				   void (*cstr_pty_fn)(void *, uint32_t),
				   void (*cstr_elt_fn)(void *, uint32_t),
				   void (*free_elt_fn)(void *));
static void update_search_pty_type(uint32_t n,
				   int pty_size,
				   int elt_size,
				   int (*cmp_pty_fn)(void *, void *),
				   int (*cmp_elt_val_fn)(void *, void *),
				   int (*cmp_elt_size_fn)(void *, void *),
				   void (*cstr_pty_fn)(void *, uint32_t),
				   void (*cstr_elt_fn)(void *, uint32_t),
				   void (*free_elt_fn)(void *));

/**
   Run heap_uint32_{push, pop, free} and heap_uint32_{update, search} tests 
   on uint32_t elements across priority types. A pointer to an element is 
   passed as elt in heap_uint32_push and the element is fully copied into 
   the elts array of the heap. NULL as free_elt_fn is sufficient to free 
   the heap.
*/

int cmp_uint32_fn(void *a, void *b){
  if (*(uint32_t *)a >  *(uint32_t *)b){
    return 1;
  }else if(*(uint32_t *)a < *(uint32_t *)b){
    return -1;
  }else{
    return 0;
  }
}

int cmp_uint64_fn(void *a, void *b){
  if (*(uint64_t *)a >  *(uint64_t *)b){
    return 1;
  }else if(*(uint64_t *)a < *(uint64_t *)b){
    return -1;
  }else{
    return 0;
  }
}

int cmp_double_fn(void *a, void *b){
  return *(double *)a - *(double *)b;
}

int cmp_long_double_fn(void *a, void *b){
  return *(long double *)a - *(long double *)b;
}

/**
   Construct element or priority objects from an uint32_t value. The ptr 
   parameter is a pointer to a preallocated block of size elt_size or 
   pty_size.
*/
void cstr_uint32_fn(void *ptr, uint32_t val){
  uint32_t *s = ptr;
  *s = val;
  s = NULL;
}

void cstr_uint64_fn(void *ptr, uint32_t val){
  uint64_t *s = ptr;
  *s = val;
  s = NULL;
}

void cstr_double_fn(void *ptr, uint32_t val){
  double *s = ptr;
  *s = val;
  s = NULL;
}

void cstr_long_double_fn(void *ptr, uint32_t val){
  long double *s = ptr;
  *s = val;
  s = NULL;
}

/**
   Runs a heap_uint32_{push, pop, free} test on uint32_t elements across 
   priority types.
*/
void run_push_pop_free_uint32_elt_test(){
  uint32_t n = 1000000;
  int num_pty_types = 4;
  const char *pty_types[4] = {"uint32_t",
			      "uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[4] = {sizeof(uint32_t),
		      sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  int (*cmp_pty_fn_arr[4])(void *, void *) = {cmp_uint32_fn,
					      cmp_uint64_fn,
					      cmp_double_fn,
					      cmp_long_double_fn};
  void (*cstr_pty_fn_arr[4])(void *, uint32_t) = {cstr_uint32_fn,
						  cstr_uint64_fn,
						  cstr_double_fn,
						  cstr_long_double_fn};
  printf("Run a heap_uint32_{push, pop, free} test on uint32_t elements "
	 "across priority types\n");
  for (int i = 0; i < num_pty_types; i++){
    printf("\tnumber of elements: %u, priority type: %s\n", n, pty_types[i]);
    push_pop_free_pty_type(n,
			   pty_sizes[i],
			   sizeof(uint32_t),
			   cmp_pty_fn_arr[i],
			   cmp_uint32_fn,
			   cmp_uint32_fn,
			   cstr_pty_fn_arr[i],
			   cstr_uint32_fn,
			   NULL);
  }
}

/**
   Runs a heap_uint32_{update, search} test on uint32_t elements across 
   priority types.
*/
void run_update_search_uint32_elt_test(){
  uint32_t n = 1000000;
  int num_pty_types = 4;
  const char *pty_types[4] = {"uint32_t",
			      "uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[4] = {sizeof(uint32_t),
		      sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  int (*cmp_pty_fn_arr[4])(void *, void *) = {cmp_uint32_fn,
					      cmp_uint64_fn,
					      cmp_double_fn,
					      cmp_long_double_fn};
  void (*cstr_pty_fn_arr[4])(void *, uint32_t) = {cstr_uint32_fn,
						  cstr_uint64_fn,
						  cstr_double_fn,
						  cstr_long_double_fn};
  printf("Run a heap_uint32_{update, search} test on uint32_t elements "
	 "across priority types\n");
  for (int i = 0; i < num_pty_types; i++){
    printf("\tnumber of elements: %u, priority type: %s\n", n, pty_types[i]);
    update_search_pty_type(n,
			   pty_sizes[i],
			   sizeof(uint32_t),
			   cmp_pty_fn_arr[i],
			   cmp_uint32_fn,
			   cmp_uint32_fn,
			   cstr_pty_fn_arr[i],
			   cstr_uint32_fn,
			   NULL);
  }
}

/**
   Run heap_uint32_{push, pop, free} and heap_uint32_{update, search} tests 
   on multilayered uint32_ptr_t elements across priority types. A pointer 
   to a pointer to an element is passed as elt in heap_uint32_push, and the 
   pointer to the element is copied into the elts array of a heap. An 
   element-specific free_elt_fn is necessary to free the heap.
*/

typedef struct{
  uint32_t *val;
} uint32_ptr_t;

/**
   Compares the values of two uint32_ptr_t elements. 
*/
int cmp_uint32_ptr_val_fn(void *a, void *b){
  uint32_ptr_t **s_a  = a;
  uint32_ptr_t **s_b  = b;
  if (*((*s_a)->val) > *((*s_b)->val)){
    return 1;
  }else if(*((*s_a)->val) < *((*s_b)->val)){
    return -1;
  }else{
    return 0;
  }
}

/**
   Compares the elt_size blocks of two uint32_ptr_t elements for hashing.
*/
int cmp_uint32_ptr_elt_size_fn(void *a, void *b){
  return memcmp(a, b, sizeof(uint32_ptr_t *));
}

/**
   Constructs an uint32_ptr_t element. The elt parameter is a pointer to a 
   preallocated block of size elt_size, i.e. sizeof(uint32_ptr_t *).
*/
void cstr_uint32_ptr_fn(void *elt, uint32_t val){
  uint32_ptr_t **s = elt;
  *s = malloc(sizeof(uint32_ptr_t));
  assert(*s != NULL);
  (*s)->val = malloc(sizeof(uint32_t));
  assert((*s)->val != NULL);
  *((*s)->val) = val;
  s = NULL;
}

/**
   Frees an uint32_ptr_t element and leaves a block of size elt_size pointed 
   to by the elt parameter.
*/
void free_uint32_ptr_fn(void *elt){
  uint32_ptr_t **s = elt;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

/**
   Runs a heap_uint32_{push, pop, free} test on uint32_ptr_t elements across 
   priority types.
*/
void run_push_pop_free_uint32_ptr_t_elt_test(){
  uint32_t n = 1000000;
  int num_pty_types = 4;
  const char *pty_types[4] = {"uint32_t",
			      "uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[4] = {sizeof(uint32_t),
		      sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  int (*cmp_pty_fn_arr[4])(void *, void *) = {cmp_uint32_fn,
					      cmp_uint64_fn,
					      cmp_double_fn,
					      cmp_long_double_fn};
  void (*cstr_pty_fn_arr[4])(void *, uint32_t) = {cstr_uint32_fn,
						  cstr_uint64_fn,
						  cstr_double_fn,
						  cstr_long_double_fn};
  printf("Run a heap_uint32_{push, pop, free} test on multilayered "
	 "uint32_ptr_t elements across priority types\n");
  for (int i = 0; i < num_pty_types; i++){
    printf("\tnumber of elements: %u, priority type: %s\n", n, pty_types[i]);
    push_pop_free_pty_type(n,
			   pty_sizes[i],
			   sizeof(uint32_ptr_t *),
			   cmp_pty_fn_arr[i],
			   cmp_uint32_ptr_val_fn,
			   cmp_uint32_ptr_elt_size_fn,
			   cstr_pty_fn_arr[i],
			   cstr_uint32_ptr_fn,
			   free_uint32_ptr_fn);
  }
}

/**
   Runs a heap_uint32_{update, search} test on uint32_ptr_t elements across 
   priority types.
*/
void run_update_search_uint32_ptr_t_elt_test(){
  uint32_t n = 1000000;
  int num_pty_types = 4;
  const char *pty_types[4] = {"uint32_t",
			      "uint64_t",
			      "double",
			      "long double"};
  int pty_sizes[4] = {sizeof(uint32_t),
		      sizeof(uint64_t),
		      sizeof(double),
		      sizeof(long double)};
  int (*cmp_pty_fn_arr[4])(void *, void *) = {cmp_uint32_fn,
					      cmp_uint64_fn,
					      cmp_double_fn,
					      cmp_long_double_fn};
  void (*cstr_pty_fn_arr[4])(void *, uint32_t) = {cstr_uint32_fn,
						  cstr_uint64_fn,
						  cstr_double_fn,
						  cstr_long_double_fn};
  printf("Run a heap_uint32_{update, search} test on multilayered "
	 "uint32_ptr_t elements across priority types\n");
  for (int i = 0; i < num_pty_types; i++){
    printf("\tnumber of elements: %u, priority type: %s\n", n, pty_types[i]);
    update_search_pty_type(n,
			   pty_sizes[i],
			   sizeof(uint32_ptr_t *),
			   cmp_pty_fn_arr[i],
			   cmp_uint32_ptr_val_fn,
			   cmp_uint32_ptr_elt_size_fn,
			   cstr_pty_fn_arr[i],
			   cstr_uint32_ptr_fn,
			   free_uint32_ptr_fn);
  }
}

/** 
   Helper functions for heap_uint32_{push, pop, free} tests.
*/

static void push_incr_ptys_elts(heap_uint32_t *h,
				int *result,
				int pty_size,
				void **elt_arr,
				uint32_t arr_size,
				void (*cstr_pty_fn)(void *, uint32_t)){
  uint32_t first_half_arr_size = arr_size / 2;
  uint32_t n = h->num_elts;
  void *pty = malloc(pty_size);
  assert(pty != NULL);
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < first_half_arr_size; i++){
    cstr_pty_fn(pty, i);
    heap_uint32_push(h, pty, elt_arr[i]);
  }
  t = clock() - t;
  printf("\t\tpush 1/2 elements, incr. priorities:           "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n + first_half_arr_size);
  t = clock();
  for (uint32_t i = first_half_arr_size; i < arr_size; i++){
    cstr_pty_fn(pty, i);
    heap_uint32_push(h, pty, elt_arr[i]);
  }
  t = clock() - t;
  printf("\t\tpush residual elements, incr. priorities:      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n + arr_size);
  free(pty);
  pty = NULL;
}

static void push_decr_ptys_elts(heap_uint32_t *h,
				int *result,
				int pty_size,
				void **elt_arr,
				uint32_t arr_size,
				void (*cstr_pty_fn)(void *, uint32_t)){
  uint32_t first_half_arr_size = arr_size / 2;
  uint32_t n = h->num_elts;
  void *pty = malloc(pty_size);
  assert(pty != NULL);
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < first_half_arr_size; i++){
    cstr_pty_fn(pty, arr_size - i - 1);
    heap_uint32_push(h, pty, elt_arr[arr_size - i - 1]);
  }
  t = clock() - t;
  printf("\t\tpush 1/2 elements, decr. priorities:           "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n + first_half_arr_size);
  t = clock();
  for (uint32_t i = first_half_arr_size; i < arr_size; i++){
    cstr_pty_fn(pty, arr_size - i - 1);
    heap_uint32_push(h, pty, elt_arr[arr_size - i - 1]);
  }
  t = clock() - t;
  printf("\t\tpush residual elements, decr. priorities:      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n + arr_size);
  free(pty);
  pty = NULL;
}

static void pop_ptys_elts(heap_uint32_t *h,
			  int *result,
			  int pty_size,
			  int elt_size,
			  void **elt_arr,
			  uint32_t arr_size,
			  int (*cmp_pty_fn)(void *, void *),
			  int (*cmp_elt_val_fn)(void *, void *)){
  uint32_t first_half_arr_size = arr_size / 2;
  uint32_t n = h->num_elts;
  void *pty_prev, *pty_cur, *elt;
  pty_prev = malloc(pty_size);
  assert(pty_prev != NULL);
  pty_cur = malloc(pty_size);
  assert(pty_cur != NULL);
  elt = malloc(elt_size);
  assert(elt != NULL);
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < first_half_arr_size; i++){
    if (i == 0){
      heap_uint32_pop(h, pty_cur, elt);
      *result *= (cmp_elt_val_fn(elt, elt_arr[i]) == 0);
    }else{
      heap_uint32_pop(h, pty_cur, elt);
      *result *= (cmp_pty_fn(pty_prev, pty_cur) <= 0);
      *result *= (cmp_elt_val_fn(elt, elt_arr[i]) == 0);
    }
    memcpy(pty_prev, pty_cur, pty_size); 
  }
  t = clock() - t;
  printf("\t\tpop 1/2 elements:                              "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n - first_half_arr_size);
  t = clock();
  for (uint32_t i = first_half_arr_size; i < arr_size; i++){
    if (i == 0){
      heap_uint32_pop(h, pty_cur, elt);
      *result *= (cmp_elt_val_fn(elt, elt_arr[i]) == 0);
    }else{
      heap_uint32_pop(h, pty_cur, elt);
      *result *= (cmp_pty_fn(pty_prev, pty_cur) <= 0);
      *result *= (cmp_elt_val_fn(elt, elt_arr[i]) == 0);
    }
    memcpy(pty_prev, pty_cur, pty_size); 
  }
  t = clock() - t;
  printf("\t\tpop residual elements:                         "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n - arr_size);
  free(pty_prev);
  free(pty_cur);
  free(elt);
  pty_prev = NULL;
  pty_cur = NULL;
  elt = NULL;
}

static void free_heap(heap_uint32_t *h){
  clock_t t;
  t = clock();
  heap_uint32_free(h);
  t = clock() - t;
  printf("\t\tfree time:                                     "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

static void push_pop_free_pty_type(uint32_t n,
				   int pty_size,
				   int elt_size,
				   int (*cmp_pty_fn)(void *, void *),
				   int (*cmp_elt_val_fn)(void *, void *),
				   int (*cmp_elt_size_fn)(void *, void *),
				   void (*cstr_pty_fn)(void *, uint32_t),
				   void (*cstr_elt_fn)(void *, uint32_t),
				   void (*free_elt_fn)(void *)){
  heap_uint32_t h;
  uint32_t init_size = 1;
  void **elt_arr;
  int result = 1;
  //preallocate to avoid the allocation of complex elements during timing exps
  elt_arr = malloc(n * sizeof(void *));
  assert(elt_arr != NULL);
  for (uint32_t i = 0; i < n; i++){
    elt_arr[i] = malloc(elt_size);
    assert(elt_arr[i] != NULL);
    cstr_elt_fn(elt_arr[i], i);
  }
  heap_uint32_init(&h,
		   init_size,
		   pty_size,
		   elt_size,
		   cmp_pty_fn,
		   cmp_elt_size_fn,
		   free_elt_fn);
  push_incr_ptys_elts(&h, &result, pty_size, elt_arr, n, cstr_pty_fn);
  pop_ptys_elts(&h,
		&result,
		pty_size,
		elt_size,
		elt_arr,
		n,
		cmp_pty_fn,
		cmp_elt_val_fn);
  push_decr_ptys_elts(&h, &result, pty_size, elt_arr, n, cstr_pty_fn);
  pop_ptys_elts(&h,
		&result,
		pty_size,
		elt_size,
		elt_arr,
		n,
		cmp_pty_fn,
		cmp_elt_val_fn);
  push_incr_ptys_elts(&h, &result, pty_size, elt_arr, n, cstr_pty_fn);
  free_heap(&h);
  printf("\t\torder correctness:                             ");
  print_test_result(result);
  //free elt_arr
  for (uint32_t i = 0; i < n; i++){
    free(elt_arr[i]);
  }
  free(elt_arr);
}

/** 
   Helper functions for heap_uint32_{update, search} tests.
*/

static void push_rev_incr_ptys_elts(heap_uint32_t *h,
				    int *result,
				    int pty_size,
				    void **elt_arr,
				    uint32_t arr_size,
				    void (*cstr_pty_fn)(void *, uint32_t)){
  uint32_t first_half_arr_size = arr_size / 2;
  uint32_t n = h->num_elts;
  void *pty = malloc(pty_size);
  assert(pty != NULL);
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < first_half_arr_size; i++){
    cstr_pty_fn(pty, i);
    heap_uint32_push(h, pty, elt_arr[arr_size - i - 1]);
  }
  t = clock() - t;
  printf("\t\tpush 1/2 elements, rev. incr. priorities:      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n + first_half_arr_size);
  t = clock();
  for (uint32_t i = first_half_arr_size; i < arr_size; i++){
    cstr_pty_fn(pty, i);
    heap_uint32_push(h, pty, elt_arr[arr_size - i - 1]);
  }
  t = clock() - t;
  printf("\t\tpush residual elements, rev. incr. priorities: "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n + arr_size);
  free(pty);
  pty = NULL;
}

static void update_rev_ptys_elts(heap_uint32_t *h,
				 int *result,
				 int pty_size,
				 void **elt_arr,
				 uint32_t arr_size,
				 void (*cstr_pty_fn)(void *, uint32_t)){
  uint32_t first_half_arr_size = arr_size / 2;
  uint32_t n = h->num_elts;
  void *pty = malloc(pty_size);
  assert(pty != NULL);
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < first_half_arr_size; i++){
    cstr_pty_fn(pty, i);
    heap_uint32_update(h, pty, elt_arr[i]);
  }
  t = clock() - t;
  printf("\t\tupdate 1/2 elements:                           "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n);
  t = clock();
  for (uint32_t i = first_half_arr_size; i < arr_size; i++){
    cstr_pty_fn(pty, i);
    heap_uint32_update(h, pty, elt_arr[i]);
  }
  t = clock() - t;
  printf("\t\tupdate residual elements:                      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n);
  free(pty);
  pty = NULL;
}

static void search_ptys_elts(heap_uint32_t *h,
			     int *result,
			     int pty_size,
			     int elt_size,
			     void **elt_arr,
			     uint32_t arr_size){
  uint32_t n = h->num_elts;
  void *pty, *elt, *ptr;
  pty = malloc(pty_size);
  assert(pty != NULL);
  elt = malloc(elt_size);
  assert(elt != NULL);
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < arr_size; i++){
    ptr = heap_uint32_search(h, elt_arr[i]);
    *result *= (ptr != NULL);
  }
  t = clock() - t;
  printf("\t\tin heap search:                                "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n);
  heap_uint32_pop(h, pty, elt);
  t = clock();
  for (uint32_t i = 0; i < arr_size; i++){
    ptr = heap_uint32_search(h, elt);
    *result *= (ptr == NULL);
  }
  t = clock() - t;
  printf("\t\tnot in heap search:                            "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (h->num_elts == n - 1);
  heap_uint32_push(h, pty, elt);
  *result *= (h->num_elts == n);
  free(pty);
  free(elt);
  pty = NULL;
  elt = NULL;
}

static void update_search_pty_type(uint32_t n,
				   int pty_size,
				   int elt_size,
				   int (*cmp_pty_fn)(void *, void *),
				   int (*cmp_elt_val_fn)(void *, void *),
				   int (*cmp_elt_size_fn)(void *, void *),
				   void (*cstr_pty_fn)(void *, uint32_t),
				   void (*cstr_elt_fn)(void *, uint32_t),
				   void (*free_elt_fn)(void *)){
  heap_uint32_t h;
  uint32_t init_size = 1;
  void **elt_arr;
  int result = 1;
  //preallocate to avoid the allocation of complex elements during timing exps
  elt_arr = malloc(n * sizeof(void *));
  assert(elt_arr != NULL);
  for (uint32_t i = 0; i < n; i++){
    elt_arr[i] = malloc(elt_size);
    assert(elt_arr[i] != NULL);
    cstr_elt_fn(elt_arr[i], i);
  }
  heap_uint32_init(&h,
		   init_size,
		   pty_size,
		   elt_size,
		   cmp_pty_fn,
		   cmp_elt_size_fn,
		   free_elt_fn);
  push_rev_incr_ptys_elts(&h, &result, pty_size, elt_arr, n, cstr_pty_fn);
  update_rev_ptys_elts(&h, &result, pty_size, elt_arr, n, cstr_pty_fn);
  search_ptys_elts(&h, &result, pty_size, elt_size, elt_arr, n);
  pop_ptys_elts(&h,
		&result,
		pty_size,
		elt_size,
		elt_arr,
		n,
		cmp_pty_fn,
		cmp_elt_val_fn);
  free_heap(&h);
  printf("\t\torder correctness:                             ");
  print_test_result(result);
  //free elt_arr
  for (uint32_t i = 0; i < n; i++){
    free(elt_arr[i]);
  }
  free(elt_arr);
}

/**
   Print test result.
*/
void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_push_pop_free_uint32_elt_test();
  run_push_pop_free_uint32_ptr_t_elt_test();
  run_update_search_uint32_elt_test();
  run_update_search_uint32_ptr_t_elt_test();
  return 0;
}
