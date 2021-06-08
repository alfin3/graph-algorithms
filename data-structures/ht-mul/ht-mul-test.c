/**
   ht-mul-test.c

   Tests of a hash table with generic hash keys and generic elements.
   The implementation is based on a multiplication method for hashing and an 
   open addressing method for resolving collisions.

   The following command line arguments can be used to customize tests:
   ht-mul-test
      [0, # bits in size_t - 1) : i s.t. # inserts = 2^i
      [0, # bits in size_t) : a given k = sizeof(size_t)
      [0, # bits in size_t) : b s.t. k * 2^a <= key size <= k * 2^b
      > 0 : c
      > 0 : d s.t. c / d < 1.0
      > 0 : e
      > 0 : f s.t. e / f < 1.0
      > 0 : g s.t. c / d <= alpha <= e / f, in g steps
      [0, 1] : on/off insert search uint test
      [0, 1] : on/off remove delete uint test
      [0, 1] : on/off insert search uint_ptr test
      [0, 1] : on/off remove delete uint_ptr test
      [0, 1] : on/off corner cases test

   usage examples:
   ./ht-mul-test
   ./ht-mul-test 18
   ./ht-mul-test 17 5 6 
   ./ht-mul-test 19 0 2 2 10 3 10 10
   ./ht-mul-test 19 0 2 2 10 3 10 10 1 1 0 0 0

   ht-mul-test can be run with any subset of command line arguments in the
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
#include "ht-mul.h"
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
  "ht-mul-test \n"
  "[0, # bits in size_t - 1) : i s.t. # inserts = 2^i \n"
  "[0, # bits in size_t) : a given k = sizeof(size_t) \n"
  "[0, # bits in size_t) : b s.t. k * 2^a <= key size <= k * 2^b \n"
  "> 0 : c \n"
  "> 0 : d s.t. c / d < 1.0 \n"
  "> 0 : e \n"
  "> 0 : f s.t. e / f < 1.0 \n"
  "> 0 : g s.t. c / d <= alpha <= e / f, in g steps \n"
  "[0, 1] : on/off insert search uint test \n"
  "[0, 1] : on/off remove delete uint test \n"
  "[0, 1] : on/off insert search uint_ptr test \n"
  "[0, 1] : on/off remove delete uint_ptr test \n"
  "[0, 1] : on/off corner cases test \n";
const int C_ARGC_MAX = 14;
const size_t C_ARGS_DEF[13] = {14, 0, 3, 1, 10, 9, 10, 10, 1, 1, 1, 1, 1};
const size_t C_SIZE_MAX = (size_t)-1;
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);
const float C_ALPHA_MAX = 1.0; /* cannot be reached */

/* insert, search, free, remove, delete tests */
const size_t C_KEY_SIZE_FACTOR = sizeof(size_t);

/* corner cases test */
const unsigned char C_CORNER_KEY_A = 2;
const unsigned char C_CORNER_KEY_B = 1;
const size_t C_CORNER_KEY_SIZE = sizeof(unsigned char);
const size_t C_CORNER_HT_COUNT = 1024u;
const float C_CORNER_ALPHA = 0.001;

void insert_search_free(size_t num_ins,
			size_t key_size,
			size_t elt_size,
			float alpha,
                        size_t (*rdc_key)(const void *, size_t),
			void (*new_elt)(void *, size_t),
			size_t (*val_elt)(const void *),
			void (*free_elt)(void *));
void remove_delete(size_t num_ins,
		   size_t key_size,
		   size_t elt_size,
		   float alpha,
                   size_t (*rdc_key)(const void *, size_t),
		   void (*new_elt)(void *, size_t),
		   size_t (*val_elt)(const void *),
		   void (*free_elt)(void *));
void *byte_ptr(const void *block, size_t i);
void print_test_result(int res);

