/**
   ht-divchn-test.c

   Tests of a hash table with generic contiguous keys and generic
   contiguous and non-contiguous elements. The implementation is based on a
   division method for hashing and a chaining method for resolving
   collisions.

   The following command line arguments can be used to customize tests:
   ht-divchn-test
      [0, size_t width - 1) : i s.t. # inserts = 2**i
      [0, size_t width) : a given k = sizeof(size_t)
      [0, size_t width) : b s.t. k * 2**a <= key size <= k * 2**b
      > 0 : c
      > 0 : d
      > 0 : e log base 2
      > 0 : f s.t. c / 2**e <= load factor bound <= d / 2**e, in f steps
      [0, 1] : on/off insert search uint test
      [0, 1] : on/off remove delete uint test
      [0, 1] : on/off insert search uint_ptr test
      [0, 1] : on/off remove delete uint_ptr test
      [0, 1] : on/off corner cases test

   usage examples:
   ./ht-divchn-test
   ./ht-divchn-test 20
   ./ht-divchn-test 17 5 6
   ./ht-divchn-test 19 0 2 3000 4000 11 10

   ht-divchn-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99 with the only requirement that the width of size_t is
   greater or equal to 16, less than 2040, and is even.
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
#include "utilities-lim.h"

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
  "[0, size_t width - 1) : i s.t. # inserts = 2**i\n"
  "[0, size_t width) : a given k = sizeof(size_t)\n"
  "[0, size_t width) : b s.t. k * 2**a <= key size <= k * 2**b\n"
  "> 0 : c\n"
  "> 0 : d\n"
  "> 0 : e log base 2\n"
  "> 0 : f s.t. c / 2**e <= load factor bound <= d / 2**e, in f steps\n"
  "[0, 1] : on/off insert search uint test\n"
  "[0, 1] : on/off remove delete uint test\n"
  "[0, 1] : on/off insert search uint_ptr test\n"
  "[0, 1] : on/off remove delete uint_ptr test\n"
  "[0, 1] : on/off corner cases test\n";
const int C_ARGC_ULIMIT = 13;
const size_t C_ARGS_DEF[12] = {14u, 0u, 2u, 1024u, 30720u, 11u, 10u,
                               1u, 1u, 1u, 1u, 1u};
const size_t C_SIZE_ULIMIT = (size_t)-1;
const size_t C_FULL_BIT = PRECISION_FROM_ULIMIT((size_t)-1);

/* corner cases test */
const size_t C_CORNER_LOG_KEY_START = 0u;
const size_t C_CORNER_LOG_KEY_END = 8u;
const size_t C_CORNER_HT_COUNT = 1543u;
const size_t C_CORNER_ALPHA_N = 33u;
const size_t C_CORNER_LOG_ALPHA_D = 15u; /* lf bound is 33/32768 */

void insert_search_free(size_t num_ins,
                        size_t key_size,
                        size_t elt_size,
                        size_t elt_alignment,
                        size_t alpha_n,
                        size_t log_alpha_d,
                        void (*new_elt)(void *, size_t),
                        size_t (*val_elt)(const void *),
                        void (*free_elt)(void *));
void remove_delete(size_t num_ins,
                   size_t key_size,
                   size_t elt_size,
                   size_t elt_alignment,
                   size_t alpha,
                   size_t log_alpha_d,
                   void (*new_elt)(void *, size_t),
                   size_t (*val_elt)(const void *),
                   void (*free_elt)(void *));
void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);

/**
   Test hash table operations on distinct contiguous keys and contiguous
   size_t elements across key sizes and load factor upper bounds. For test
   purposes a key is a random key_size block with the exception of a
   distinct non-random sizeof(size_t)-sized sub-block inside the key_size
   block. An element is an elt_size block of size sizeof(size_t) with a
   size_t value. Keys and elements are entirely copied into a hash table and
   free_key and free_elt are NULL.
*/

void new_uint(void *elt, size_t val){
  size_t *s = elt;
  *s = val;
}

size_t val_uint(const void *elt){
  return *(size_t *)elt;
}

