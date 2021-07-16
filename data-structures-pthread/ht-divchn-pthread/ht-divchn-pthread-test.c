/**
   ht-divchn-pthread-test.c

   Tests of a hash table with generic hash keys and generic elements that 
   is concurrently accessible and modifiable. The implementation is based
   on a division method for hashing and a chaining method for resolving
   collisions.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99. The requirements are: i) CHAR_BIT * sizeof(size_t) is
   greater or equal to 16 and is even, and ii) pthreads API is available.
*/

#define _XOPEN_SOURCE 600

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
  "ht-divchn-pthread-test\n"
  "[0, # bits in size_t - 1) : i s.t. # inserts = 2**i\n"
  "[0, # bits in size_t) : a given k = sizeof(size_t)\n"
  "[0, # bits in size_t) : b s.t. k * 2**a <= key size <= k * 2**b\n"
  "> 0 : c\n"
  "> 0 : d\n"
  "> 0 : e log base 2\n"
  "> 0 : f s.t. c / 2**e <= alpha <= d / 2**e, in f steps\n"
  "[0, 1] : on/off insert search uint test\n"
  "[0, 1] : on/off remove delete uint test\n"
  "[0, 1] : on/off insert search uint_ptr test\n"
  "[0, 1] : on/off remove delete uint_ptr test\n"
  "[0, 1] : on/off corner cases test\n";
const int C_ARGC_MAX = 13;
const size_t C_ARGS_DEF[12] = {14, 0, 2, 1024, 30720u, 11, 10, 1, 1, 1, 1, 1};
const size_t C_SIZE_MAX = (size_t)-1;
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);

/* insert, search, free, remove, delete tests */
const size_t C_KEY_SIZE_FACTOR = sizeof(size_t);

/* corner cases test */
const size_t C_CORNER_LOG_KEY_START = 0;
const size_t C_CORNER_LOG_KEY_END = 8;
const size_t C_CORNER_HT_COUNT = 1543;
const size_t C_CORNER_ALPHA_N = 33;
const size_t C_CORNER_LOG_ALPHA_D = 15; /* alpha is 33/32768 */
const size_t C_CORNER_MIN_NUM = 0;
const size_t C_CORNER_NUM_LOCKS = 1;
const size_t C_CORNER_NUM_GROW_THREADS = 1;

void insert_search_free(size_t num_ins,
			size_t key_size,
			size_t elt_size,
			size_t alpha_n,
			size_t log_alpha_d,
			size_t num_threads,
			size_t log_num_locks,
			size_t num_grow_threads,
			size_t batch_count,
			void (*new_elt)(void *, size_t),
			size_t (*val_elt)(const void *),
			void (*free_elt)(void *));
void remove_delete(size_t num_ins,
		   size_t key_size,
		   size_t elt_size,
		   size_t alpha_n,
		   size_t log_alpha_d,
		   size_t num_threads,
		   size_t log_num_locks,
		   size_t num_grow_threads,
		   size_t batch_count,
		   void (*new_elt)(void *, size_t),
		   size_t (*val_elt)(const void *),
		   void (*free_elt)(void *));
void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);
double timer();

/**
   Test hash table operations on distinct keys and size_t elements 
   across key sizes and load factor upper bounds. For test purposes a key
   is random with the exception of a distinct non-random C_KEY_SIZE_FACTOR-
   sized block inside the key. A pointer to an element is passed as elt in
   ht_divchn_pthread_insert and the element is fully copied into the hash
   table. NULL as free_elt is sufficient to delete the element.
*/

void new_uint(void *elt, size_t val){
  size_t *s = elt;
  *s = val;
}

size_t val_uint(const void *elt){
  return *(size_t *)elt;
}

