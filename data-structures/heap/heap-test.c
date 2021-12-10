/**
   heap-test.c

   Tests of a (min) heap across i) division- and mutliplication-based hash
   tables, ii) contiguous and noncontiguous elements, and iii) basic 
   priority types.

   The following command line arguments can be used to customize tests:
   heap-test
      [0, size_t width - 1) : i s.t. # inserts = 2^i
      > 0 : a
      < size_t width : b s.t. 0.0 < a / 2**b
      > 0 : c
      < size_t width : d s.t. 0.0 < c / 2**d <= 1.0
      [0, 1] : on/off push pop free division hash table test
      [0, 1] : on/off update search division hash table test
      [0, 1] : on/off push pop free multiplication hash table test
      [0, 1] : on/off update search multiplication hash table test

   usage examples:
   ./heap-test
   ./heap-test 21
   ./heap-test 20 10 10
   ./heap-test 20 1 0
   ./heap-test 20 100 0
   ./heap-test 20 1 0 10 10 0 0
   ./heap-test 20 1 0 1 0 0 0
   ./heap-test 20 1 0 100 0 0 0

   heap-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99 with the only requirement that the width of size_t is
   greater or equal to 16, less than 2040, and is even.

   TODO: add portable size_t printing
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "heap.h"
#include "ht-divchn.h"
#include "ht-muloa.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-lim.h"

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "heap-test\n"
  "[0, size_t width - 1) : i s.t. # inserts = 2^i\n"
  "> 0 : a\n"
  "< size_t width : b s.t. 0.0 < a / 2**b\n"
  "> 0 : c\n"
  "< size_t width : d s.t. 0.0 < c / 2**d <= 1.0\n"
  "[0, 1] : on/off push pop free division hash table test\n"
  "[0, 1] : on/off update search division hash table test\n"
  "[0, 1] : on/off push pop free multiplication hash table test\n"
  "[0, 1] : on/off update search multiplication hash table test\n";
const int C_ARGC_MAX = 10;
const size_t C_ARGS_DEF[9] = {14u, 1u, 0u, 341u, 10u, 1u, 1u, 1u, 1u};
const size_t C_FULL_BIT = UINT_WIDTH_FROM_MAX((size_t)-1);

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
const size_t C_H_MIN_NUM = 1u;

void push_pop_free(size_t num_ins,
		   size_t pty_size,
		   size_t elt_size,
		   const struct heap_ht *hht,
		   void (*new_pty)(void *, size_t),
		   void (*new_elt)(void *, size_t),
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *),
		   size_t (*rdc_elt)(const void *),
		   void (*free_elt)(void *));
void update_search(size_t num_ins,
		   size_t pty_size,
		   size_t elt_size,
		   const struct heap_ht *hht,
		   void (*new_pty)(void *, size_t),
		   void (*new_elt)(void *, size_t),
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *),
		   size_t (*rdc_elt)(const void *),
		   void (*free_elt)(void *));
void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);

/**
   Run heap_{push, pop, free} and heap_{update, search} tests with division-
   and mutliplication-based hash tables on size_t elements across
   priority types. An element is an elt_size block of size sizeof(size_t)
   with a size_t value. Elements are entirely copied into the heap and
   free_elt is NULL.
*/

void new_uint(void *a, size_t val){
  size_t *s = a;
  *s = val;
}
void new_double(void *a, size_t val){
  double *s = a;
  *s = val;
}
void new_long_double(void *a, size_t val){
  long double *s = a;
  *s = val;
}

int cmp_uint(const void *a, const void *b){
  return ((*(size_t *)a > *(size_t *)b) -
	  (*(size_t *)a < *(size_t *)b));
}
int cmp_double(const void *a, const void *b){
  return ((*(double *)a > *(double *)b) -
	  (*(double *)a < *(double *)b));
}
int cmp_long_double(const void *a, const void *b){
  return ((*(long double *)a > *(long double *)b) -
	  (*(long double *)a < *(long double *)b));
}

size_t rdc_uint(const void *a){
  return *(size_t *)a;
}

