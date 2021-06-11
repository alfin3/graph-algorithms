/**
   ht-divchn-pthread-test.c

   Tests of a hash table with generic hash keys and generic elements.
   The implementation is based on a division method for hashing and a
   chaining method for resolving collisions.

   The following command line arguments can be used to customize tests:
   ht-divchn-pthread-test
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
   ./ht-divchn-pthread-test
   ./ht-divchn-pthread-test 17 0 4 1 100
   ./ht-divchn-pthread-test 17 0 4 2 10 0 8
   ./ht-divchn-pthread-test 17 5 6 2 10 0 1
   ./ht-divchn-pthread-test 17 5 6 2 10 0 1 0 0 1 1 0

   ht-divchn-pthread-test can be run with any subset of command line
   arguments in the above-defined order. If the (i + 1)th argument is
   specified then the ith argument must be specified for i >= 0. Default
   values are used for the unspecified arguments according to the
   C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99 with the only requirement that CHAR_BIT * sizeof(size_t)
   is greater or equal to 16 and is even.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include "ht-divchn-pthread.h"
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-pthread.h"

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
  "ht-div-pthread-test\n"
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
const int C_CORNER_KEY_POW_START = 0;
const int C_CORNER_KEY_POW_END = 8;
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
void *byte_ptr(const void *block, size_t i);
void print_test_result(int res);
double timer();

void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

/**
   Test hash table operations on distinct keys and size_t elements 
   across key sizes and load factor upper bounds. For test purposes a key
   is random with the exception of a distinct non-random C_KEY_SIZE_FACTOR-
   sized block inside the key. A pointer to an element is passed as elt in
   ht_divchn_pthread_insert and the element is fully copied into the hash table.
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
   Runs a ht_divchn_pthread_{insert, search, free} test on distinct keys and 
   size_t elements across key sizes >= C_KEY_SIZE_FACTOR and load factor
   upper bounds.
*/
void run_insert_search_free_uint_test(int ins_pow,
				      int key_pow_start,
				      int key_pow_end,
				      float alpha_factor,
				      int alpha_pow_start,
				      int alpha_pow_end){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(size_t);
  float alpha;
  num_ins = pow_two(ins_pow);
  for (i = key_pow_start; i <= key_pow_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two(i);
    printf("Run a ht_divchn_pthread_{insert, search, free} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = alpha_pow_start; j <= alpha_pow_end; j++){
      alpha = alpha_factor * pow_two(j);
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
   Test hash table operations on distinct keys and noncontiguous
   uint_ptr_t elements across key sizes and load factor upper bounds. 
   For test purposes a key is random with the exception of a distinct
   non-random C_KEY_SIZE_FACTOR-sized block inside the key. A pointer to a
   pointer to an element is passed as elt in ht_divchn_pthread_insert, and
   the pointer to the element is copied into the hash table. An element-
   specific free_elt is necessary to delete the element.
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
  s = NULL;
}

/**
   Runs a ht_divchn_pthread_{insert, search, free} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_insert_search_free_uint_ptr_test(int ins_pow,
					  int key_pow_start,
					  int key_pow_end,
					  float alpha_factor,
					  int alpha_pow_start,
					  int alpha_pow_end){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size =  sizeof(uint_ptr_t *);
  float alpha;
  num_ins = pow_two(ins_pow);
  for (i = key_pow_start; i <= key_pow_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two(i);
    printf("Run a ht_divchn_pthread_{insert, search, free} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    for (j = alpha_pow_start; j <= alpha_pow_end; j++){
      alpha = alpha_factor * pow_two(j);
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
   Helper functions for the ht_divchn_pthread_{insert, search, free} tests
   across key sizes and load factor upper bounds, on size_t and uint_ptr_t
   elements.
*/

typedef struct{
  size_t start;
  size_t count;
  const void *keys;
  const void *elts;
  ht_divchn_pthread_t *ht;
} insert_arg_t;

void *insert_thread(void *arg){
  size_t i;
  size_t batch_count = 1000;
  insert_arg_t *ia = arg;
  /*printf("thread entered, "
	 "ht->count : %lu, "
	 "ht->num_elts : %lu, "
	 "start : %lu, "
	 "count : %lu\n",
	 TOLU(ia->ht->count),
	 TOLU(ia->ht->num_elts),
	 TOLU(ia->start),
	 TOLU(ia->count));*/
  for (i = 0; i < ia->count; i += batch_count){
    if (ia->count - i < batch_count){
      ht_divchn_pthread_insert(ia->ht,
			       ptr(ia->keys,
				   ia->start + i,
				   ia->ht->key_size),
			       ptr(ia->elts,
				   ia->start + i,
				   ia->ht->elt_size),
			       ia->count - i);
    }else{
      ht_divchn_pthread_insert(ia->ht,
			       ptr(ia->keys,
				   ia->start + i,
				   ia->ht->key_size),
			       ptr(ia->elts,
				   ia->start + i,
				   ia->ht->elt_size),
			       batch_count);
    }
  }
  /*printf("thread finished, "
	 "ht->count : %lu, "
	 "ht->num_elts : %lu\n",
	 TOLU(ia->ht->count),
	 TOLU(ia->ht->num_elts));*/
  return NULL;
}

void insert_keys_elts(ht_divchn_pthread_t *ht,
		      const void *keys,
		      const void *elts,
		      size_t count,
		      size_t num_threads,
		      int *res){
  size_t n = ht->num_elts;
  size_t i;
  size_t seg_count, rem_count;
  size_t start = 0;
  double t;
  pthread_t *iids = NULL;
  insert_arg_t *ias = NULL;
  iids = malloc_perror(num_threads, sizeof(pthread_t));
  ias = malloc_perror(num_threads, sizeof(insert_arg_t));
  seg_count = count / num_threads;
  rem_count = count % num_threads; /* to distribute among threads */
  for (i = 0; i < num_threads; i++){
    ias[i].start = start;
    ias[i].count = seg_count;
    if (rem_count > 0){
      ias[i].count++;
      rem_count--;
    }
    ias[i].keys = keys;
    ias[i].elts = elts;
    ias[i].ht = ht;
    start += ias[i].count;
  }
  t = timer();
  for (i = 1; i < num_threads; i++){
    thread_create_perror(&iids[i], insert_thread, &ias[i]);
  }
  /* use the parent thread as well */
  insert_thread(&ias[0]);
  for (i = 1; i < num_threads; i++){
    thread_join_perror(iids[i], NULL);
  }
  t = timer() - t;
  printf("\t\t%lu threads, insert time:         "
	 "%.4f seconds\n", TOLU(num_threads), t);
  *res *= (ht->num_elts == n + count);
  free(iids);
  free(ias);
  iids = NULL;
  ias = NULL;
}

void search_in_ht(const ht_divchn_pthread_t *ht,
		  const void *keys,
		  const void *elts,
		  size_t count,
		  size_t (*val_elt)(const void *),
		  int *res){
  size_t n = ht->num_elts;
  size_t i;
  double t;
  void *elt = NULL;
  t = timer();
  for (i = 0; i < count; i++){
    elt = ht_divchn_pthread_search(ht, ptr(keys, i, ht->key_size));
  }
  t = timer() - t;
  for (i = 0; i < count; i++){
    elt = ht_divchn_pthread_search(ht, ptr(keys, i, ht->key_size));
    *res *= (val_elt(ptr(elts, i, ht->elt_size)) == val_elt(elt));
  }
  printf("\t\tin ht search time:              "
	 "%.4f seconds\n", t);
  *res *= (ht->num_elts == n);
}

void search_not_in_ht(const ht_divchn_pthread_t *ht,
		      void **keys,
		      size_t count,
		      int *res){
  size_t n = ht->num_elts;
  size_t i;
  double t;
  void *elt = NULL;
  t = timer();
  for (i = 0; i < count; i++){
    elt = ht_divchn_pthread_search(ht, ptr(keys, i, ht->key_size));
  }
  t = timer() - t;
  for (i = 0; i < count; i++){
    elt = ht_divchn_pthread_search(ht, ptr(keys, i, ht->key_size));
    *res *= (elt == NULL);
  }
  printf("\t\tnot in ht search time:          "
	 "%.4f seconds\n", t);
  *res *= (ht->num_elts == n);
}

void free_ht(ht_divchn_pthread_t *ht){
  clock_t t;
  t = clock();
  ht_divchn_pthread_free(ht);
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
  void *keys = NULL, *elts = NULL;
  ht_divchn_pthread_t ht;
  keys = malloc_perror(num_ins, key_size);
  elts = malloc_perror(num_ins, elt_size);
  for (i = 0; i < num_ins; i++){
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      *(unsigned char *)byte_ptr(ptr(keys, i, key_size),
				 j) = RANDOM(); /* mod 2^CHAR_BIT */
    }
    *(size_t *)byte_ptr(ptr(keys, i, key_size),
			key_size - C_KEY_SIZE_FACTOR) = i;
    new_elt(ptr(elts, i, elt_size), i);
  }
  ht_divchn_pthread_init(&ht,
			 key_size,
			 elt_size,
			 500,
			 4,
			 alpha,
			 free_elt,
			 NULL);
  insert_keys_elts(&ht,
		   keys,
		   elts,
		   num_ins,
		   4,
		   &res);
  search_in_ht(&ht, keys, elts, num_ins, val_elt, &res);
  for (i = 0; i < num_ins; i++){
    *(size_t *)byte_ptr(ptr(keys, i, key_size),
			key_size - C_KEY_SIZE_FACTOR) = i + num_ins;
  }
  search_not_in_ht(&ht, keys, num_ins, &res);
  free_ht(&ht);
  printf("\t\tsearch correctness:             ");
  print_test_result(res);
  free(keys);
  free(elts);
  keys = NULL;
  elts = NULL;
}

/**
   Runs a corner cases test.
*/
void run_corner_cases_test(int ins_pow){
  int res = 1;
  int j;
  size_t i, k;
  size_t elt;
  size_t elt_size = sizeof(size_t);
  size_t key_size;
  size_t num_ins;
  void *key = NULL;
  ht_divchn_pthread_t ht;
  num_ins = pow_two(ins_pow);
  key = malloc_perror(1, pow_two(C_CORNER_KEY_POW_END));
  for (i = 0; i < pow_two(C_CORNER_KEY_POW_END); i++){
    *(unsigned char *)byte_ptr(key, i) = RANDOM();
  }
  printf("Run corner cases test --> ");
  for (j = C_CORNER_KEY_POW_START; j <= C_CORNER_KEY_POW_END; j++){
    key_size = pow_two(j);
    ht_divchn_pthread_init(&ht,
			   key_size,
			   elt_size,
			   4,
			   4,
			   C_CORNER_ALPHA,
			   NULL,
			   NULL);
    for (k = 0; k < num_ins; k++){
      elt = k;
      ht_divchn_pthread_insert(&ht, key, &elt, 1);
    }
    res *= (ht.count_ix == 0);
    res *= (ht.count == C_CORNER_HT_COUNT);
    res *= (ht.num_elts == 1);
    res *= (*(size_t *)ht_divchn_pthread_search(&ht, key) == elt);
    ht_divchn_pthread_delete(&ht, key);
    res *= (ht.count == C_CORNER_HT_COUNT);
    res *= (ht.num_elts == 0);
    res *= (ht_divchn_pthread_search(&ht, key) == NULL);
    ht_divchn_pthread_free(&ht);
  }
  print_test_result(res);
  free(key);
  key = NULL;
}

/**
   Helper functions.
*/

/**
   Computes a pointer to the ith byte in a block.
*/
void *byte_ptr(const void *block, size_t i){
  return (void *)((char *)block + i);
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

/**
   Times execution.
*/
double timer(){
  struct timeval tm;
  gettimeofday(&tm, NULL);
  return tm.tv_sec + tm.tv_usec / (double)1000000;
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
 /* if (args[8]) run_remove_delete_uint_test(args[0],
					   args[1],
					   args[2],
					   alpha_factor,
					   args[5],
					   args[6]); */
  if (args[9]) run_insert_search_free_uint_ptr_test(args[0],
						    args[1],
						    args[2],
						    alpha_factor,
						    args[5],
						    args[6]);
 /* if (args[10]) run_remove_delete_uint_ptr_test(args[0],
						args[1],
						args[2],
						alpha_factor,
						args[5],
						args[6]); */
  if (args[11]) run_corner_cases_test(args[0]); 
  free(args);
  args = NULL;
  return 0;
}