/**
   Test hash table operations on distinct keys and size_t elements 
   across key sizes and load factor upper bounds. For test purposes a key
   is random with the exception of a distinct non-random C_KEY_SIZE_FACTOR-
   sized block inside the key. A pointer to an element is passed as elt in
   ht_mul_insert and the element is fully copied into the hash table.
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
   Runs a ht_mul_{insert, search, free} test on distinct keys and 
   size_t elements across key sizes >= C_KEY_SIZE_FACTOR and load factor
   upper bounds.
*/
void run_insert_search_free_uint_test(int ins_pow,
				      int key_pow_start,
				      int key_pow_end,
				      float alpha_start,
				      float alpha_step,
				      int num_alpha_steps){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(size_t);
  float alpha;
  num_ins = pow_two(ins_pow);
  for (i = key_pow_start; i <= key_pow_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two(i);
    printf("Run a ht_mul_{insert, search, free} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = 0; j < num_alpha_steps; j++){
      alpha = alpha_start + j * alpha_step;
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), alpha);
      insert_search_free(num_ins,
			 key_size,
			 elt_size,
			 alpha,
                         NULL,
			 new_uint,
			 val_uint,
			 NULL);
    }
  }
}
/**
   Runs a ht_mul_{remove, delete} test on distinct keys and size_t
   elements across key sizes >= C_KEY_SIZE_FACTOR and load factor upper
   bounds.
*/
void run_remove_delete_uint_test(int ins_pow,
				 int key_pow_start,
				 int key_pow_end,
				 float alpha_start,
				 float alpha_step,
				 int num_alpha_steps){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(size_t);
  float alpha;
  num_ins = pow_two(ins_pow);
  for (i = key_pow_start; i <= key_pow_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two(i);
    printf("Run a ht_mul_{remove, delete} test on distinct "
	   "%lu-byte keys and size_t elements\n", TOLU(key_size));
    for (j = 0; j < num_alpha_steps; j++){
      alpha = alpha_start + j * alpha_step;
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), alpha);
      remove_delete(num_ins,
		    key_size,
		    elt_size,
		    alpha,
                    NULL,
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
   pointer to an element is passed as elt in ht_mul_insert, and the pointer
   to the element is copied into the hash table. An element-specific
   free_elt is necessary to delete the element.
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
   Runs a ht_mul_{insert, search, free} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_insert_search_free_uint_ptr_test(int ins_pow,
					  int key_pow_start,
					  int key_pow_end,
					  float alpha_start,
					  float alpha_step,
					  int num_alpha_steps){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size =  sizeof(uint_ptr_t *);
  float alpha;
  num_ins = pow_two(ins_pow);
  for (i = key_pow_start; i <= key_pow_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two(i);
    printf("Run a ht_mul_{insert, search, free} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    for (j = 0; j < num_alpha_steps; j++){
      alpha = alpha_start + j * alpha_step;
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), alpha);
      insert_search_free(num_ins,
			 key_size,
			 elt_size,
			 alpha,
                         NULL,
			 new_uint_ptr,
			 val_uint_ptr,
			 free_uint_ptr);
    }
  }
}

/**
   Runs a ht_mul_{remove, delete} test on distinct keys and 
   noncontiguous uint_ptr_t elements across key sizes >= C_KEY_SIZE_FACTOR
   and load factor upper bounds.
*/
void run_remove_delete_uint_ptr_test(int ins_pow,
				     int key_pow_start,
				     int key_pow_end,
				     float alpha_start,
				     float alpha_step,
				     int num_alpha_steps){
  int i, j;
  size_t num_ins;
  size_t key_size;
  size_t elt_size = sizeof(uint_ptr_t *);
  float alpha;
  num_ins = pow_two(ins_pow);
  for (i = key_pow_start; i <= key_pow_end; i++){
    key_size = C_KEY_SIZE_FACTOR * pow_two(i);
    printf("Run a ht_mul_{remove, delete} test on distinct "
	   "%lu-byte keys and noncontiguous uint_ptr_t elements\n",
	   TOLU(key_size));
    for (j = 0; j < num_alpha_steps; j++){
      alpha = alpha_start + j * alpha_step;
      printf("\tnumber of inserts: %lu, load factor upper bound: %.4f\n",
	     TOLU(num_ins), alpha);
      remove_delete(num_ins,
		    key_size,
		    elt_size,
		    alpha,
                    NULL,
		    new_uint_ptr,
		    val_uint_ptr,
		    free_uint_ptr);
    }
  }
}