/**
   Runs a heap_{push, pop, free} test with a ht_divchn_t hash table on
   size_t elements across priority types.
*/
void run_push_pop_free_divchn_uint_test(size_t log_ins,
					size_t alpha_n,
					size_t log_alpha_d){
  int i;
  size_t n;
  struct ht_divchn ht_divchn;
  struct heap_ht hht;
  n = pow_two_perror(log_ins);
  hht.ht = &ht_divchn;
  hht.alpha_n = alpha_n;
  hht.log_alpha_d = log_alpha_d;
  hht.init = ht_divchn_init_helper;
  hht.align = ht_divchn_align_helper;
  hht.insert = ht_divchn_insert_helper;
  hht.search = ht_divchn_search_helper;
  hht.remove = ht_divchn_remove_helper;
  hht.free = ht_divchn_free_helper;
  printf("Run a heap_{push, pop, free} test with a ht_divchn "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n),
	   (float)alpha_n / pow_two_perror(log_alpha_d), C_PTY_TYPES[i]);
    push_pop_free(n,
		  C_PTY_SIZES[i],
		  sizeof(size_t),
		  &hht,
		  C_NEW_PTY_ARR[i],
		  new_uint,
		  C_CMP_PTY_ARR[i],
		  cmp_uint,
		  rdc_uint,
		  NULL);
  }
}

/**
   Runs a heap_{update, search} test with a ht_divchn hash table on
   size_t elements across priority types.
*/
void run_update_search_divchn_uint_test(size_t log_ins,
					size_t alpha_n,
					size_t log_alpha_d){
  int i;
  size_t n;
  struct ht_divchn ht_divchn;
  struct heap_ht hht;
  n = pow_two_perror(log_ins);
  hht.ht = &ht_divchn;
  hht.alpha_n = alpha_n;
  hht.log_alpha_d = log_alpha_d;
  hht.init = ht_divchn_init_helper;
  hht.align = ht_divchn_align_helper;
  hht.insert = ht_divchn_insert_helper;
  hht.search = ht_divchn_search_helper;
  hht.remove = ht_divchn_remove_helper;
  hht.free = ht_divchn_free_helper;
  printf("Run a heap_{update, search} test with a ht_divchn "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n),
	   (float)alpha_n / pow_two_perror(log_alpha_d), C_PTY_TYPES[i]);
    update_search(n,
		  C_PTY_SIZES[i],
		  sizeof(size_t),
		  &hht,
		  C_NEW_PTY_ARR[i],
		  new_uint,
		  C_CMP_PTY_ARR[i],
		  cmp_uint,
		  rdc_uint,
		  NULL);
  }
}

/**
   Runs a heap_{push, pop, free} test with a ht_muloa hash table on
   size_t elements across priority types.
*/
void run_push_pop_free_muloa_uint_test(size_t log_ins,
				       size_t alpha_n,
				       size_t log_alpha_d){
  int i;
  size_t n;
  struct ht_muloa ht_muloa;
  struct heap_ht hht;
  n = pow_two_perror(log_ins);
  hht.ht = &ht_muloa;
  hht.alpha_n = alpha_n;
  hht.log_alpha_d = log_alpha_d;
  hht.init = ht_muloa_init_helper;
  hht.align = ht_muloa_align_helper;
  hht.insert = ht_muloa_insert_helper;
  hht.search = ht_muloa_search_helper;
  hht.remove = ht_muloa_remove_helper;
  hht.free = ht_muloa_free_helper;
  printf("Run a heap_{push, pop, free} test with a ht_muloa "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n),
	   (float)alpha_n / pow_two_perror(log_alpha_d),
	   C_PTY_TYPES[i]);
    push_pop_free(n,
		  C_PTY_SIZES[i],
		  sizeof(size_t),
		  &hht,
		  C_NEW_PTY_ARR[i],
		  new_uint,
		  C_CMP_PTY_ARR[i],
		  cmp_uint,
		  rdc_uint,
		  NULL);
  }
}