/**
   Runs a ht_divchn_pthread_{insert, search, free} test on distinct keys and 
   size_t elements across key sizes >= C_KEY_SIZE_FACTOR and load factor
   upper bounds.
*/
void run_insert_search_free_uint_test(size_t log_ins,
				      size_t log_key_start,
				      size_t log_key_end,
				      size_t alpha_n_start,
				      size_t alpha_n_end,
                                      size_t log_alpha_d,
				      size_t num_alpha_steps,
				      size_t num_threads,
				      size_t log_num_locks,
				      size_t num_grow_threads,
				      size_t batch_count){
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
    printf("Run a ht_divchn_pthread_{insert, search, free} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    printf("\t# threads (nt):   %lu\n"
	   "\t# locks:          %lu\n"
	   "\t# grow threads:   %lu\n"
	   "\tbatch count:      %lu\n",
	   TOLU(num_threads),
	   TOLU(pow_two_perror(log_num_locks)),
	   TOLU(num_grow_threads),
	   TOLU(batch_count));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\t# inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      insert_search_free(num_ins,
			 key_size,
			 elt_size,
			 alpha_n,
			 log_alpha_d,
			 num_threads,
			 log_num_locks,
			 num_grow_threads,
			 batch_count,
			 new_uint,
			 val_uint,
			 NULL);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Runs a ht_divchn_pthread_{remove, delete} test on distinct keys and
   size_t elements across key sizes >= C_KEY_SIZE_FACTOR and load factor
   upper bounds.
*/
void run_remove_delete_uint_test(size_t log_ins,
				 size_t log_key_start,
				 size_t log_key_end,
				 size_t alpha_n_start,
				 size_t alpha_n_end,
				 size_t log_alpha_d,
				 size_t num_alpha_steps,
				 size_t num_threads,
				 size_t log_num_locks,
				 size_t num_grow_threads,
				 size_t batch_count){
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
    printf("Run a ht_divchn_pthread_{remove, delete} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    printf("\t# threads (nt):   %lu\n"
	   "\t# locks:          %lu\n"
	   "\t# grow threads:   %lu\n"
	   "\tbatch count:      %lu\n",
	   TOLU(num_threads),
	   TOLU(pow_two_perror(log_num_locks)),
	   TOLU(num_grow_threads),
	   TOLU(batch_count));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      remove_delete(num_ins,
		    key_size,
		    elt_size,
		    alpha_n,
		    log_alpha_d,
		    num_threads,
		    log_num_locks,
		    num_grow_threads,
		    batch_count,
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
   Runs a ht_divchn_pthread_{insert, search, free} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_insert_search_free_uint_ptr_test(size_t log_ins,
					  size_t log_key_start,
					  size_t log_key_end,
					  size_t alpha_n_start,
					  size_t alpha_n_end,
					  size_t log_alpha_d,
					  size_t num_alpha_steps,
					  size_t num_threads,
					  size_t log_num_locks,
					  size_t num_grow_threads,
					  size_t batch_count){
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
    printf("Run a ht_divchn_pthread_{insert, search, free} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    printf("\t# threads (nt):   %lu\n"
	   "\t# locks:          %lu\n"
	   "\t# grow threads:   %lu\n"
	   "\tbatch count:      %lu\n",
	   TOLU(num_threads),
	   TOLU(pow_two_perror(log_num_locks)),
	   TOLU(num_grow_threads),
	   TOLU(batch_count));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      insert_search_free(num_ins,
			 key_size,
			 elt_size,
			 alpha_n,
			 log_alpha_d,
			 num_threads,
			 log_num_locks,
			 num_grow_threads,
			 batch_count,
			 new_uint_ptr,
			 val_uint_ptr,
			 free_uint_ptr);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Runs a ht_divchn_pthread_{remove, delete} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_remove_delete_uint_ptr_test(size_t log_ins,
				     size_t log_key_start,
				     size_t log_key_end,
				     size_t alpha_n_start,
				     size_t alpha_n_end,
				     size_t log_alpha_d,
				     size_t num_alpha_steps,
				     size_t num_threads,
				     size_t log_num_locks,
				     size_t num_grow_threads,
				     size_t batch_count){
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
    printf("Run a ht_divchn_pthread_{remove, delete} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    printf("\t# threads (nt):   %lu\n"
	   "\t# locks:          %lu\n"
	   "\t# grow threads:   %lu\n"
	   "\tbatch count:      %lu\n",
	   TOLU(num_threads),
	   TOLU(pow_two_perror(log_num_locks)),
	   TOLU(num_grow_threads),
	   TOLU(batch_count));
    for (j = 0; j <= num_alpha_steps; j++){
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), (float)alpha_n / pow_two_perror(log_alpha_d));
      remove_delete(num_ins,
		    key_size,
		    elt_size,
		    alpha_n,
		    log_alpha_d,
		    num_threads,
		    log_num_locks,
		    num_grow_threads,
		    batch_count,
		    new_uint_ptr,
		    val_uint_ptr,
		    free_uint_ptr);
      alpha_n += (j < num_alpha_steps) * step + (rem > 0 && rem--);
    }
  }
}

