/**
   ht-muloa-test.c

   Tests of a hash table with generic hash keys and generic elements.
   The implementation is based on a multiplication method for hashing and an 
   open addressing method for resolving collisions.

   The following command line arguments can be used to customize tests:
   ht-muloa-test
      [0, # bits in size_t - 1) : i s.t. # inserts = 2**i
      [0, # bits in size_t) : a given k = sizeof(size_t)
      [0, # bits in size_t) : b s.t. k * 2**a <= key size <= k * 2**b
      > 0 : c
      > 0 : d
      > 0 : e log base 2 s.t. c <= d <= 2**e
      > 0 : f s.t. c / 2**e <= alpha <= d / 2**e, in f steps
      [0, 1] : on/off insert search uint test
      [0, 1] : on/off remove delete uint test
      [0, 1] : on/off insert search uint_ptr test
      [0, 1] : on/off remove delete uint_ptr test
      [0, 1] : on/off corner cases test

   usage examples:
   ./ht-muloa-test
   ./ht-muloa-test 18
   ./ht-muloa-test 17 5 6 
   ./ht-muloa-test 19 0 2 3000 4000 15 10
   ./ht-muloa-test 19 0 2 3000 4000 15 10 1 1 0 0 0

   ht-muloa-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99 with the only requirement that CHAR_BIT * sizeof(size_t)
   is greater or equal to 16 and is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "ht-muloa.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

/**
   Generate random numbers in a portable way for test purposes only; rand()
   in the Linux C Library uses the same generator as random(), which may not
   be the case on older rand() implementations, and on current
   implementations on different systems.
*/
#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand()) /* [0, RAND_MAX] */
#define DRAND() ((double)rand() / RAND_MAX) /* [0.0, 1.0] */

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "ht-muloa-test\n"
  "[0, # bits in size_t - 1) : i s.t. # inserts = 2**i\n"
  "[0, # bits in size_t) : a given k = sizeof(size_t)\n"
  "[0, # bits in size_t) : b s.t. k * 2**a <= key size <= k * 2**b\n"
  "> 0 : c\n"
  "> 0 : d\n"
  "> 0 : e log base 2 s.t. c <= d <= 2**e\n"
  "> 0 : f s.t. c / 2**e <= alpha <= d / 2**e, in f steps\n"
  "[0, 1] : on/off insert search uint test\n"
  "[0, 1] : on/off remove delete uint test\n"
  "[0, 1] : on/off insert search uint_ptr test\n"
  "[0, 1] : on/off remove delete uint_ptr test\n"
  "[0, 1] : on/off corner cases test\n";
const int C_ARGC_MAX = 13;
const size_t C_ARGS_DEF[12] = {14, 0, 2, 3277, 32768u, 15, 8, 1, 1, 1, 1, 1};
const size_t C_SIZE_MAX = (size_t)-1;
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);

/* insert, search, free, remove, delete tests */
const size_t C_KEY_SIZE_FACTOR = sizeof(size_t);

/* corner cases test */
const unsigned char C_CORNER_KEY_A = 2;
const unsigned char C_CORNER_KEY_B = 1;
const size_t C_CORNER_KEY_SIZE = sizeof(unsigned char);
const size_t C_CORNER_HT_COUNT = 2048;
const size_t C_CORNER_ALPHA_N = 33;
const size_t C_CORNER_LOG_ALPHA_D = 15; /* alpha is 33/32768 */

void insert_search_free(size_t num_ins,
			size_t key_size,
			size_t elt_size,
			size_t alpha_n,
			size_t log_alpha_d,
                        size_t (*rdc_key)(const void *, size_t),
			void (*new_elt)(void *, size_t),
			size_t (*val_elt)(const void *),
			void (*free_elt)(void *));
void remove_delete(size_t num_ins,
		   size_t key_size,
		   size_t elt_size,
		   size_t alpha,
		   size_t log_alpha_d,
                   size_t (*rdc_key)(const void *, size_t),
		   void (*new_elt)(void *, size_t),
		   size_t (*val_elt)(const void *),
		   void (*free_elt)(void *));
void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);