/**
   Runs a heap_{update, search} test with a ht_muloa hash table on
   size_t elements across priority types.
*/
void run_update_search_muloa_uint_test(size_t log_ins,
				       size_t alpha_n,
				       size_t log_alpha_d){
  int i;
  size_t n;
  struct ht_muloa ht_muloa;
  struct heap_ht hht;
  n = pow_two_perror(log_ins);
  hht.ht = &ht_muloa;
  hht.alpha_n = alpha_n;
  hht.log_alpha_d = log_alpha_d;
  hht.init = ht_muloa_init_helper;
  hht.align = ht_muloa_align_helper;
  hht.insert = ht_muloa_insert_helper;
  hht.search = ht_muloa_search_helper;
  hht.remove = ht_muloa_remove_helper;
  hht.free = ht_muloa_free_helper;
  printf("Run a heap_{update, search} test with a ht_muloa "
	 "hash table on size_t elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n),
	   (float)alpha_n / pow_two_perror(log_alpha_d),
	   C_PTY_TYPES[i]);
    update_search(n,
		  C_PTY_SIZES[i],
		  sizeof(size_t),
		  &hht,
		  C_NEW_PTY_ARR[i],
		  new_uint,
		  C_CMP_PTY_ARR[i],
		  cmp_uint,
		  rdc_uint,
		  NULL);
  }
}

/**
   Run heap_{push, pop, free} and heap_{update, search} tests with division-
   and mutliplication-based hash tables on noncontiguous uint_ptr
   elements across priority types. Because an element is noncontiguous, a
   pointer to an element is copied as an elt_size block. An element-specific
   free_elt is necessary to delete an element.
*/

struct uint_ptr{
  size_t *val;
};

void new_uint_ptr(void *a, size_t val){
  struct uint_ptr **s = a;
  *s = malloc_perror(1, sizeof(struct uint_ptr));
  (*s)->val = malloc_perror(1, sizeof(size_t));
  *((*s)->val) = val;
}

int cmp_uint_ptr(const void *a, const void *b){
  return
    ((*((*(struct uint_ptr **)a)->val) > *((*(struct uint_ptr **)b)->val)) -
     (*((*(struct uint_ptr **)a)->val) < *((*(struct uint_ptr **)b)->val)));
}

size_t rdc_uint_ptr(const void *a){
  return *((*(struct uint_ptr **)a)->val);
}

void free_uint_ptr(void *a){
  struct uint_ptr **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
}

/**
   Runs a heap_{push, pop, free} test with a ht_divchn hash table on
   noncontiguous uint_ptr elements across priority types.
*/
void run_push_pop_free_divchn_uint_ptr_test(size_t log_ins,
					    size_t alpha_n,
					    size_t log_alpha_d){
  int i;
  size_t n;
  struct ht_divchn ht_divchn;
  struct heap_ht hht;
  n = pow_two_perror(log_ins);
  hht.ht = &ht_divchn;
  hht.alpha_n = alpha_n;
  hht.log_alpha_d = log_alpha_d;
  hht.init = ht_divchn_init_helper;
  hht.align = ht_divchn_align_helper;
  hht.insert = ht_divchn_insert_helper;
  hht.search = ht_divchn_search_helper;
  hht.remove = ht_divchn_remove_helper;
  hht.free = ht_divchn_free_helper;
  printf("Run a heap_{push, pop, free} test with a ht_divchn "
	 "hash table on noncontiguous uint_ptr elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n),
	   (float)alpha_n / pow_two_perror(log_alpha_d), C_PTY_TYPES[i]);
    push_pop_free(n,
		  C_PTY_SIZES[i],
		  sizeof(struct uint_ptr *),
		  &hht,
		  C_NEW_PTY_ARR[i],
		  new_uint_ptr,
		  C_CMP_PTY_ARR[i],
		  cmp_uint_ptr,
		  rdc_uint_ptr,
		  free_uint_ptr);
  }
}