/**
   Helper functions for the ht_divchn_pthread_{insert, search, free} tests
   across key sizes and load factor upper bounds, on size_t and uint_ptr_t
   elements.
*/

/* Insert */

typedef struct{
  size_t start;
  size_t count;
  size_t batch_count;
  const void *keys;
  const void *elts;
  ht_divchn_pthread_t *ht;
} insert_arg_t;

void *insert_thread(void *arg){
  size_t i;
  const insert_arg_t *ia = arg;
  for (i = 0; i < ia->count; i += ia->batch_count){
    if (ia->count - i < ia->batch_count){
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
			       ia->batch_count);
    }
  }
  return NULL;
}

void insert_keys_elts(ht_divchn_pthread_t *ht,
		      const void *keys,
		      const void *elts,
		      size_t count,
		      size_t num_threads,
		      size_t batch_count,
		      int *res){
  size_t n = ht->num_elts;
  size_t init_count = ht->count;
  size_t i;
  size_t seg_count, rem_count;
  size_t start = 0;
  double t;
  pthread_t *iids = NULL;
  insert_arg_t *ias = NULL;
  iids = malloc_perror(num_threads, sizeof(pthread_t));
  ias = malloc_perror(num_threads, sizeof(insert_arg_t));
  seg_count = count / num_threads;
  rem_count = count % num_threads; /* distribute among threads */
  for (i = 0; i < num_threads; i++){
    ias[i].start = start;
    ias[i].count = seg_count;
    ias[i].batch_count = batch_count;
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
  if (init_count < ht->count){
    printf("\t\tinsert w/ growth time               "
	   "%.4f seconds\n", t);
  }else{
    printf("\t\tinsert w/o growth time              "
	   "%.4f seconds\n", t);
  }
  *res *= (ht->num_elts == n + count);
  free(iids);
  free(ias);
  iids = NULL;
  ias = NULL;
}

/* Search */

typedef struct{
  size_t start;
  size_t count;
  size_t *elt_count; /* for each thread */
  const void *keys;
  const void *elts;
  const ht_divchn_pthread_t *ht;
  size_t (*val_elt)(const void *);
} search_arg_t;

void *search_thread(void *arg){
  size_t i;
  const search_arg_t *sa = arg;
  for (i = 0; i < sa->count; i++){
    ht_divchn_pthread_search(sa->ht, ptr(sa->keys,
					 sa->start + i,
					 sa->ht->key_size));
  }
  return NULL;
}

void *search_res_thread(void *arg){
  size_t i;
  const void *elt = NULL;
  const search_arg_t *sa = arg;
  *(sa->elt_count) = 0;
  for (i = 0; i < sa->count; i++){
    elt = ht_divchn_pthread_search(sa->ht, ptr(sa->keys,
					       sa->start + i,
					       sa->ht->key_size));
    if (elt != NULL){
      *(sa->elt_count) +=
	(sa->val_elt(ptr(sa->elts, sa->start + i, sa->ht->elt_size)) ==
	 sa->val_elt(elt));
    }
  }
  return NULL;
}

size_t search_ht_helper(const ht_divchn_pthread_t *ht,
			const void *keys,
			const void *elts,
			size_t count,
			size_t num_threads,
			size_t (*val_elt)(const void *),
			double *t){
  size_t i;
  size_t ret = 0;
  size_t seg_count, rem_count;
  size_t start = 0;
  size_t *elt_counts = NULL;
  pthread_t *sids = NULL;
  search_arg_t *sas = NULL;
  elt_counts = calloc_perror(num_threads, sizeof(size_t));
  sids = malloc_perror(num_threads, sizeof(pthread_t));
  sas = malloc_perror(num_threads, sizeof(search_arg_t));
  seg_count = count / num_threads;
  rem_count = count - seg_count * num_threads; /* distribute among threads */
  for (i = 0; i < num_threads; i++){
    sas[i].start = start;
    sas[i].count = seg_count;
    if (rem_count > 0){
      sas[i].count++;
      rem_count--;
    }
    sas[i].elt_count = &elt_counts[i];
    sas[i].keys = keys;
    sas[i].elts = elts;
    sas[i].ht = ht;
    sas[i].val_elt = val_elt;
    start += sas[i].count;
  }
  /* timing */
  *t = timer();
  for (i = 1; i < num_threads; i++){
    thread_create_perror(&sids[i], search_thread, &sas[i]);
  }
  search_thread(&sas[0]);
  for (i = 1; i < num_threads; i++){
    thread_join_perror(sids[i], NULL);
  }
  *t = timer() - *t;
  /* correctness */
  for (i = 1; i < num_threads; i++){
    thread_create_perror(&sids[i], search_res_thread, &sas[i]);
  }
  search_res_thread(&sas[0]);
  ret += elt_counts[0];
  for (i = 1; i < num_threads; i++){
    thread_join_perror(sids[i], NULL);
    ret += elt_counts[i];
  }
  free(elt_counts);
  free(sids);
  free(sas);
  elt_counts = NULL;
  sids = NULL;
  sas = NULL;
  return ret;
}

void search_in_ht(const ht_divchn_pthread_t *ht,
		  const void *keys,
		  const void *elts,
		  size_t count,
		  size_t num_threads,
		  size_t (*val_elt)(const void *),
		  int *res){
  size_t n = ht->num_elts;
  double t;
  *res *=
    (search_ht_helper(ht, keys, elts, count, num_threads, val_elt, &t) ==
     ht->num_elts);
  *res *= (n == ht->num_elts);
  if (num_threads == 1){
    printf("\t\tin ht search time (nt = 1):         "
	   "%.4f seconds\n", t);
  }else{
    printf("\t\tin ht search time:                  "
	   "%.4f seconds\n", t);
  }
}

void search_nin_ht(const ht_divchn_pthread_t *ht,
		   const void *keys,
		   const void *elts,
		   size_t count,
		   size_t num_threads,
		   size_t (*val_elt)(const void *),
		   int *res){
  size_t n = ht->num_elts;
  double t;
  *res *=
    (search_ht_helper(ht, keys, elts, count, num_threads, val_elt, &t) == 0);
  *res *= (n == ht->num_elts);
  if (num_threads == 1){
    printf("\t\tnot in ht search time (nt = 1):     " 
	   "%.4f seconds\n", t);
  }else{
    printf("\t\tnot in ht search time:              "
	   "%.4f seconds\n", t);
  }
}

/* Search */

void free_ht(ht_divchn_pthread_t *ht, int verb){
  double t;;
  t = timer();
  ht_divchn_pthread_free(ht);
  t = timer() - t;
  if (verb){
    printf("\t\tfree time:                          "
	   "%.4f seconds\n", t);
  }
}

/* Insert, search, free */

void insert_search_free(size_t num_ins,
			size_t key_size,
			size_t elt_size,
			size_t alpha_n,
			size_t log_alpha_d,
			size_t num_threads,
			size_t log_num_locks,
			size_t num_grow_threads,
			size_t batch_count,
			void (*new_elt)(void *, size_t),
			size_t (*val_elt)(const void *),
			void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  void *key = NULL;
  void *keys = NULL, *elts = NULL;
  ht_divchn_pthread_t ht;
  keys = malloc_perror(num_ins, key_size);
  elts = malloc_perror(num_ins, elt_size);
  for (i = 0; i < num_ins; i++){
    key = ptr(keys, i, key_size);
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      /* set random bytes in a key, each to RANDOM mod 2**CHAR_BIT */
      *(unsigned char *)ptr(key, j, 1) = RANDOM();
    }
    /* set non-random bytes in a key, and create element */
    *(size_t *)ptr(key, key_size - C_KEY_SIZE_FACTOR, 1) = i;
    new_elt(ptr(elts, i, elt_size), i);
  }
  ht_divchn_pthread_init(&ht,
			 key_size,
			 elt_size,
			 0,
			 alpha_n,
			 log_alpha_d,
			 log_num_locks,
			 num_grow_threads,
			 NULL,
			 NULL); /* NULL to reinsert non-contig. elements */
  insert_keys_elts(&ht, keys, elts, num_ins, num_threads, batch_count, &res);
  free_ht(&ht, 0);
  ht_divchn_pthread_init(&ht,
			 key_size,
			 elt_size,
			 num_ins,
			 alpha_n,
			 log_alpha_d,
			 log_num_locks,
			 num_grow_threads,
			 NULL,
			 free_elt);
  insert_keys_elts(&ht, keys, elts, num_ins, num_threads, batch_count, &res);
  search_in_ht(&ht, keys, elts, num_ins, num_threads, val_elt, &res);
  search_in_ht(&ht, keys, elts, num_ins, 1, val_elt, &res);
  for (i = 0; i < num_ins; i++){
    key = ptr(keys, i, key_size);
    /* set non-random bytes in a key s.t. it is not in ht */
    *(size_t *)ptr(key, key_size - C_KEY_SIZE_FACTOR, 1) = i + num_ins;
  }
  search_nin_ht(&ht, keys, elts, num_ins, num_threads, val_elt, &res);
  search_nin_ht(&ht, keys, elts, num_ins, 1, val_elt, &res);
  free_ht(&ht, 1);
  printf("\t\tsearch correctness:                 ");
  print_test_result(res);
  free(keys);
  free(elts);
  keys = NULL;
  elts = NULL;
}

