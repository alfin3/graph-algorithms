/**
   ht-divchn-test.c

   Tests of a hash table with generic hash keys and generic elements.
   The implementation is based on a division method for hashing and a
   chaining method for resolving collisions.

   The following command line arguments can be used to customize tests:
   ht-divchn-test
      [0, # bits in size_t - 1) : i s.t. # inserts = 2^i
      [0, # bits in size_t) : a given k = sizeof(size_t)
      [0, # bits in size_t) : b s.t. k * 2^a <= key size <= k * 2^b
      > 0 : c
      > 0 : d s.t. z = c / d
      [0, # bits in size_t) : e
      [0, # bits in size_t) : f s.t. z * 2^e <= alpha <= z * 2^f
      [0, 1] : on/off insert search uint test
      [0, 1] : on/off remove delete uint test
      [0, 1] : on/off insert search uint_ptr test
      [0, 1] : on/off remove delete uint_ptr test
      [0, 1] : on/off corner cases test

   usage examples:
   ./ht-divchn-test
   ./ht-divchn-test 17 0 4 1 100
   ./ht-divchn-test 17 0 4 2 10 0 8
   ./ht-divchn-test 17 5 6 2 10 0 1
   ./ht-divchn-test 17 5 6 2 10 0 1 0 0 1 1 0

   ht-divchn-test can be run with any subset of command line arguments in the
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
#include "ht-divchn.h"
#include "dll.h"
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
  "ht-divchn-test\n"
  "[0, # bits in size_t - 1) : i s.t. # inserts = 2^i\n"
  "[0, # bits in size_t) : a given k = sizeof(size_t)\n"
  "[0, # bits in size_t) : b s.t. k * 2^a <= key size <= k * 2^b\n"
  "> 0 : c\n"
  "> 0 : d s.t. z = c / d\n"
  "[0, # bits in size_t) : e\n"
  "[0, # bits in size_t) : f s.t. z * 2^e <= alpha <= z * 2^f\n"
  "[0, 1] : on/off insert search uint test\n"
  "[0, 1] : on/off remove delete uint test\n"
  "[0, 1] : on/off insert search uint_ptr test\n"
  "[0, 1] : on/off remove delete uint_ptr test\n"
  "[0, 1] : on/off corner cases test\n";
const int C_ARGC_MAX = 13;
const size_t C_ARGS_DEF[12] = {14, 0, 2, 3, 10, 0, 6, 1, 1, 1, 1, 1};
const size_t C_SIZE_MAX = (size_t)-1;
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);

/* insert, search, free, remove, delete tests */
const size_t C_KEY_SIZE_FACTOR = sizeof(size_t);

/* corner cases test */
const int C_CORNER_LOG_KEY_START = 0;
const int C_CORNER_LOG_KEY_END = 8;
const size_t C_CORNER_HT_COUNT = 1543u;
const float C_CORNER_ALPHA = 0.001;

void insert_search_free(size_t num_ins,
			size_t key_size,
			size_t elt_size,
			float alpha,
			void (*new_elt)(void *, size_t),
			size_t (*val_elt)(const void *),
			void (*free_elt)(void *));