/**
   Runs a heap_{update, search} test with a ht_divchn hash table on
   noncontiguous uint_ptr elements across priority types.
*/
void run_update_search_divchn_uint_ptr_test(size_t log_ins,
					    size_t alpha_n,
					    size_t log_alpha_d){
  int i;
  size_t n;
  struct ht_divchn ht_divchn;
  struct heap_ht hht;
  n = pow_two_perror(log_ins);
  hht.ht = &ht_divchn;
  hht.alpha_n = alpha_n;
  hht.log_alpha_d = log_alpha_d;
  hht.init = ht_divchn_init_helper;
  hht.align = ht_divchn_align_helper;
  hht.insert = ht_divchn_insert_helper;
  hht.search = ht_divchn_search_helper;
  hht.remove = ht_divchn_remove_helper;
  hht.free = ht_divchn_free_helper;
  printf("Run a heap_{update, search} test with a ht_divchn "
	 "hash table on noncontiguous uint_ptr elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n),
	   (float)alpha_n / pow_two_perror(log_alpha_d), C_PTY_TYPES[i]);
    update_search(n,
		  C_PTY_SIZES[i],
		  sizeof(struct uint_ptr *),
		  &hht,
		  C_NEW_PTY_ARR[i],
		  new_uint_ptr,
		  C_CMP_PTY_ARR[i],
		  cmp_uint_ptr,
		  rdc_uint_ptr,
		  free_uint_ptr);
  }
}

/**
   Runs a heap_{push, pop, free} test with a ht_muloa hash table on
   noncontiguous uint_ptr elements across priority types.
*/
void run_push_pop_free_muloa_uint_ptr_test(size_t log_ins,
					   size_t alpha_n,
					   size_t log_alpha_d){
  int i;
  size_t n;
  struct ht_muloa ht_muloa;
  struct heap_ht hht;
  n = pow_two_perror(log_ins);
  hht.ht = &ht_muloa;
  hht.alpha_n = alpha_n;
  hht.log_alpha_d = log_alpha_d;
  hht.init = ht_muloa_init_helper;
  hht.align = ht_muloa_align_helper;
  hht.insert = ht_muloa_insert_helper;
  hht.search = ht_muloa_search_helper;
  hht.remove = ht_muloa_remove_helper;
  hht.free = ht_muloa_free_helper;
  printf("Run a heap_{push, pop, free} test with a ht_muloa "
	 "hash table on noncontiguous uint_ptr elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n),
	   (float)alpha_n / pow_two_perror(log_alpha_d),
	   C_PTY_TYPES[i]);
    push_pop_free(n,
		  C_PTY_SIZES[i],
		  sizeof(struct uint_ptr *),
		  &hht,
		  C_NEW_PTY_ARR[i],
		  new_uint_ptr,
		  C_CMP_PTY_ARR[i],
		  cmp_uint_ptr,
		  rdc_uint_ptr,
		  free_uint_ptr);
  }
}

/**
   Runs a heap_{update, search} test with a ht_muloa hash table on
   noncontiguous uint_ptr elements across priority types.
*/
void run_update_search_muloa_uint_ptr_test(size_t log_ins,
					   size_t alpha_n,
					   size_t log_alpha_d){
  int i;
  size_t n;
  struct ht_muloa ht_muloa;
  struct heap_ht hht;
  n = pow_two_perror(log_ins);
  hht.ht = &ht_muloa;
  hht.alpha_n = alpha_n;
  hht.log_alpha_d = log_alpha_d;
  hht.init = ht_muloa_init_helper;
  hht.align = ht_muloa_align_helper;
  hht.insert = ht_muloa_insert_helper;
  hht.search = ht_muloa_search_helper;
  hht.remove = ht_muloa_remove_helper;
  hht.free = ht_muloa_free_helper;
  printf("Run a heap_{update, search} test with a ht_muloa "
	 "hash table on noncontiguous uint_ptr elements\n");
  for (i = 0; i < C_PTY_TYPES_COUNT; i++){
    printf("\tnumber of elements:      %lu\n"
	   "\tload factor upper bound: %.4f\n"
	   "\tpriority type:           %s\n",
	   TOLU(n),
	   (float)alpha_n / pow_two_perror(log_alpha_d),
	   C_PTY_TYPES[i]);
    update_search(n,
		  C_PTY_SIZES[i],
		  sizeof(struct uint_ptr *),
		  &hht,
		  C_NEW_PTY_ARR[i],
		  new_uint_ptr,
		  C_CMP_PTY_ARR[i],
		  cmp_uint_ptr,
		  rdc_uint_ptr,
		  free_uint_ptr);
  }
}