/** 
   Helper functions for the ht_mul_{insert, search, free} tests
   across key sizes and load factor upper bounds, on size_t and 
   uint_ptr_t elements.
*/

void insert_keys_elts(ht_mul_t *ht,
		      void **keys,
		      void **elts,
		      size_t count,
		      int *res){
  size_t n = ht->num_elts;
  size_t i;
  clock_t t;
  t = clock();
  for (i = 0; i < count; i++){
    ht_mul_insert(ht, keys[i], elts[i]);
  }
  t = clock() - t;
  printf("\t\tinsert time:                    "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n + count);
}

void search_in_ht(const ht_mul_t *ht,
		  void **keys,
		  void **elts,
		  size_t count,
		  int *res,
		  size_t (*val_elt)(const void *)){
  size_t n = ht->num_elts;
  size_t i;
  void *elt = NULL;
  clock_t t;
  t = clock();
  for (i = 0; i < count; i++){
    elt = ht_mul_search(ht, keys[i]);
  }
  t = clock() - t;
  for (i = 0; i < count; i++){
    elt = ht_mul_search(ht, keys[i]);
    *res *= (val_elt(elts[i]) == val_elt(elt));
  }
  printf("\t\tin ht search time:              "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void search_not_in_ht(const ht_mul_t *ht,
		      void **keys,
		      size_t count,
		      int *res){
  size_t n = ht->num_elts;
  size_t i;
  void *elt = NULL;
  clock_t t;
  t = clock();
  for (i = 0; i < count; i++){
    elt = ht_mul_search(ht, keys[i]);
  }
  t = clock() - t;
  for (i = 0; i < count; i++){
    elt = ht_mul_search(ht, keys[i]);
    *res *= (elt == NULL);
  }
  printf("\t\tnot in ht search time:          "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void free_ht(ht_mul_t *ht){
  clock_t t;
  t = clock();
  ht_mul_free(ht);
  t = clock() - t;
  printf("\t\tfree time:                      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

void insert_search_free(size_t num_ins,
			size_t key_size,
			size_t elt_size,
			float alpha,
                        size_t (*rdc_key)(const void *, size_t),
			void (*new_elt)(void *, size_t),
			size_t (*val_elt)(const void *),
			void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  void **keys = NULL, **elts = NULL;
  ht_mul_t ht;
  keys = malloc_perror(num_ins, sizeof(void *));
  elts = malloc_perror(num_ins, sizeof(void *));
  for (i = 0; i < num_ins; i++){
    keys[i] = malloc_perror(1, key_size);
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      *(unsigned char *)byte_ptr(keys[i], j) = RANDOM(); /* mod 2^CHAR_BIT */
    }
    *(size_t *)byte_ptr(keys[i], key_size - C_KEY_SIZE_FACTOR) = i;
    elts[i] = malloc_perror(1, elt_size);
    new_elt(elts[i], i);
  }
  ht_mul_init(&ht, key_size, elt_size, alpha, rdc_key, free_elt);
  insert_keys_elts(&ht, keys, elts, num_ins, &res);
  search_in_ht(&ht, keys, elts, num_ins, &res, val_elt);
  for (i = 0; i < num_ins; i++){
    *(size_t *)byte_ptr(keys[i], key_size - C_KEY_SIZE_FACTOR) = i + num_ins;
  }
  search_not_in_ht(&ht, keys, num_ins, &res);
  free_ht(&ht);
  printf("\t\tsearch correctness:             ");
  print_test_result(res);
  for (i = 0; i < num_ins; i++){
    free(keys[i]);
    free(elts[i]);
  }
  free(keys);
  free(elts);
  keys = NULL;
  elts = NULL;
}

/** 
   Helper functions for the ht_mul_{remove, delete} tests
   across key sizes and load factor upper bounds, on size_t and 
   uint_ptr_t elements.
*/

void remove_key_elts(ht_mul_t *ht,
		     void **keys,
		     void **elts,
		     size_t count,
		     int *res,
		     size_t (*val_elt)(const void *)){
  size_t n = ht->num_elts;
  size_t i;
  void *ptr = NULL, *elt = NULL;
  clock_t t_first_half, t_second_half;
  elt = malloc_perror(1, ht->elt_size);
  t_first_half = clock();
  for (i = 0; i < count; i += 2){
    ht_mul_remove(ht, keys[i], elt);
    /* if an element is noncontiguous, it is still accessible from elts[i] */
  }
  t_first_half = clock() - t_first_half;
  *res *= (ht->num_elts == ((count & 1) ?
			    (n - count / 2 - 1) :
			    (n - count / 2)));
  for (i = 0; i < count; i++){
    if (i & 1){
      ptr = ht_mul_search(ht, keys[i]);
      *res *= (val_elt(elts[i]) == val_elt(ptr));
    }else{
      *res *= (ht_mul_search(ht, keys[i]) == NULL);
    }
  }
  t_second_half = clock();
  for (i = 1; i < count; i += 2){
    ht_mul_remove(ht, keys[i], elt);
    /* if an element is noncontiguous, it is still accessible from elts[i] */
  }
  t_second_half = clock() - t_second_half;
  *res *= (ht->num_elts == 0);
  for (i = 0; i < count; i++){
    *res *= (ht_mul_search(ht, keys[i]) == NULL);
  }
  printf("\t\tremove 1/2 elements time:       "
	 "%.4f seconds\n", (float)t_first_half / CLOCKS_PER_SEC);
  printf("\t\tremove residual elements time:  "
	 "%.4f seconds\n", (float)t_second_half / CLOCKS_PER_SEC);
  free(elt);
  elt = NULL;
}

void delete_key_elts(ht_mul_t *ht,
		     void **keys,
		     void **elts,
		     size_t count,
		     int *res,
		     size_t (*val_elt)(const void *)){
  size_t n = ht->num_elts;
  size_t i;
  void *ptr;
  clock_t t_first_half, t_second_half;
  t_first_half = clock();
  for (i = 0; i < count; i += 2){
    ht_mul_delete(ht, keys[i]);
  }
  t_first_half = clock() - t_first_half;
  *res *= (ht->num_elts == ((count & 1) ?
			    (n - count / 2 - 1) :
			    (n - count / 2)));
  for (i = 0; i < count; i++){
    if (i & 1){
      ptr = ht_mul_search(ht, keys[i]);
      *res *= (val_elt(elts[i]) == val_elt(ptr));
    }else{
      *res *= (ht_mul_search(ht, keys[i]) == NULL);
    }
  }
  t_second_half = clock();
  for (i = 1; i < count; i += 2){
    ht_mul_delete(ht, keys[i]);
  }
  t_second_half = clock() - t_second_half;
  *res *= (ht->num_elts == 0);
  for (i = 0; i < count; i++){
    *res *= (ht_mul_search(ht, keys[i]) == NULL);
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
                   size_t (*rdc_key)(const void *, size_t),
		   void (*new_elt)(void *, size_t),
		   size_t (*val_elt)(const void *),
		   void (*free_elt)(void *)){
  int res = 1;
  size_t i, j;
  void **keys = NULL, **elts = NULL;
  ht_mul_t ht;
  keys = malloc_perror(num_ins, sizeof(void *));
  elts = malloc_perror(num_ins, sizeof(void *));
  for (i = 0; i < num_ins; i++){
    keys[i] = malloc_perror(1, key_size);
    for (j = 0; j < key_size - C_KEY_SIZE_FACTOR; j++){
      *(unsigned char *)byte_ptr(keys[i], j) = RANDOM(); /* mod 2^CHAR_BIT */
    }
    *(size_t *)byte_ptr(keys[i], key_size - C_KEY_SIZE_FACTOR) = i;
    elts[i] = malloc_perror(1, elt_size);
    new_elt(elts[i], i);
  }
  ht_mul_init(&ht, key_size, elt_size, alpha, rdc_key, free_elt);
  insert_keys_elts(&ht, keys, elts, num_ins, &res);
  remove_key_elts(&ht, keys, elts, num_ins, &res, val_elt);
  insert_keys_elts(&ht, keys, elts, num_ins, &res);
  delete_key_elts(&ht, keys, elts, num_ins, &res, val_elt);
  free_ht(&ht);
  printf("\t\tremove and delete correctness:  ");
  print_test_result(res);
  for (i = 0; i < num_ins; i++){
    free(keys[i]);
    free(elts[i]);
  }
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
  size_t elt;
  size_t elt_size = sizeof(size_t);
  size_t i, num_ins;
  ht_mul_t ht;
  ht_mul_init(&ht, C_CORNER_KEY_SIZE, elt_size, C_CORNER_ALPHA, NULL, NULL);
  num_ins = pow_two(ins_pow);
  printf("Run corner cases test --> ");
  for (i = 0; i < num_ins; i++){
    elt = i;
    ht_mul_insert(&ht, &C_CORNER_KEY_A, &elt);
  }
  res *= (ht.count == C_CORNER_HT_COUNT);
  res *= (ht.num_elts == 1);
  res *= (*(size_t *)ht_mul_search(&ht, &C_CORNER_KEY_A) == elt);
  res *= (ht_mul_search(&ht, &C_CORNER_KEY_B) == NULL);
  ht_mul_insert(&ht, &C_CORNER_KEY_B, &elt);
  res *= (ht.count == C_CORNER_HT_COUNT);
  res *= (ht.num_elts == 2);
  res *= (*(size_t *)ht_mul_search(&ht, &C_CORNER_KEY_A) == elt);
  res *= (*(size_t *)ht_mul_search(&ht, &C_CORNER_KEY_B) == elt);
  ht_mul_delete(&ht, &C_CORNER_KEY_A);
  res *= (ht.count == C_CORNER_HT_COUNT);
  res *= (ht.num_elts == 1);
  res *= (ht_mul_search(&ht, &C_CORNER_KEY_A) == NULL);
  res *= (*(size_t *)ht_mul_search(&ht, &C_CORNER_KEY_B) == elt);
  ht_mul_delete(&ht, &C_CORNER_KEY_B);
  res *= (ht.count == C_CORNER_HT_COUNT);
  res *= (ht.num_elts == 0);
  res *= (ht_mul_search(&ht, &C_CORNER_KEY_A) == NULL);
  res *= (ht_mul_search(&ht, &C_CORNER_KEY_B) == NULL);
  print_test_result(res);
  free_ht(&ht);
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

int main(int argc, char *argv[]){
  int i;
  int num_alpha_steps;
  size_t *args = NULL;
  float alpha_start, alpha_end, alpha_step;
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
      args[5] < 1 ||
      args[6] < 1 ||
      args[7] < 1 ||
      args[8] > 1 ||
      args[9] > 1 ||
      args[10] > 1 ||
      args[11] > 1 ||
      args[12] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  alpha_start = (float)args[3] / args[4];
  alpha_end = (float)args[5] / args[6];
  if (alpha_start >= C_ALPHA_MAX ||
      alpha_end >= C_ALPHA_MAX){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  alpha_step = (alpha_end - alpha_start) / args[7];
  num_alpha_steps = (alpha_end == alpha_start) ? args[7] : args[7] + 1;
  if (args[8]) run_insert_search_free_uint_test(args[0],
						args[1],
						args[2],
						alpha_start,
						alpha_step,
						num_alpha_steps);
  if (args[9]) run_remove_delete_uint_test(args[0],
					   args[1],
					   args[2],
					   alpha_start,
					   alpha_step,
					   num_alpha_steps);
  if (args[10]) run_insert_search_free_uint_ptr_test(args[0],
						    args[1],
						    args[2],
						    alpha_start,
						    alpha_step,
						    num_alpha_steps);
  if (args[11]) run_remove_delete_uint_ptr_test(args[0],
						args[1],
						args[2],
						alpha_start,
						alpha_step,
						num_alpha_steps);
  if (args[12]) run_corner_cases_test(args[0]);
  free(args);
  args = NULL;
  return 0;
}