/**
   Runs a ht_divchn_{insert, search, free} test on distinct keys and
   size_t elements across key sizes >= sizeof(size_t) and load factor
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
  size_t elt_alignment = sizeof(size_t);
  size_t step, rem;
  size_t alpha_n;
  num_ins = pow_two_perror(log_ins);
  step = (alpha_n_end - alpha_n_start) / num_alpha_steps;
  for (i = log_key_start; i <= log_key_end; i++){
    alpha_n = alpha_n_start;
    rem = alpha_n_end - alpha_n_start - step * num_alpha_steps;
    key_size = sizeof(size_t) * pow_two_perror(i);
    printf("Run a ht_divchn_{insert, search, free} test on distinct "
           "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
             TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      insert_search_free(num_ins,
                         key_size,
                         elt_size,
                         elt_alignment,
                         alpha_n,
                         log_alpha_d,
                         new_uint,
                         val_uint,
                         NULL);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}
/**
   Runs a ht_divchn_{remove, delete} test on distinct keys and size_t
   elements across key sizes >= sizeof(size_t) and load factor upper
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
  size_t elt_alignment = sizeof(size_t);
  size_t step, rem;
  size_t alpha_n;
  num_ins = pow_two_perror(log_ins);
  step = (alpha_n_end - alpha_n_start) / num_alpha_steps;
  for (i = log_key_start; i <= log_key_end; i++){
    alpha_n = alpha_n_start;
    rem = alpha_n_end - alpha_n_start - step * num_alpha_steps;
    key_size = sizeof(size_t) * pow_two_perror(i);
    printf("Run a ht_divchn_{remove, delete} test on distinct "
           "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
             TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      remove_delete(num_ins,
                    key_size,
                    elt_size,
                    elt_alignment,
                    alpha_n,
                    log_alpha_d,
                    new_uint,
                    val_uint,
                    NULL);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Test hash table operations on distinct contiguous keys and noncontiguous
   uint_ptr elements across key sizes and load factor upper bounds. For
   test purposes a key is a random key_size block with the exception of a
   distinct non-random sizeof(size_t)-sized sub-block inside the key_size
   block. A key is fully copied into a hash table as a key_size block.
   Because an element is noncontiguous, a pointer to an element is copied as
   an elt_size block. free_key is NULL. An element-specific free_elt is
   necessary to delete an element.
*/

struct uint_ptr{
  size_t *val;
};

void new_uint_ptr(void *elt, size_t val){
  struct uint_ptr **s = elt;
  *s = malloc_perror(1, sizeof(struct uint_ptr));
  (*s)->val = malloc_perror(1, sizeof(size_t));
  *((*s)->val) = val;
}

size_t val_uint_ptr(const void *elt){
  struct uint_ptr **s  = (struct uint_ptr **)elt;
  return *((*s)->val);
}

void free_uint_ptr(void *elt){
  struct uint_ptr **s = elt;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
}

/**
   Runs a ht_divchn_{insert, search, free} test on distinct keys and
   noncontiguous uint_ptr elements across key sizes >= sizeof(size_t)
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
  size_t elt_size =  sizeof(struct uint_ptr *);
  size_t elt_alignment = sizeof(struct uint_ptr *);
  size_t step, rem;
  size_t alpha_n;
  num_ins = pow_two_perror(log_ins);
  step = (alpha_n_end - alpha_n_start) / num_alpha_steps;
  for (i = log_key_start; i <= log_key_end; i++){
    alpha_n = alpha_n_start;
    rem = alpha_n_end - alpha_n_start - step * num_alpha_steps;
    key_size = sizeof(size_t) * pow_two_perror(i);
    printf("Run a ht_divchn_{insert, search, free} test on distinct "
           "%lu-byte keys and noncontiguous uint_ptr elements\n",
           TOLU(key_size));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
             TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      insert_search_free(num_ins,
                         key_size,
                         elt_size,
                         elt_alignment,
                         alpha_n,
                         log_alpha_d,
                         new_uint_ptr,
                         val_uint_ptr,
                         free_uint_ptr);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Runs a ht_divchn_{remove, delete} test on distinct keys and
   noncontiguous uint_ptr elements across key sizes >= sizeof(size_t)
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
  size_t elt_size = sizeof(struct uint_ptr *);
  size_t elt_alignment = sizeof(struct uint_ptr *);
  size_t step, rem;
  size_t alpha_n;
  num_ins = pow_two_perror(log_ins);
  step = (alpha_n_end - alpha_n_start) / num_alpha_steps;
  for (i = log_key_start; i <= log_key_end; i++){
    alpha_n = alpha_n_start;
    rem = alpha_n_end - alpha_n_start - step * num_alpha_steps;
    key_size = sizeof(size_t) * pow_two_perror(i);
    printf("Run a ht_divchn_{remove, delete} test on distinct "
           "%lu-byte keys and noncontiguous uint_ptr elements\n",
           TOLU(key_size));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
             TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      remove_delete(num_ins,
                    key_size,
                    elt_size,
                    elt_alignment,
                    alpha_n,
                    log_alpha_d,
                    new_uint_ptr,
                    val_uint_ptr,
                    free_uint_ptr);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Helper functions for the ht_divchn_{insert, search, free} tests
   across key sizes and load factor upper bounds, on size_t and
   uint_ptr elements.
*/