void remove_delete(size_t num_ins,
		   size_t key_size,
		   size_t elt_size,
		   float alpha,
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
   ht_divchn_insert and the element is fully copied into the hash table.
   NULL as free_elt is sufficient to delete the element.
*/

void new_uint(void *elt, size_t val){
  size_t *s = elt;
  *s = val;
  s = NULL;
}

size_t val_uint(const void *elt){
  return *(size_t *)elt;
}

/**
   Runs a ht_divchn_{insert, search, free} test on distinct keys and 
   size_t elements across key sizes >= C_KEY_SIZE_FACTOR and load factor
   upper bounds.
*/
void run_insert_search_free_uint_test(int log_ins,
				      int log_key_start,
				      int log_key_end,
				      float alpha_factor,
				      int log_alpha_start,
				      int log_alpha_end){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(size_t);
  float alpha;
  num_ins = pow_two_perror(log_ins);
  for (i = log_key_start; i <= log_key_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two_perror(i);
    printf("Run a ht_divchn_{insert, search, free} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = log_alpha_start; j <= log_alpha_end; j++){
      alpha = alpha_factor * pow_two_perror(j);
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), alpha);
      insert_search_free(num_ins,
			 key_size,
			 elt_size,
			 alpha,
			 new_uint,
			 val_uint,
			 NULL);
    }
  }
}
/**
   Runs a ht_divchn_{remove, delete} test on distinct keys and size_t
   elements across key sizes >= C_KEY_SIZE_FACTOR and load factor upper
   bounds.
*/
void run_remove_delete_uint_test(int log_ins,
				 int log_key_start,
				 int log_key_end,
				 float alpha_factor,
				 int log_alpha_start,
				 int log_alpha_end){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(size_t);
  float alpha;
  num_ins = pow_two_perror(log_ins);
  for (i = log_key_start; i <= log_key_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two_perror(i);
    printf("Run a ht_divchn_{remove, delete} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = log_alpha_start; j <= log_alpha_end; j++){
      alpha = alpha_factor * pow_two_perror(j);
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), alpha);
      remove_delete(num_ins,
		    key_size,
		    elt_size,
		    alpha,
		    new_uint,
		    val_uint,
		    NULL);
    }
  }
}

/**
   Test hash table operations on distinct keys and noncontiguous
   uint_ptr_t elements across key sizes and load factor upper bounds. 
   For test purposes a key is random with the exception of a distinct
   non-random C_KEY_SIZE_FACTOR-sized block inside the key. A pointer to a
   pointer to an element is passed as elt in ht_divchn_insert, and the
   pointer to the element is copied into the hash table. An element-specific
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
  s = NULL;
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
   Runs a ht_divchn_{insert, search, free} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_insert_search_free_uint_ptr_test(int log_ins,
					  int log_key_start,
					  int log_key_end,
					  float alpha_factor,
					  int log_alpha_start,
					  int log_alpha_end){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size =  sizeof(uint_ptr_t *);
  float alpha;
  num_ins = pow_two_perror(log_ins);
  for (i = log_key_start; i <= log_key_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two_perror(i);
    printf("Run a ht_divchn_{insert, search, free} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    for (j = log_alpha_start; j <= log_alpha_end; j++){
      alpha = alpha_factor * pow_two_perror(j);
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), alpha);
      insert_search_free(num_ins,
			 key_size,
			 elt_size,
			 alpha,
			 new_uint_ptr,
			 val_uint_ptr,
			 free_uint_ptr);
    }
  }
}

/**
   Runs a ht_divchn_{remove, delete} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_remove_delete_uint_ptr_test(int log_ins,
				     int log_key_start,
				     int log_key_end,
				     float alpha_factor,
				     int log_alpha_start,
				     int log_alpha_end){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(uint_ptr_t *);
  float alpha;
  num_ins = pow_two_perror(log_ins);
  for (i = log_key_start; i <= log_key_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two_perror(i);
    printf("Run a ht_divchn_{remove, delete} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    for (j = log_alpha_start; j <= log_alpha_end; j++){
      alpha = alpha_factor * pow_two_perror(j);
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), alpha);
      remove_delete(num_ins,
		    key_size,
		    elt_size,
		    alpha,
		    new_uint_ptr,
		    val_uint_ptr,
		    free_uint_ptr);
    }
  }
}

/** 
   Helper functions for the ht_divchn_{insert, search, free} tests
   across key sizes and load factor upper bounds, on size_t and 
   uint_ptr_t elements.
*/