/**
   Test hash table operations on distinct keys and size_t elements 
   across key sizes and load factor upper bounds. For test purposes a key
   is random with the exception of a distinct non-random C_KEY_SIZE_FACTOR-
   sized block inside the key. A pointer to an element is passed as elt in
   ht_muloa_insert and the element is fully copied into the hash table.
   NULL as free_elt is sufficient to delete the element.
*/

void new_uint(void *elt, size_t val){
  size_t *s = elt;
  *s = val;
}

size_t val_uint(const void *elt){
  return *(size_t *)elt;
}

/**
   Runs a ht_muloa_{insert, search, free} test on distinct keys and 
   size_t elements across key sizes >= C_KEY_SIZE_FACTOR and load factor
   upper bounds.
*/
void run_insert_search_free_uint_test(size_t log_ins,
				      size_t log_key_start,
				      size_t log_key_end,
				      size_t alpha_n_start,
				      size_t alpha_n_end,
                                      size_t log_alpha_d,
				      size_t num_alpha_steps){
  size_t i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(size_t);
  size_t step, rem;
  size_t alpha_n;
  num_ins = pow_two_perror(log_ins);
  step = (alpha_n_end - alpha_n_start) / num_alpha_steps;
  for (i = log_key_start; i <= log_key_end; i++){
    alpha_n = alpha_n_start;
    rem = alpha_n_end - alpha_n_start - step * num_alpha_steps;
    key_size = C_KEY_SIZE_FACTOR * pow_two_perror(i);
    printf("Run a ht_muloa_{insert, search, free} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      insert_search_free(num_ins,
			 key_size,
			 elt_size,
			 alpha_n,
			 log_alpha_d,
                         NULL,
			 new_uint,
			 val_uint,
			 NULL);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Runs a ht_muloa_{remove, delete} test on distinct keys and size_t
   elements across key sizes >= C_KEY_SIZE_FACTOR and load factor upper
   bounds.
*/
void run_remove_delete_uint_test(size_t log_ins,
				 size_t log_key_start,
				 size_t log_key_end,
				 size_t alpha_n_start,
				 size_t alpha_n_end,
				 size_t log_alpha_d,
				 size_t num_alpha_steps){
  size_t i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(size_t);
  size_t step, rem;
  size_t alpha_n;
  num_ins = pow_two_perror(log_ins);
  step = (alpha_n_end - alpha_n_start) / num_alpha_steps;
  for (i = log_key_start; i <= log_key_end; i++){
    alpha_n = alpha_n_start;
    rem = alpha_n_end - alpha_n_start - step * num_alpha_steps;
    key_size = C_KEY_SIZE_FACTOR * pow_two_perror(i);
    printf("Run a ht_muloa_{remove, delete} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      remove_delete(num_ins,
		    key_size,
		    elt_size,
		    alpha_n,
		    log_alpha_d,
                    NULL,
		    new_uint,
		    val_uint,
		    NULL);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Test hash table operations on distinct keys and noncontiguous
   uint_ptr_t elements across key sizes and load factor upper bounds. 
   For test purposes a key is random with the exception of a distinct
   non-random C_KEY_SIZE_FACTOR-sized block inside the key. A pointer to a
   pointer to an element is passed as elt in ht_muloa_insert, and the pointer
   to the element is copied into the hash table. An element-specific
   free_elt is necessary to delete the element (see specification).
*/

typedef struct{
  size_t *val;
} uint_ptr_t;

void new_uint_ptr(void *elt, size_t val){
  uint_ptr_t **s = elt;
  *s = malloc_perror(1, sizeof(uint_ptr_t));
  (*s)->val = malloc_perror(1, sizeof(size_t));
  *((*s)->val) = val;
}

size_t val_uint_ptr(const void *elt){
  uint_ptr_t **s  = (uint_ptr_t **)elt;
  return *((*s)->val);
}

void free_uint_ptr(void *elt){
  uint_ptr_t **s = elt;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
}

/**
   Runs a ht_muloa_{insert, search, free} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_insert_search_free_uint_ptr_test(size_t log_ins,
					  size_t log_key_start,
					  size_t log_key_end,
					  size_t alpha_n_start,
					  size_t alpha_n_end,
					  size_t log_alpha_d,
					  size_t num_alpha_steps){
  size_t i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size =  sizeof(uint_ptr_t *);
  size_t step, rem;
  size_t alpha_n;
  num_ins = pow_two_perror(log_ins);
  step = (alpha_n_end - alpha_n_start) / num_alpha_steps;
  for (i = log_key_start; i <= log_key_end; i++){
    alpha_n = alpha_n_start;
    rem = alpha_n_end - alpha_n_start - step * num_alpha_steps;
    key_size = C_KEY_SIZE_FACTOR * pow_two_perror(i);
    printf("Run a ht_muloa_{insert, search, free} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      insert_search_free(num_ins,
			 key_size,
			 elt_size,
			 alpha_n,
			 log_alpha_d,
                         NULL,
			 new_uint_ptr,
			 val_uint_ptr,
			 free_uint_ptr);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Runs a ht_muloa_{remove, delete} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_remove_delete_uint_ptr_test(size_t log_ins,
				     size_t log_key_start,
				     size_t log_key_end,
				     size_t alpha_n_start,
				     size_t alpha_n_end,
				     size_t log_alpha_d,
				     size_t num_alpha_steps){
  size_t i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(uint_ptr_t *);
  size_t step, rem;
  size_t alpha_n;
  num_ins = pow_two_perror(log_ins);
  step = (alpha_n_end - alpha_n_start) / num_alpha_steps;
  for (i = log_key_start; i <= log_key_end; i++){
    alpha_n = alpha_n_start;
    rem = alpha_n_end - alpha_n_start - step * num_alpha_steps;
    key_size = C_KEY_SIZE_FACTOR * pow_two_perror(i);
    printf("Run a ht_muloa_{remove, delete} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      remove_delete(num_ins,
		    key_size,
		    elt_size,
		    alpha_n,
		    log_alpha_d,
                    NULL,
		    new_uint_ptr,
		    val_uint_ptr,
		    free_uint_ptr);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/** 
   Helper functions for the ht_muloa_{insert, search, free} tests
   across key sizes and load factor upper bounds, on size_t and 
   uint_ptr_t elements.
*/

void insert_keys_elts(ht_muloa_t *ht,
		      const void *key_elts,
		      size_t count,
		      int *res){
  const char *p = NULL, *p_start = NULL, *p_end = NULL;
  size_t n = ht->num_elts;
  size_t init_count = ht->count;
  clock_t t;
  p_start = key_elts;
  p_end = ptr(key_elts, count, ht->pair_size);
  t = clock();
  for (p = p_start; p != p_end; p += ht->pair_size){
    ht_muloa_insert(ht, p, p + ht->key_size);
  }
  t = clock() - t;
  if (init_count < ht->count){
    printf("\t\tinsert w/ growth time           "
	   "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  }else{
    printf("\t\tinsert w/o growth time          "
	   "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  }
  *res *= (ht->num_elts == n + count);
}

void search_in_ht(const ht_muloa_t *ht,
		  const void *key_elts,
		  size_t count,
		  size_t (*val_elt)(const void *),
                  int *res){
  const char *p = NULL, *p_start = NULL, *p_end = NULL;
  size_t n = ht->num_elts;
  const void *elt = NULL;
  clock_t t;
  p_start = key_elts;
  p_end = ptr(key_elts, count, ht->pair_size);
  t = clock();
  for (p = p_start; p != p_end; p += ht->pair_size){
    elt = ht_muloa_search(ht, p);
  }
  t = clock() - t;
  for (p = p_start; p != p_end; p += ht->pair_size){
    elt = ht_muloa_search(ht, p);
    *res *= (val_elt(p + ht->key_size) == val_elt(elt));
  }
  printf("\t\tin ht search time:              "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void search_nin_ht(const ht_muloa_t *ht,
		   const void *nin_keys,
		   size_t count,
		   int *res){
  const char *p = NULL, *p_start = NULL, *p_end = NULL;
  size_t n = ht->num_elts;
  const void *elt = NULL;
  clock_t t;
  p_start = nin_keys;
  p_end = ptr(nin_keys, count, ht->key_size);
  t = clock();
  for (p = p_start; p != p_end; p += ht->key_size){
    elt = ht_muloa_search(ht, p);
  }
  t = clock() - t;
  for (p = p_start; p != p_end; p += ht->key_size){
    elt = ht_muloa_search(ht, p);
    *res *= (elt == NULL);
  }
  printf("\t\tnot in ht search time:          "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void free_ht(ht_muloa_t *ht){
  clock_t t;
  t = clock();
  ht_muloa_free(ht);
  t = clock() - t;
  printf("\t\tfree time:                      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}
void insert_search_free(size_t num_ins,
			size_t key_size,
			size_t elt_size,
			size_t alpha_n,
			size_t log_alpha_d,
                        size_t (*rdc_key)(const void *, size_t),
			void (*new_elt)(void *, size_t),
			size_t (*val_elt)(const void *),
			void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  size_t pair_size = add_sz_perror(key_size, elt_size);
  void *key = NULL;
  void *key_elts = NULL;
  void *nin_keys = NULL;
  ht_muloa_t ht;
  key_elts = malloc_perror(num_ins, pair_size);
  nin_keys = malloc_perror(num_ins, key_size);
  for (i = 0; i < num_ins; i++){
    key = ptr(key_elts, i, pair_size);
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      *(unsigned char *)ptr(key, j, 1) = RANDOM(); /* mod 2^CHAR_BIT */
    }
    *(size_t *)ptr(key, key_size - C_KEY_SIZE_FACTOR, 1) = i;
    new_elt((char *)ptr(key_elts, i, pair_size) + key_size, i);
  }
  ht_muloa_init(&ht,
		key_size,
		elt_size,
		0,
		alpha_n,
		log_alpha_d,
		rdc_key,
		NULL);
  insert_keys_elts(&ht, key_elts, num_ins, &res);
  free_ht(&ht);
  ht_muloa_init(&ht,
		key_size,
		elt_size,
		num_ins,
		alpha_n,
		log_alpha_d,
		rdc_key,
		free_elt);
  insert_keys_elts(&ht, key_elts, num_ins, &res);
  search_in_ht(&ht, key_elts, num_ins, val_elt, &res);
  for (i = 0; i < num_ins; i++){
    key = ptr(nin_keys, i, key_size);
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      *(unsigned char *)ptr(key, j, 1) = RANDOM(); /* mod 2^CHAR_BIT */
    }
    *(size_t *)ptr(key, key_size - C_KEY_SIZE_FACTOR, 1) = i + num_ins;
  }
  search_nin_ht(&ht, nin_keys, num_ins, &res);
  free_ht(&ht);
  printf("\t\tsearch correctness:             ");
  print_test_result(res);
  free(key_elts);
  free(nin_keys);
  key_elts = NULL;
  nin_keys = NULL;
}

/** 
   Helper functions for the ht_muloa_{remove, delete} tests
   across key sizes and load factor upper bounds, on size_t and 
   uint_ptr_t elements.
*/

void remove_key_elts(ht_muloa_t *ht,
		     const void *key_elts,
		     size_t count,
		     size_t (*val_elt)(const void *),
		     int *res){
  const char *p = NULL, *p_start = NULL, *p_end = NULL;
  size_t n = ht->num_elts;
  size_t step_size = mul_sz_perror(2, ht->pair_size);
  size_t i;
  void *elt = NULL;
  clock_t t_first_half, t_second_half;
  elt = malloc_perror(1, ht->elt_size);
  p = key_elts;
  t_first_half = clock();
  for (i = 0; i < count; i += 2){ /* count < SIZE_MAX */
    p += (i > 0) * step_size; /* avoid undef. behavior of pointer increment */
    ht_muloa_remove(ht, p, elt);
    /* noncontiguous element is still accessible from key_elts */
  }
  t_first_half = clock() - t_first_half;
  *res *= (ht->num_elts == ((count & 1) ?
			    (n - count / 2 - 1) :
			    (n - count / 2)));
  p_start = key_elts;
  p_end = ptr(key_elts, count, ht->pair_size);
  i = 0;
  for (p = p_start; p != p_end; p += ht->pair_size){
    if (1 & i++){
      *res *= (val_elt(p + ht->key_size) == val_elt(ht_muloa_search(ht, p)));
    }else{
      *res *= (ht_muloa_search(ht, p) == NULL);
    }
  }
  p = ptr(key_elts, (count > 0), ht->pair_size);
  t_second_half = clock();
  for (i = 1; i < count; i += 2){ /* count < SIZE_MAX */
    p += (i > 1) * step_size; /* avoid undef. behavior of pointer increment */
    ht_muloa_remove(ht, p, elt);
    /* noncontiguous element is still accessible from key_elts */
  }
  t_second_half = clock() - t_second_half;
  *res *= (ht->num_elts == 0);
  p_start = key_elts;
  p_end = ptr(key_elts, count, ht->pair_size);
  for (p = p_start; p != p_end; p += ht->pair_size){
    *res *= (ht_muloa_search(ht, p) == NULL);
  }
  printf("\t\tremove 1/2 elements time:       "
	 "%.4f seconds\n", (float)t_first_half / CLOCKS_PER_SEC);
  printf("\t\tremove residual elements time:  "
	 "%.4f seconds\n", (float)t_second_half / CLOCKS_PER_SEC);
  free(elt);
  elt = NULL;
}

void delete_key_elts(ht_muloa_t *ht,
		     const void *key_elts,
		     size_t count,
		     size_t (*val_elt)(const void *),
                     int *res){
  const char *p = NULL, *p_start = NULL, *p_end = NULL;
  size_t n = ht->num_elts;
  size_t step_size = mul_sz_perror(2, ht->pair_size);
  size_t i;
  clock_t t_first_half, t_second_half;
  p = key_elts;
  t_first_half = clock();
  for (i = 0; i < count; i += 2){ /* count < SIZE_MAX */
    p += (i > 0) * step_size; /* avoid undef. behavior of pointer increment */
    ht_muloa_delete(ht, p);
  }
  t_first_half = clock() - t_first_half;
  *res *= (ht->num_elts == ((count & 1) ?
			    (n - count / 2 - 1) :
			    (n - count / 2)));
  p_start = key_elts;
  p_end = ptr(key_elts, count, ht->pair_size);
  i = 0;
  for (p = p_start; p != p_end; p += ht->pair_size){
    if (1 & i++){
      *res *= (val_elt(p + ht->key_size) == val_elt(ht_muloa_search(ht, p)));
    }else{
      *res *= (ht_muloa_search(ht, p) == NULL);
    }
  }
  p = ptr(key_elts, (count > 0), ht->pair_size);
  t_second_half = clock();
  for (i = 1; i < count; i += 2){ /* count < SIZE_MAX */
    p += (i > 1) * step_size; /* avoid undef. behavior of pointer increment */
    ht_muloa_delete(ht, p);
  }
  t_second_half = clock() - t_second_half;
  *res *= (ht->num_elts == 0);
  p_start = key_elts;
  p_end = ptr(key_elts, count, ht->pair_size);
  for (p = p_start; p != p_end; p += ht->pair_size){
    *res *= (ht_muloa_search(ht, p) == NULL);
  }
  printf("\t\tdelete 1/2 elements time:       "
	 "%.4f seconds\n", (float)t_first_half / CLOCKS_PER_SEC);
  printf("\t\tdelete residual elements time:  "
	 "%.4f seconds\n", (float)t_second_half / CLOCKS_PER_SEC);
}
void remove_delete(size_t num_ins,
		   size_t key_size,
		   size_t elt_size,
		   size_t alpha_n,
		   size_t log_alpha_d,
                   size_t (*rdc_key)(const void *, size_t),
		   void (*new_elt)(void *, size_t),
		   size_t (*val_elt)(const void *),
		   void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  size_t pair_size = add_sz_perror(key_size, elt_size);
  void *key = NULL;
  void *key_elts = NULL;
  ht_muloa_t ht;
  key_elts = malloc_perror(num_ins, pair_size);
  for (i = 0; i < num_ins; i++){
    key = ptr(key_elts, i, pair_size);
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      *(unsigned char *)ptr(key, j, 1) = RANDOM(); /* mod 2^CHAR_BIT */
    }
    *(size_t *)ptr(key, key_size - C_KEY_SIZE_FACTOR, 1) = i;
    new_elt((char *)ptr(key_elts, i, pair_size) + key_size, i);
  }
  ht_muloa_init(&ht,
		key_size,
		elt_size,
		0,
		alpha_n,
		log_alpha_d,
		rdc_key,
		free_elt);
  insert_keys_elts(&ht, key_elts, num_ins, &res);
  remove_key_elts(&ht, key_elts, num_ins, val_elt, &res);
  insert_keys_elts(&ht, key_elts, num_ins, &res);
  delete_key_elts(&ht, key_elts, num_ins, val_elt, &res);
  free_ht(&ht);
  printf("\t\tremove and delete correctness:  ");
  print_test_result(res);
  free(key_elts);
  key_elts = NULL;
}

/**
   Runs a corner cases test.
*/
void run_corner_cases_test(int log_ins){
  int res = 1;
  size_t elt;
  size_t elt_size = sizeof(size_t);
  size_t i, num_ins;
  ht_muloa_t ht;
  ht_muloa_init(&ht,
		C_CORNER_KEY_SIZE,
		elt_size,
		0,
		C_CORNER_ALPHA_N,
		C_CORNER_LOG_ALPHA_D,
		NULL,
		NULL);
  num_ins = pow_two_perror(log_ins);
  printf("Run corner cases test --> ");
  for (i = 0; i < num_ins; i++){
    elt = i;
    ht_muloa_insert(&ht, &C_CORNER_KEY_A, &elt);
  }
  res *= (ht.num_elts == 1);
  res *= (*(size_t *)ht_muloa_search(&ht, &C_CORNER_KEY_A) == elt);
  res *= (ht_muloa_search(&ht, &C_CORNER_KEY_B) == NULL);
  ht_muloa_insert(&ht, &C_CORNER_KEY_B, &elt);
  res *= (ht.count == C_CORNER_HT_COUNT);
  res *= (ht.num_elts == 2);
  res *= (*(size_t *)ht_muloa_search(&ht, &C_CORNER_KEY_A) == elt);
  res *= (*(size_t *)ht_muloa_search(&ht, &C_CORNER_KEY_B) == elt);
  ht_muloa_delete(&ht, &C_CORNER_KEY_A);
  res *= (ht.count == C_CORNER_HT_COUNT);
  res *= (ht.num_elts == 1);
  res *= (ht_muloa_search(&ht, &C_CORNER_KEY_A) == NULL);
  res *= (*(size_t *)ht_muloa_search(&ht, &C_CORNER_KEY_B) == elt);
  ht_muloa_delete(&ht, &C_CORNER_KEY_B);
  res *= (ht.count == C_CORNER_HT_COUNT);
  res *= (ht.num_elts == 0);
  res *= (ht_muloa_search(&ht, &C_CORNER_KEY_A) == NULL);
  res *= (ht_muloa_search(&ht, &C_CORNER_KEY_B) == NULL);
  print_test_result(res);
  free_ht(&ht);
}

/**
   Helper functions.
*/

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
  RGENS_SEED();
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
      args[1] > C_FULL_BIT - 1 ||
      args[2] > C_FULL_BIT - 1 ||
      args[1] > args[2] ||
      args[3] < 1 ||
      args[4] < 1 ||
      args[5] > C_FULL_BIT - 1 ||
      args[3] > args[4] ||
      args[3] > pow_two_perror(args[5]) ||
      args[4] > pow_two_perror(args[5]) ||
      args[6] < 1 ||
      args[7] < 1 ||
      args[8] > 1 ||
      args[9] > 1 ||
      args[10] > 1 ||
      args[11] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  };
  if (args[7]) run_insert_search_free_uint_test(args[0],
						args[1],
						args[2],
						args[3],
						args[4],
						args[5],
						args[6]);
  if (args[8]) run_remove_delete_uint_test(args[0],
					   args[1],
					   args[2],
					   args[3],
					   args[4],
					   args[5],
					   args[6]);
  if (args[9]) run_insert_search_free_uint_ptr_test(args[0],
						    args[1],
						    args[2],
						    args[3],
						    args[4],
						    args[5],
						    args[6]);
  if (args[10]) run_remove_delete_uint_ptr_test(args[0],
						args[1],
						args[2],
						args[3],
						args[4],
						args[5],
						args[6]);
  if (args[11]) run_corner_cases_test(args[0]);
  free(args);
  args = NULL;
  return 0;
}