/**
   Helper functions for the ht_divchn_pthread_{remove, delete} tests
   across key sizes and load factor upper bounds, on size_t and 
   uint_ptr_t elements.
*/

/* Remove */

typedef struct{
  size_t start;
  size_t count;
  size_t batch_count;
  const void *keys;
  void *elts;
  ht_divchn_pthread_t *ht;
} remove_arg_t;

void *remove_thread(void *arg){
  size_t i;
  const remove_arg_t *ra = arg;
  for (i = 0; i < ra->count; i += ra->batch_count){
    if (ra->count - i < ra->batch_count){
      ht_divchn_pthread_remove(ra->ht,
			       ptr(ra->keys,
				   ra->start + i,
				   ra->ht->key_size),
			       ptr(ra->elts,
				   ra->start + i,
				   ra->ht->elt_size),
			       ra->count - i);
    }else{
      ht_divchn_pthread_remove(ra->ht,
			       ptr(ra->keys,
				   ra->start + i,
				   ra->ht->key_size),
			       ptr(ra->elts,
				   ra->start + i,
				   ra->ht->elt_size),
			       ra->batch_count);
    }
  }
  return NULL;
}

void remove_key_elts(ht_divchn_pthread_t *ht,
		     const void *keys,
		     void *elts,
		     size_t count,
		     size_t num_threads,
		     size_t batch_count,
		     int *res){
  size_t i;
  size_t seg_count, rem_count;
  size_t start = 0;
  double t;
  pthread_t *rids = NULL;
  remove_arg_t *ras = NULL;
  rids = malloc_perror(num_threads, sizeof(pthread_t));
  ras = malloc_perror(num_threads, sizeof(remove_arg_t));
  seg_count = count / num_threads;
  rem_count = count - seg_count * num_threads; /* distribute among threads */
  for (i = 0; i < num_threads; i++){
    ras[i].start = start;
    ras[i].count = seg_count;
    ras[i].batch_count = batch_count;
    if (rem_count > 0){
      ras[i].count++;
      rem_count--;
    }
    ras[i].keys = keys;
    ras[i].elts = elts;
    ras[i].ht = ht;
    start += ras[i].count;
  }
  t = timer();
  for (i = 1; i < num_threads; i++){
    thread_create_perror(&rids[i], remove_thread, &ras[i]);
  }
  /* use the parent thread as well */
  remove_thread(&ras[0]);
  for (i = 1; i < num_threads; i++){
    thread_join_perror(rids[i], NULL);
  }
  t = timer() - t;
  *res *= (ht->num_elts == 0);
  for (i = 0; i < count; i++){
    *res *=
      (ht_divchn_pthread_search(ht, ptr(keys, i, ht->key_size)) == NULL);
  }
  for (i = 0; i < ht->count; i++){
    *res *= (ht->key_elts[i] == NULL);
  }
  printf("\t\tremove time:                        "
	 "%.4f seconds\n", t);
  free(rids);
  free(ras);
  rids = NULL;
  ras = NULL;
}