void insert_keys_elts(struct ht_divchn *ht,
                      const unsigned char *keys,
                      const void *elts,
                      size_t count,
                      int *res){
  size_t i;
  size_t n = ht->num_elts;
  size_t init_count = ht->count;
  const unsigned char *k = NULL;
  const void *e = NULL;
  clock_t t;
  k = keys;
  e = elts;
  t = clock();
  for (i = 0; i < count; i++){
    ht_divchn_insert(ht, k, e);
    k += ht->key_size;
    e = (char *)e + ht->elt_size;
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

void search_in_ht(const struct ht_divchn *ht,
                  const unsigned char *keys,
                  const void *elts,
                  size_t count,
                  size_t (*val_elt)(const void *),
                  int *res){
  size_t i;
  size_t n = ht->num_elts;
  const unsigned char *k = NULL;
  const void *e = NULL;
  const void *elt = NULL;
  clock_t t;
  k = keys;
  t = clock();
  for (i = 0; i < count; i++){
    elt = ht_divchn_search(ht, k);
    k += ht->key_size;
  }
  k = keys;
  e = elts;
  t = clock() - t;
  for (i = 0; i < count; i++){
    elt = ht_divchn_search(ht, k);
    *res *= (val_elt(e) == val_elt(elt));
    k += ht->key_size;
    e = (char *)e + ht->elt_size;
  }
  printf("\t\tin ht search time:              "
         "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void search_nin_ht(const struct ht_divchn *ht,
                   const unsigned char *nin_keys,
                   size_t count,
                   int *res){
  size_t i;
  size_t n = ht->num_elts;
  const unsigned char *k = NULL;
  const void *elt = NULL;
  clock_t t;
  k = nin_keys;
  t = clock();
  for (i = 0; i < count; i++){
    elt = ht_divchn_search(ht, k);
    k += ht->key_size;
  }
  k = nin_keys;
  t = clock() - t;
  for (i = 0; i < count; i++){
    elt = ht_divchn_search(ht, k);
    *res *= (elt == NULL);
    k += ht->key_size;
  }
  printf("\t\tnot in ht search time:          "
         "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void free_ht(struct ht_divchn *ht){
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
                        size_t elt_alignment,
                        size_t alpha_n,
                        size_t log_alpha_d,
                        void (*new_elt)(void *, size_t),
                        size_t (*val_elt)(const void *),
                        void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  size_t val;
  unsigned char key_buf[sizeof(size_t)];
  unsigned char *key = NULL;
  unsigned char *keys = NULL;
  unsigned char *nin_keys = NULL;
  void *elts = NULL;
  struct ht_divchn ht;
  keys = malloc_perror(num_ins, key_size);
  elts = malloc_perror(num_ins, elt_size);
  nin_keys = malloc_perror(num_ins, key_size);
  for (i = 0; i < num_ins; i++){
    key = ptr(keys, i, key_size);
    for (j = 0; j < key_size - sizeof(size_t); j++){
      *(unsigned char *)ptr(key, j, 1) = RANDOM(); /* mod 2**CHAR_BIT */
    }
    memcpy(key_buf, &i, sizeof(size_t)); /* eff. type in key unchanged */
    memcpy(ptr(key, key_size - sizeof(size_t), 1), key_buf, sizeof(size_t));
    new_elt(ptr(elts, i, elt_size), i);
  }
  ht_divchn_init(&ht,
                 key_size,
                 elt_size,
                 0,
                 alpha_n,
                 log_alpha_d,
                 NULL,
                 NULL,
                 NULL,
                 NULL);
  insert_keys_elts(&ht, keys, elts, num_ins, &res); /* no dereferencing */
  free_ht(&ht);
  ht_divchn_init(&ht,
                 key_size,
                 elt_size,
                 num_ins,
                 alpha_n,
                 log_alpha_d,
                 NULL,
                 NULL,
                 NULL,
                 free_elt);
  ht_divchn_align(&ht, elt_alignment);
  insert_keys_elts(&ht, keys, elts, num_ins, &res);
  search_in_ht(&ht, keys, elts, num_ins, val_elt, &res);
  for (i = 0; i < num_ins; i++){
    key = ptr(nin_keys, i, key_size);
    val = i + num_ins;
    for (j = 0; j < key_size - sizeof(size_t); j++){
      *(unsigned char *)ptr(key, j, 1) = RANDOM(); /* mod 2**CHAR_BIT */
    }
    memcpy(key_buf, &val, sizeof(size_t)); /* eff. type in key unchanged */
    memcpy(ptr(key, key_size - sizeof(size_t), 1), key_buf, sizeof(size_t));
  }
  search_nin_ht(&ht, nin_keys, num_ins, &res);
  free_ht(&ht);
  printf("\t\tsearch correctness:             ");
  print_test_result(res);
  free(keys);
  free(elts);
  free(nin_keys);
  keys = NULL;
  elts = NULL;
  nin_keys = NULL;
}

/**
   Helper functions for the ht_divchn_{remove, delete} tests
   across key sizes and load factor upper bounds, on size_t and
   uint_ptr elements.
*/

void remove_key_elts(struct ht_divchn *ht,
                     const unsigned char *keys,
                     const void *elts,
                     size_t count,
                     size_t (*val_elt)(const void *),
                     int *res){
  size_t i;
  size_t n = ht->num_elts;
  size_t key_step_size = mul_sz_perror(2, ht->key_size);
  const unsigned char *k = NULL;
  const void *e = NULL;
  void *elt = NULL;
  clock_t t_first_half, t_second_half;
  elt = malloc_perror(1, ht->elt_size);
  k = keys;
  t_first_half = clock();
  for (i = 0; i < count; i += 2){ /* count < SIZE_MAX */
    k += (i > 0) * key_step_size; /* avoid UB in pointer increment */
    ht_divchn_remove(ht, k, elt);
    /* noncontiguous element is still accessible from elts */
  }
  t_first_half = clock() - t_first_half;
  *res *= (ht->num_elts == ((count & 1) ?
                            (n - count / 2 - 1) :
                            (n - count / 2)));
  k = keys;
  e = elts;
  for (i = 0; i < count; i++){
    if (i & 1){
      *res *= (val_elt(e) == val_elt(ht_divchn_search(ht, k)));
    }else{
      *res *= (ht_divchn_search(ht, k) == NULL);
    }
    k += ht->key_size;
    e = (char *)e + ht->elt_size;
  }
  k = ptr(keys, 1, ht->key_size); /* 1 <= count */
  t_second_half = clock();
  for (i = 1; i < count; i += 2){ /* count < SIZE_MAX */
    k += (i > 1) * key_step_size; /* avoid UB in pointer increment */
    ht_divchn_remove(ht, k, elt);
    /* noncontiguous element is still accessible from elts */
  }
  t_second_half = clock() - t_second_half;
  *res *= (ht->num_elts == 0);
  k = keys;
  for (i = 0; i < count; i++){
    *res *= (ht_divchn_search(ht, k) == NULL);
    k += ht->key_size;
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

void delete_key_elts(struct ht_divchn *ht,
                     const unsigned char *keys,
                     const void *elts,
                     size_t count,
                     size_t (*val_elt)(const void *),
                     int *res){
  size_t i;
  size_t n = ht->num_elts;
  size_t key_step_size = mul_sz_perror(2, ht->key_size);
  const unsigned char *k = NULL;
  const void *e = NULL;
  clock_t t_first_half, t_second_half;
  k = keys;
  t_first_half = clock();
  for (i = 0; i < count; i += 2){ /* count < SIZE_MAX */
    k += (i > 0) * key_step_size; /* avoid UB in pointer increment */
    ht_divchn_delete(ht, k);
  }
  t_first_half = clock() - t_first_half;
  *res *= (ht->num_elts == ((count & 1) ?
                            (n - count / 2 - 1) :
                            (n - count / 2)));
  k = keys;
  e = elts;
  for (i = 0; i < count; i++){
    if (i & 1){
      *res *= (val_elt(e) == val_elt(ht_divchn_search(ht, k)));
    }else{
      *res *= (ht_divchn_search(ht, k) == NULL);
    }
    k += ht->key_size;
    e = (char *)e + ht->elt_size;
  }
  k = ptr(keys, 1, ht->key_size); /* 1 <= count */
  t_second_half = clock();
  for (i = 1; i < count; i += 2){ /* count < SIZE_MAX */
    k += (i > 1) * key_step_size; /* avoid UB in pointer increment */
    ht_divchn_delete(ht, k);
  }
  t_second_half = clock() - t_second_half;
  *res *= (ht->num_elts == 0);
  k = keys;
  for (i = 0; i < count; i++){
    *res *= (ht_divchn_search(ht, k) == NULL);
    k += ht->key_size;
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
                   size_t elt_alignment,
                   size_t alpha_n,
                   size_t log_alpha_d,
                   void (*new_elt)(void *, size_t),
                   size_t (*val_elt)(const void *),
                   void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  unsigned char key_buf[sizeof(size_t)];
  unsigned char *key = NULL;
  unsigned char *keys = NULL;
  void *elts = NULL;
  struct ht_divchn ht;
  keys = malloc_perror(num_ins, key_size);
  elts = malloc_perror(num_ins, elt_size);
  for (i = 0; i < num_ins; i++){
    key = ptr(keys, i, key_size);
    for (j = 0; j < key_size - sizeof(size_t); j++){
      *(unsigned char *)ptr(key, j, 1) = RANDOM(); /* mod 2**CHAR_BIT */
    }
    memcpy(key_buf, &i, sizeof(size_t)); /* eff. type in key unchanged */
    memcpy(ptr(key, key_size - sizeof(size_t), 1), key_buf, sizeof(size_t));
    new_elt(ptr(elts, i, elt_size), i);
  }
  ht_divchn_init(&ht,
                 key_size,
                 elt_size,
                 0,
                 alpha_n,
                 log_alpha_d,
                 NULL,
                 NULL,
                 NULL,
                 free_elt);
  ht_divchn_align(&ht, elt_alignment);
  insert_keys_elts(&ht, keys, elts, num_ins, &res);
  remove_key_elts(&ht, keys, elts, num_ins, val_elt, &res);
  insert_keys_elts(&ht, keys, elts, num_ins, &res);
  delete_key_elts(&ht, keys, elts, num_ins, val_elt, &res);
  free_ht(&ht);
  printf("\t\tremove and delete correctness:  ");
  print_test_result(res);
  free(keys);
  free(elts);
  keys = NULL;
  elts = NULL;
}

/**
   Runs a corner cases test.
*/
void run_corner_cases_test(size_t log_ins){
  int res = 1;
  size_t j;
  size_t i, k;
  size_t elt;
  size_t elt_size = sizeof(size_t);
  size_t elt_alignment = sizeof(size_t);
  size_t key_size;
  size_t num_ins;
  void *key = NULL;
  struct ht_divchn ht;
  num_ins = pow_two_perror(log_ins);
  key = malloc_perror(1, pow_two_perror(C_CORNER_LOG_KEY_END));
  for (i = 0; i < pow_two_perror(C_CORNER_LOG_KEY_END); i++){
    *(unsigned char *)ptr(key, i, 1) = RANDOM();
  }
  printf("Run corner cases test --> ");
  for (j = C_CORNER_LOG_KEY_START; j <= C_CORNER_LOG_KEY_END; j++){
    key_size = pow_two_perror(j);
    ht_divchn_init(&ht,
                   key_size,
                   elt_size,
                   0,
                   C_CORNER_ALPHA_N,
                   C_CORNER_LOG_ALPHA_D,
                   NULL,
                   NULL,
                   NULL,
                   NULL);
    ht_divchn_align(&ht, elt_alignment);
    for (k = 0; k < num_ins; k++){
      elt = k;
      ht_divchn_insert(&ht, key, &elt);
    }
    res *= (ht.count_ix == 0 &&
            ht.count == C_CORNER_HT_COUNT &&
            ht.num_elts == 1 &&
            *(const size_t *)ht_divchn_search(&ht, key) == elt);
    ht_divchn_delete(&ht, key);
    res *= (ht.count == C_CORNER_HT_COUNT &&
            ht.num_elts == 0 &&
            ht_divchn_search(&ht, key) == NULL);
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
  RGENS_SEED();
  if (argc > C_ARGC_ULIMIT){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_ULIMIT - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_ULIMIT - 1) * sizeof(size_t));
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