void insert_keys_elts(ht_divchn_t *ht,
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
    ht_divchn_insert(ht, p, p + ht->key_size);
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

void search_in_ht(const ht_divchn_t *ht,
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
    elt = ht_divchn_search(ht, p);
  }
  t = clock() - t;
  for (p = p_start; p != p_end; p += ht->pair_size){
    elt = ht_divchn_search(ht, p);
    *res *= (val_elt(p + ht->key_size) == val_elt(elt));
  }
  printf("\t\tin ht search time:              "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void search_nin_ht(const ht_divchn_t *ht,
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
    elt = ht_divchn_search(ht, p);
  }
  t = clock() - t;
  for (p = p_start; p != p_end; p += ht->key_size){
    elt = ht_divchn_search(ht, p);
    *res *= (elt == NULL);
  }
  printf("\t\tnot in ht search time:          "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void free_ht(ht_divchn_t *ht){
  clock_t t;
  t = clock();
  ht_divchn_free(ht);
  t = clock() - t;
  printf("\t\tfree time:                      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

void insert_search_free(size_t num_ins,
			size_t key_size,
			size_t elt_size,
			float alpha,
			void (*new_elt)(void *, size_t),
			size_t (*val_elt)(const void *),
			void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  size_t pair_size = add_sz_perror(key_size, elt_size);
  void *key = NULL;
  void *key_elts = NULL;
  void *nin_keys = NULL;
  ht_divchn_t ht;
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
  ht_divchn_init(&ht, key_size, elt_size, 0, alpha, NULL);
  insert_keys_elts(&ht, key_elts, num_ins, &res);
  free_ht(&ht);
  ht_divchn_init(&ht, key_size, elt_size, num_ins, alpha, free_elt);
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
   Helper functions for the ht_divchn_{remove, delete} tests
   across key sizes and load factor upper bounds, on size_t and 
   uint_ptr_t elements.
*/

void remove_key_elts(ht_divchn_t *ht,
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
    ht_divchn_remove(ht, p, elt);
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
      *res *= (val_elt(p + ht->key_size) == val_elt(ht_divchn_search(ht, p)));
    }else{
      *res *= (ht_divchn_search(ht, p) == NULL);
    }
  }
  p = ptr(key_elts, (count > 0), ht->pair_size);
  t_second_half = clock();
  for (i = 1; i < count; i += 2){ /* count < SIZE_MAX */
    p += (i > 1) * step_size; /* avoid undef. behavior of pointer increment */
    ht_divchn_remove(ht, p, elt);
    /* noncontiguous element is still accessible from key_elts */
  }
  t_second_half = clock() - t_second_half;
  *res *= (ht->num_elts == 0);
  p_start = key_elts;
  p_end = ptr(key_elts, count, ht->pair_size);
  for (p = p_start; p != p_end; p += ht->pair_size){
    *res *= (ht_divchn_search(ht, p) == NULL);
  }
  for (i = 0; i < ht->count; i++){
    *res *= (ht->key_elts[i] == NULL);
  }
  printf("\t\tremove 1/2 elements time:       "
	 "%.4f seconds\n", (float)t_first_half / CLOCKS_PER_SEC);
  printf("\t\tremove residual elements time:  "
	 "%.4f seconds\n", (float)t_second_half / CLOCKS_PER_SEC);
  free(elt);
  elt = NULL;
}

void delete_key_elts(ht_divchn_t *ht,
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
    ht_divchn_delete(ht, p);
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
      *res *= (val_elt(p + ht->key_size) == val_elt(ht_divchn_search(ht, p)));
    }else{
      *res *= (ht_divchn_search(ht, p) == NULL);
    }
  }
  p = ptr(key_elts, (count > 0), ht->pair_size);
  t_second_half = clock();
  for (i = 1; i < count; i += 2){ /* count < SIZE_MAX */
    p += (i > 1) * step_size; /* avoid undef. behavior of pointer increment */
    ht_divchn_delete(ht, p);
  }
  t_second_half = clock() - t_second_half;
  *res *= (ht->num_elts == 0);
  p_start = key_elts;
  p_end = ptr(key_elts, count, ht->pair_size);
  for (p = p_start; p != p_end; p += ht->pair_size){
    *res *= (ht_divchn_search(ht, p) == NULL);
  }
  for (i = 0; i < ht->count; i++){
    *res *= (ht->key_elts[i] == NULL);
  }
  printf("\t\tdelete 1/2 elements time:       "
	 "%.4f seconds\n", (float)t_first_half / CLOCKS_PER_SEC);
  printf("\t\tdelete residual elements time:  "
	 "%.4f seconds\n", (float)t_second_half / CLOCKS_PER_SEC);
}

void remove_delete(size_t num_ins,
		   size_t key_size,
		   size_t elt_size,
		   float alpha,
		   void (*new_elt)(void *, size_t),
		   size_t (*val_elt)(const void *),
		   void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  size_t pair_size = add_sz_perror(key_size, elt_size);
  void *key = NULL;
  void *key_elts = NULL;
  ht_divchn_t ht;
  key_elts = malloc_perror(num_ins, pair_size);
  for (i = 0; i < num_ins; i++){
    key = ptr(key_elts, i, pair_size);
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      *(unsigned char *)ptr(key, j, 1) = RANDOM(); /* mod 2^CHAR_BIT */
    }
    *(size_t *)ptr(key, key_size - C_KEY_SIZE_FACTOR, 1) = i;
    new_elt((char *)ptr(key_elts, i, pair_size) + key_size, i);
  }
  ht_divchn_init(&ht, key_size, elt_size, 0, alpha, free_elt);
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
  int j;
  size_t i, k;
  size_t elt;
  size_t elt_size = sizeof(size_t);
  size_t key_size;
  size_t num_ins;
  void *key = NULL;
  ht_divchn_t ht;
  num_ins = pow_two_perror(log_ins);
  key = malloc_perror(1, pow_two_perror(C_CORNER_LOG_KEY_END));
  for (i = 0; i < pow_two_perror(C_CORNER_LOG_KEY_END); i++){
    *(unsigned char *)ptr(key, i, 1) = RANDOM();
  }
  printf("Run corner cases test --> ");
  for (j = C_CORNER_LOG_KEY_START; j <= C_CORNER_LOG_KEY_END; j++){
    key_size = pow_two_perror(j);
    ht_divchn_init(&ht, key_size, elt_size, 0, C_CORNER_ALPHA, NULL);
    for (k = 0; k < num_ins; k++){
      elt = k;
      ht_divchn_insert(&ht, key, &elt);
    }
    res *= (ht.count_ix == 0);
    res *= (ht.count == C_CORNER_HT_COUNT);
    res *= (ht.num_elts == 1);
    res *= (*(const size_t *)ht_divchn_search(&ht, key) == elt);
    ht_divchn_delete(&ht, key);
    res *= (ht.count == C_CORNER_HT_COUNT);
    res *= (ht.num_elts == 0);
    res *= (ht_divchn_search(&ht, key) == NULL);
    ht_divchn_free(&ht);
  }
  print_test_result(res);
  free(key);
  key = NULL;
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
  float alpha_factor;
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
      args[6] > C_FULL_BIT - 1 ||
      args[5] > args[6] ||
      args[7] > 1 ||
      args[8] > 1 ||
      args[9] > 1 ||
      args[10] > 1 ||
      args[11] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  alpha_factor = (float)args[3] / args[4];
  if (args[7]) run_insert_search_free_uint_test(args[0],
						args[1],
						args[2],
						alpha_factor,
						args[5],
						args[6]);
  if (args[8]) run_remove_delete_uint_test(args[0],
					   args[1],
					   args[2],
					   alpha_factor,
					   args[5],
					   args[6]);
  if (args[9]) run_insert_search_free_uint_ptr_test(args[0],
						    args[1],
						    args[2],
						    alpha_factor,
						    args[5],
						    args[6]);
  if (args[10]) run_remove_delete_uint_ptr_test(args[0],
						args[1],
						args[2],
						alpha_factor,
						args[5],
						args[6]);
  if (args[11]) run_corner_cases_test(args[0]);
  free(args);
  args = NULL;
  return 0;
}