/** 
   Helper functions for heap_{push, pop, free} tests.
*/

void push_ptys_elts(struct heap *h,
		    const void *pty_elts,
		    size_t count,
                    int *res){
  size_t half_count;
  size_t n = h->num_elts;
  const void *p = NULL, *p_start = NULL, *p_end = NULL;
  clock_t t_first, t_second;
  half_count = count >> 1;  /* count > 0 */
  p_start = pty_elts;
  p_end = ptr(pty_elts, half_count, h->pair_size);
  t_first = clock();
  for (p = p_start; p != p_end; p = (char *)p + h->pair_size){
    heap_push(h, p, (char *)p + h->elt_offset);
  }
  t_first = clock() - t_first;
  p_start = ptr(pty_elts, half_count, h->pair_size);
  p_end = ptr(pty_elts, count, h->pair_size);
  t_second = clock();
  for (p = p_start; p != p_end; p = (char *)p + h->pair_size){
    heap_push(h, p, (char *)p + h->elt_offset);
  }
  t_second = clock() - t_second;
  printf("\t\tpush 1/2 elements:                           "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tpush residual elements:                      "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + count);
}

void push_rev_ptys_elts(struct heap *h,
			const void *pty_elts,
			size_t count,
                        int *res){
  const void *p = NULL, *p_start = NULL, *p_end = NULL;
  size_t half_count;
  size_t n = h->num_elts;
  clock_t t_first, t_second;
  half_count = count >> 1;
  /* backwards pointer iteration; count > 0 */
  p_start = ptr(pty_elts, count - 1, h->pair_size);
  p_end = ptr(pty_elts, half_count, h->pair_size);
  t_first = clock();
  for (p = p_start; p != p_end; p = (char *)p - h->pair_size){
    heap_push(h, p, (char *)p + h->elt_offset);
  }
  t_first = clock() - t_first;
  p_start = ptr(pty_elts, half_count, h->pair_size);
  p_end = pty_elts;
  t_second = clock();
  for (p = p_start; p >= p_end; p = (char *)p - h->pair_size){
    heap_push(h, p, (char *)p + h->elt_offset);
  }
  t_second = clock() - t_second;
  printf("\t\tpush 1/2 elements, rev. pty order:           "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tpush residual elements, rev. pty order:      "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n + count);
}

void pop_ptys_elts(struct heap *h,
		   const void *pty_elts,
		   size_t count,
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *),
                   int *res){
  size_t i, half_count;
  size_t n = h->num_elts;
  void *p = NULL, *p_start = NULL, *p_end = NULL;
  void *pop_pty_elts = NULL;
  clock_t t_first, t_second;
  half_count = count >> 1; /* count > 0 */
  pop_pty_elts = malloc_perror(count, h->pair_size);
  p_start = pop_pty_elts;
  p_end = ptr(pop_pty_elts, half_count, h->pair_size);
  t_first = clock();
  for (p = p_start; p != p_end; p = (char *)p + h->pair_size){
    heap_pop(h, p, (char *)p + h->elt_offset);
  }
  t_first = clock() - t_first;
  p_start = ptr(pop_pty_elts, half_count, h->pair_size);
  p_end = ptr(pop_pty_elts, count, h->pair_size);
  t_second = clock();
  for (p = p_start; p != p_end; p = (char *)p + h->pair_size){
    heap_pop(h, p, (char *)p + h->elt_offset);
  }
  t_second = clock() - t_second;
  *res *= (h->num_elts == n - count);
  for (i = 0; i < count; i++){
    if (i == 0){
      *res *=
	(cmp_elt((char *)ptr(pop_pty_elts, i, h->pair_size) + h->elt_offset,
		 (char *)ptr(pty_elts, i, h->pair_size) + h->elt_offset) == 0);
    }else{
      *res *=
	(cmp_pty(ptr(pop_pty_elts, i, h->pair_size),
		 ptr(pop_pty_elts, i - 1, h->pair_size)) >= 0);
      *res *=
	(cmp_elt((char *)ptr(pop_pty_elts, i, h->pair_size) + h->elt_offset,
		 (char *)ptr(pty_elts, i, h->pair_size) + h->elt_offset) == 0);
    }
  }
  printf("\t\tpop 1/2 elements:                            "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tpop residual elements:                       "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  free(pop_pty_elts);
  pop_pty_elts = NULL;
}

void free_heap(struct heap *h){
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

void update_ptys_elts(struct heap *h,
		      const void *pty_elts,
		      size_t count,
                      int *res){
  size_t half_count = count >> 1;
  size_t n = h->num_elts;
  const void *p = NULL, *p_start = NULL, *p_end = NULL;
  clock_t t_first, t_second;
  p_start = pty_elts;
  p_end = ptr(pty_elts, half_count, h->pair_size);
  t_first = clock();
  for (p = p_start; p != p_end; p = (char *)p + h->pair_size){
    heap_update(h, p, (char *)p + h->elt_offset);
  }
  t_first = clock() - t_first;
  *res *= (h->num_elts == n);
  p_start = ptr(pty_elts, half_count, h->pair_size);
  p_end = ptr(pty_elts, count, h->pair_size);
  t_second = clock();
  for (p = p_start; p != p_end; p = (char *)p + h->pair_size){
    heap_update(h, p, (char *)p + h->elt_offset);
  }
  t_second = clock() - t_second;
  printf("\t\tupdate 1/2 elements:                         "
	 "%.4f seconds\n", (float)t_first / CLOCKS_PER_SEC);
  printf("\t\tupdate residual elements:                    "
	 "%.4f seconds\n", (float)t_second / CLOCKS_PER_SEC);
  *res *= (h->num_elts == n);
}

void search_ptys_elts(const struct heap *h,
		      const void *pty_elts,
		      const void *not_heap_elts,
		      size_t count,
                      int *res){
  size_t n = h->num_elts;
  void *rp = NULL;
  const void *p = NULL, *p_start = NULL, *p_end = NULL;
  clock_t t_heap, t_not_heap;
  p_start = pty_elts;
  p_end = ptr(pty_elts, count, h->pair_size);
  t_heap = clock();
  for (p = p_start; p != p_end; p = (char *)p + h->pair_size){
    rp = heap_search(h, (char *)p + h->elt_offset);
  }
  t_heap = clock() - t_heap;
  for (p = p_start; p != p_end; p = (char *)p + h->pair_size){
    rp = heap_search(h, (char *)p + h->elt_offset);
    *res *= (rp != NULL);
  }
  *res *= (h->num_elts == n);
  p_start = not_heap_elts;
  p_end = ptr(not_heap_elts, count, h->elt_size);
  t_not_heap = clock();
  for (p = p_start; p != p_end; p = (char *)p + h->elt_size){
    rp = heap_search(h, p);
  }
  t_not_heap = clock() - t_not_heap;
  for (p = p_start; p != p_end; p = (char *)p + h->elt_size){
    rp = heap_search(h, p);
    *res *= (rp == NULL);
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

void push_pop_free(size_t num_ins,
		   size_t pty_size,
		   size_t elt_size,
		   const struct heap_ht *hht,
		   void (*new_pty)(void *, size_t),
		   void (*new_elt)(void *, size_t),
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *),
		   size_t (*rdc_elt)(const void *),
		   void (*free_elt)(void *)){
  int res = 1;
  size_t i;
  void *pty_elts = NULL;
  struct heap h;
  heap_init(&h,
	    pty_size,
	    elt_size,
	    C_H_MIN_NUM,
	    hht,
	    cmp_pty,
	    cmp_elt,
	    rdc_elt,
	    free_elt);
  /* num_ins > 0 */
  pty_elts = malloc_perror(num_ins, h.pair_size);
  for (i = 0; i < num_ins; i++){
    new_pty(ptr(pty_elts, i, h.pair_size), i); /* no decrease with i */
    new_elt((char *)ptr(pty_elts, i, h.pair_size) + h.elt_offset, i);
  }
  push_ptys_elts(&h, pty_elts, num_ins, &res);
  pop_ptys_elts(&h, pty_elts, num_ins, cmp_pty, cmp_elt, &res);
  push_rev_ptys_elts(&h, pty_elts, num_ins, &res);
  pop_ptys_elts(&h, pty_elts, num_ins, cmp_pty, cmp_elt, &res);
  push_ptys_elts(&h, pty_elts, num_ins, &res);
  free_heap(&h);
  printf("\t\torder correctness:                           ");
  print_test_result(res);
  free(pty_elts);
  pty_elts = NULL;
}

void update_search(size_t num_ins,
		   size_t pty_size,
		   size_t elt_size,
		   const struct heap_ht *hht,
		   void (*new_pty)(void *, size_t),
		   void (*new_elt)(void *, size_t),
		   int (*cmp_pty)(const void *, const void *),
		   int (*cmp_elt)(const void *, const void *),
		   size_t (*rdc_elt)(const void *),
		   void (*free_elt)(void *)){
  int res = 1;
  size_t i;
  void *pty_elts = NULL, *pty_rev_elts = NULL, *not_heap_elts = NULL;
  struct heap h;
  heap_init(&h,
	    pty_size,
	    elt_size,
	    C_H_MIN_NUM,
	    hht, cmp_pty,
	    cmp_elt,
	    rdc_elt,
	    free_elt);
   /* num_ins > 0 */
  pty_elts = malloc_perror(num_ins, h.pair_size);
  pty_rev_elts = malloc_perror(num_ins, h.pair_size);
  not_heap_elts = malloc_perror(num_ins, elt_size);
  for (i = 0; i < num_ins; i++){
    new_pty(ptr(pty_elts, i, h.pair_size), i);  /* no decrease with i */
    new_elt((char *)ptr(pty_elts, i, h.pair_size) + h.elt_offset, i);
    new_elt(ptr(not_heap_elts, i, elt_size), num_ins + i);
  }
  for (i = 0; i < num_ins; i++){
    new_pty(ptr(pty_rev_elts, i, h.pair_size), i);  /* no decrease with i */
    memcpy((char *)ptr(pty_rev_elts, i, h.pair_size) + h.elt_offset,
	   (char *)ptr(pty_elts, num_ins - 1 - i, h.pair_size) + h.elt_offset,
	   elt_size);
  }
  push_ptys_elts(&h, pty_rev_elts, num_ins, &res);
  update_ptys_elts(&h, pty_elts, num_ins, &res);
  search_ptys_elts(&h, pty_elts, not_heap_elts, num_ins, &res);
  pop_ptys_elts(&h, pty_elts, num_ins, cmp_pty, cmp_elt, &res);
  free_heap(&h);
  printf("\t\torder correctness:                           ");
  print_test_result(res);
  if (free_elt != NULL){
    for (i = 0; i < num_ins; i++){
      free_elt((char *)ptr(pty_elts, i, h.pair_size) + h.elt_offset);
      free_elt(ptr(not_heap_elts, i, elt_size));
    }
  }
  free(pty_elts);
  free(pty_rev_elts);
  free(not_heap_elts);
  pty_elts = NULL;
  pty_rev_elts = NULL;
  not_heap_elts = NULL;
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
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
      args[2] > C_FULL_BIT - 1 ||
      args[3] < 1 ||
      args[4] > C_FULL_BIT - 1 ||
      args[3] > pow_two_perror(args[4]) ||
      args[5] > 1 ||
      args[6] > 1 ||
      args[7] > 1 ||
      args[8] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[5]){
    run_push_pop_free_divchn_uint_test(args[0], args[1], args[2]);
    run_push_pop_free_divchn_uint_ptr_test(args[0], args[1], args[2]);
  }
  if (args[6]){
    run_update_search_divchn_uint_test(args[0], args[1], args[2]);
    run_update_search_divchn_uint_ptr_test(args[0], args[1], args[2]);
  }
  if (args[7]){
    run_push_pop_free_muloa_uint_test(args[0], args[3], args[4]);
    run_push_pop_free_muloa_uint_ptr_test(args[0], args[3], args[4]);
  }
  if (args[8]){
    run_update_search_muloa_uint_test(args[0], args[3], args[4]);
    run_update_search_muloa_uint_ptr_test(args[0], args[3], args[4]);
  }
  free(args);
  args = NULL;
  return 0;
}