/* Delete */

typedef struct{
  size_t start;
  size_t count;
  size_t batch_count;
  const void *keys;
  ht_divchn_pthread_t *ht;
} delete_arg_t;

void *delete_thread(void *arg){
  size_t i;
  const delete_arg_t *da = arg;
  for (i = 0; i < da->count; i += da->batch_count){
    if (da->count - i < da->batch_count){
      ht_divchn_pthread_delete(da->ht,
			       ptr(da->keys,
				   da->start + i,
				   da->ht->key_size),
			       da->count - i);
    }else{
      ht_divchn_pthread_delete(da->ht,
			       ptr(da->keys,
				   da->start + i,
				   da->ht->key_size),
			       da->batch_count);
    }
  }
  return NULL;
}

void delete_key_elts(ht_divchn_pthread_t *ht,
		     const void *keys,
		     size_t count,
		     size_t num_threads,
		     size_t batch_count,
		     int *res){
  size_t i;
  size_t seg_count, rem_count;
  size_t start = 0;
  double t;
  pthread_t *dids = NULL;
  delete_arg_t *das = NULL;
  dids = malloc_perror(num_threads, sizeof(pthread_t));
  das = malloc_perror(num_threads, sizeof(delete_arg_t));
  seg_count = count / num_threads;
  rem_count = count - seg_count * num_threads; /* distribute among threads */
  for (i = 0; i < num_threads; i++){
    das[i].start = start;
    das[i].count = seg_count;
    das[i].batch_count = batch_count;
    if (rem_count > 0){
      das[i].count++;
      rem_count--;
    }
    das[i].keys = keys;
    das[i].ht = ht;
    start += das[i].count;
  }
  t = timer();
  for (i = 1; i < num_threads; i++){
    thread_create_perror(&dids[i], delete_thread, &das[i]);
  }
  /* use the parent thread as well */
  delete_thread(&das[0]);
  for (i = 1; i < num_threads; i++){
    thread_join_perror(dids[i], NULL);
  }
  t = timer() - t;
  *res *= (ht->num_elts == 0);
  for (i = 0; i < count; i++){
    *res *=
      (ht_divchn_pthread_search(ht, ptr(keys, i, ht->key_size)) == NULL);
  }
  for (i = 0; i < ht->count; i++){
    *res *= (ht->key_elts[i] == NULL);
  }
  printf("\t\tdelete time:                        "
	 "%.4f seconds\n", t);
  free(dids);
  free(das);
  dids = NULL;
  das = NULL;
}

/* Remove, delete */

void remove_delete(size_t num_ins,
		   size_t key_size,
		   size_t elt_size,
		   size_t alpha_n,
		   size_t log_alpha_d,
		   size_t num_threads,
		   size_t log_num_locks,
		   size_t num_grow_threads,
		   size_t batch_count,
		   void (*new_elt)(void *, size_t),
		   size_t (*val_elt)(const void *),
		   void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  void *key = NULL;
  void *keys = NULL, *elts = NULL;
  ht_divchn_pthread_t ht;
  keys = malloc_perror(num_ins, key_size);
  elts = malloc_perror(num_ins, elt_size);
  for (i = 0; i < num_ins; i++){
    key = ptr(keys, i, key_size);
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      /* set random bytes in a key, each to RANDOM mod 2**CHAR_BIT */
      *(unsigned char *)ptr(key, j, 1) = RANDOM();
    }
    /* set non-random bytes in a key, and create element */
    *(size_t *)ptr(key, key_size - C_KEY_SIZE_FACTOR, 1) = i;
    new_elt(ptr(elts, i, elt_size), i);
  }
  ht_divchn_pthread_init(&ht,
			 key_size,
			 elt_size,
			 0,
			 alpha_n,
			 log_alpha_d,
			 log_num_locks,
			 num_grow_threads,
			 NULL,
			 free_elt);
  insert_keys_elts(&ht, keys, elts, num_ins, num_threads, batch_count, &res);
  memset(elts, 0, num_ins * elt_size);
  remove_key_elts(&ht, keys, elts, num_ins, num_threads, batch_count, &res);
  search_nin_ht(&ht, keys, elts, num_ins, num_threads, val_elt, &res);
  insert_keys_elts(&ht, keys, elts, num_ins, num_threads, batch_count, &res);
  search_in_ht(&ht, keys, elts, num_ins, num_threads, val_elt, &res);
  delete_key_elts(&ht, keys, num_ins, num_threads, batch_count, &res);
  free_ht(&ht, 1);
  printf("\t\tremove and delete correctness:      ");
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
  size_t i, j, k;
  size_t elt;
  size_t elt_size = sizeof(size_t);
  size_t key_size;
  size_t num_ins;
  void *key = NULL;
  ht_divchn_pthread_t ht;
  num_ins = pow_two_perror(log_ins);
  key = malloc_perror(1, pow_two_perror(C_CORNER_LOG_KEY_END));
  for (i = 0; i < pow_two_perror(C_CORNER_LOG_KEY_END); i++){
    *(unsigned char *)ptr(key, i, 1) = RANDOM();
  }
  printf("Run corner cases test --> ");
  for (j = C_CORNER_LOG_KEY_START; j <= C_CORNER_LOG_KEY_END; j++){
    key_size = pow_two_perror(j);
    ht_divchn_pthread_init(&ht,
			   key_size,
			   elt_size,
			   C_CORNER_MIN_NUM,
			   C_CORNER_ALPHA_N,
			   C_CORNER_LOG_ALPHA_D,
			   C_CORNER_NUM_LOCKS,
			   C_CORNER_NUM_GROW_THREADS,
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
    ht_divchn_pthread_delete(&ht, key, 1);
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

/**
   Times execution.
*/
double timer(){
  struct timeval tm;
  gettimeofday(&tm, NULL);
  return tm.tv_sec + tm.tv_usec / 1e6;
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
      args[6] < 1 ||
      args[7] > 1 ||
      args[8] > 1 ||
      args[9] > 1 ||
      args[10] > 1 ||
      args[11] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[7]) run_insert_search_free_uint_test(args[0],
						args[1],
						args[2],
						args[3],
						args[4],
						args[5],
						args[6],
						4,
						15,
						4,
						1000);
  if (args[8]) run_remove_delete_uint_test(args[0],
					   args[1],
					   args[2],
					   args[3],
					   args[4],
					   args[5],
					   args[6],
					   4,
					   15,
					   4,
					   1000);
  if (args[9]) run_insert_search_free_uint_ptr_test(args[0],
						    args[1],
						    args[2],
						    args[3],
						    args[4],
						    args[5],
						    args[6],
						    4,
						    15,
						    4,
						    1000);
  if (args[10]) run_remove_delete_uint_ptr_test(args[0],
						args[1],
						args[2],
						args[3],
						args[4],
						args[5],
						args[6],
						4,
						15,
						4,
						1000);
  if (args[11]) run_corner_cases_test(args[0]); 
  free(args);
  args = NULL;
  return 0;
}
