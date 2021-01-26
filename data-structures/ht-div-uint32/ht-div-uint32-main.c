/**
   ht-div-uint32-main.c

   Tests of a hash table with generic hash keys and generic elements.
   The implementation is based on a division method for hashing and a
   chaining method for resolving collisions.
   
   Requirements for running tests: 
   - UINT32_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "ht-div-uint32.h"
#include "dll.h"
#include "utilities-mem.h"

#define RGENS_SEED() do{srandom(time(0)); srand48(random());}while (0)
#define RANDOM() (random())
#define DRAND48() (drand48())

void print_test_result(int res);
void insert_search_free_alphas(uint32_t num_inserts,
			       float alphas[],
			       int alphas_count,
			       uint32_t key_size,
			       uint32_t elt_size,
			       int (*cmp_key)(const void *, const void *),
			       void (*new_elt)(void *, uint32_t),
			       uint32_t (*elt_val)(const void *),
			       void (*free_elt)(void *));
void remove_delete_alphas(uint32_t num_inserts,
			  float alphas[],
			  int alphas_count,
			  uint32_t key_size,
			  uint32_t elt_size,
			  int (*cmp_key)(const void *, const void *),
			  void (*new_elt)(void *, uint32_t),
			  uint32_t (*elt_val)(const void *),
			  void (*free_elt)(void *));

/**
   Test hash table operations on distinct keys and uint32_t elements across
   key sizes and load factor upper bounds. A pointer to an element is passed
   as elt in ht_div_uint32_insert and the element is fully copied into the
   block pointed to from a node of the dll at the slot index computed by a
   hash function. NULL as free_elt is sufficient to delete the element.
*/

int cmp_uint32_fn(const void *a, const void *b){
  return memcmp(a, b, sizeof(uint32_t));
}

int cmp_32_fn(const void *a, const void *b){
  return memcmp(a, b, 32);
}

int cmp_256_fn(const void *a, const void *b){
  return memcmp(a, b, 256);
}

void new_uint32_fn(void *elt, uint32_t val){
  uint32_t *s = elt;
  *s = val;
  s = NULL;
}

uint32_t val_uint32_fn(const void *elt){
  return *(uint32_t *)elt;
}

/**
   Runs a ht_div_uint32_{insert, search, free} test on distinct keys and 
   uint32_t elements across key sizes and load factor upper bounds.
*/
void run_insert_search_free_uint32_test(){
  int alphas_count = 4;
  int key_sizes_count = 3;
  uint32_t key_sizes[3] = {sizeof(uint32_t), 32, 256}; //>= sizeof(uint32_t)
  uint32_t num_inserts = 1000000; //< 2^31
  float alphas[4] = {0.1, 1.0, 10.0, 100.0};
  int (*cmp_fn_arr[3])(const void *, const void *) = {cmp_uint32_fn,
						      cmp_32_fn,
						      cmp_256_fn};
  for (int i = 0; i < key_sizes_count; i++){
    printf("Run a ht_div_uint32_{insert, search, free} test on distinct "
	   "%d-byte keys and uint32_t elements\n", key_sizes[i]);
    insert_search_free_alphas(num_inserts,
			      alphas,
			      alphas_count,
			      key_sizes[i],
			      sizeof(uint32_t),
			      cmp_fn_arr[i],
			      new_uint32_fn,
			      val_uint32_fn,
			      NULL);
  }
}

/**
   Runs a ht_div_uint32_{remove, delete} test on distinct keys and uint32_t
   elements across key sizes and load factor upper bounds.
*/
void run_remove_delete_uint32_test(){
  int alphas_count = 4;
  int key_sizes_count = 3;
  uint32_t key_sizes[3] = {sizeof(uint32_t), 32, 256}; //>= sizeof(uint32_t)
  uint32_t num_inserts = 1000000; //< 2^31
  float alphas[4] = {0.1, 1.0, 10.0, 100.0};
  int (*cmp_fn_arr[3])(const void *, const void *) = {cmp_uint32_fn,
						      cmp_32_fn,
						      cmp_256_fn};
  for (int i = 0; i < key_sizes_count; i++){
    printf("Run a ht_div_uint32_{remove, delete} test on distinct "
	   "%d-byte keys and uint32_t elements\n", key_sizes[i]);
    remove_delete_alphas(num_inserts,
			 alphas,
			 alphas_count,
			 key_sizes[i],
			 sizeof(uint32_t),
			 cmp_fn_arr[i],
			 new_uint32_fn,
			 val_uint32_fn,
			 NULL);
  }
}

/**
   Test hash table operations on distinct keys and noncontiguous uint32_ptr_t
   elements across key sizes and load factor upper bounds. A pointer to a
   pointer to an element is passed as elt in ht_div_uint32_insert, and
   the pointer to the element is copied into the block pointed to from a node
   of the dll at the slot index computed by a hash function. An element-
   specific free_elt is necessary to delete the element.
*/

typedef struct{
  uint32_t *val;
} uint32_ptr_t;

void new_uint32_ptr_fn(void *elt, uint32_t val){
  uint32_ptr_t **s = elt;
  *s = malloc_perror(sizeof(uint32_ptr_t));
  (*s)->val = malloc_perror(sizeof(uint32_t));
  *((*s)->val) = val;
  s = NULL;
}

uint32_t val_uint32_ptr_fn(const void *elt){
  uint32_ptr_t **s  = (uint32_ptr_t **)elt;
  return *((*s)->val);
}

void free_uint32_ptr_fn(void *elt){
  uint32_ptr_t **s = elt;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

/**
   Runs a ht_div_uint32_{insert, search, free} test on distinct keys and 
   noncontiguous uint32_ptr_t elements across key sizes and load factor 
   upper bounds.
*/
void run_insert_search_free_uint32_ptr_test(){
  int alphas_count = 4;
  int key_sizes_count = 3;
  uint32_t key_sizes[3] = {sizeof(uint32_t), 32, 256}; // >= sizeof(uint32_t)
  uint32_t num_inserts = 1000000; //< 2^31 for this test
  float alphas[4] = {0.1, 1.0, 10.0, 100.0};
  int (*cmp_fn_arr[3])(const void *, const void *) = {cmp_uint32_fn,
						      cmp_32_fn,
						      cmp_256_fn};
  for (int i = 0; i < key_sizes_count; i++){
    printf("Run a ht_div_uint32_{insert, search, free} test on distinct "
	   "%d-byte keys and noncontiguous uint32_ptr_t elements\n",
	   key_sizes[i]);
    insert_search_free_alphas(num_inserts,
			      alphas,
			      alphas_count,
			      key_sizes[i],
			      sizeof(uint32_ptr_t *),
			      cmp_fn_arr[i],
			      new_uint32_ptr_fn,
			      val_uint32_ptr_fn,
			      free_uint32_ptr_fn);
  }
}

/**
   Runs a ht_div_uint32_{remove, delete} test on distinct keys and 
   noncontiguous uint32_ptr_t elements across key sizes and load factor 
   upper bounds.
*/
void run_remove_delete_uint32_ptr_test(){
  int alphas_count = 4;
  int key_sizes_count = 3;
  uint32_t key_sizes[3] = {sizeof(uint32_t), 32, 256}; //>= sizeof(uint32_t)
  uint32_t num_inserts = 1000000; //< 2^31
  float alphas[4] = {0.1, 1.0, 10.0, 100.0};
  int (*cmp_fn_arr[3])(const void *, const void *) = {cmp_uint32_fn,
						      cmp_32_fn,
						      cmp_256_fn};
  for (int i = 0; i < key_sizes_count; i++){
    printf("Run a ht_div_uint32_{remove, delete} test on distinct "
	   "%d-byte keys and noncontiguous uint32_ptr_t elements\n",
	   key_sizes[i]);
    remove_delete_alphas(num_inserts,
			 alphas,
			 alphas_count,
			 key_sizes[i],
			 sizeof(uint32_ptr_t *),
			 cmp_fn_arr[i],
			 new_uint32_ptr_fn,
			 val_uint32_ptr_fn,
			 free_uint32_ptr_fn);
  }
}

/** 
   Helper functions for the ht_div_uint32_{insert, search, free} tests
   across key sizes and load factor upper bounds, on uint32_t and 
   uint32_ptr_t elements.
*/

void insert_keys_elts(ht_div_uint32_t *ht,
		      void **keys,
		      void **elts,
		      uint32_t count,
		      int *res){
  uint32_t n = ht->num_elts;
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < count; i++){
    ht_div_uint32_insert(ht, keys[i], elts[i]);
  }
  t = clock() - t;
  printf("\t\tinsert time:                    "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n + count);
}

void search_in_ht(const ht_div_uint32_t *ht,
		  void **keys,
		  void **elts,
		  uint32_t count,
		  int *res,
		  uint32_t (*elt_val)(const void *)){
  uint32_t n = ht->num_elts;
  void *elt = NULL;
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < count; i++){
    elt = ht_div_uint32_search(ht, keys[i]);
    *res *= (elt_val(elts[i]) == elt_val(elt));
  }
  t = clock() - t;
  printf("\t\tin ht search time:              "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void search_not_in_ht(const ht_div_uint32_t *ht,
		      void **keys,
		      uint32_t count,
		      int *res){
  uint32_t n = ht->num_elts;
  void *elt = NULL;
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < count; i++){
    elt = ht_div_uint32_search(ht, keys[i]);
    *res *= (elt == NULL);
  }
  t = clock() - t;
  printf("\t\tnot in ht search time:          "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void free_ht(ht_div_uint32_t *ht){
  clock_t t;
  t = clock();
  ht_div_uint32_free(ht);
  t = clock() - t;
  printf("\t\tfree time:                      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

void insert_search_free(uint32_t num_inserts,
			float alpha,
			uint32_t key_size,
			uint32_t elt_size,
			int (*cmp_key)(const void *, const void *),
			void (*new_elt)(void *, uint32_t),
			uint32_t (*elt_val)(const void *),
			void (*free_elt)(void *)){
  int res = 1;
  void **keys = NULL, **elts = NULL, *ptr = NULL;
  ht_div_uint32_t ht;
  keys = malloc_perror(num_inserts * sizeof(void *));
  elts = malloc_perror(num_inserts * sizeof(void *));
  for (uint32_t i = 0; i < num_inserts; i++){
    keys[i] = malloc_perror(key_size);
    ptr = (char *)(keys[i] + key_size - sizeof(uint32_t));
    *(uint32_t *)ptr = i;
    elts[i] = malloc_perror(elt_size);
    new_elt(elts[i], i);
  }
  ht_div_uint32_init(&ht, key_size, elt_size, alpha, cmp_key, free_elt);
  insert_keys_elts(&ht, keys, elts, num_inserts, &res);
  search_in_ht(&ht, keys, elts, num_inserts, &res, elt_val);
  for (uint32_t i = 0; i < num_inserts; i++){
    ptr = (char *)(keys[i] + key_size - sizeof(uint32_t));
    *(uint32_t *)ptr = i + num_inserts;
  }
  search_not_in_ht(&ht, keys, num_inserts, &res);
  free_ht(&ht);
  printf("\t\tsearch correctness:             ");
  print_test_result(res);
  for (uint32_t i = 0; i < num_inserts; i++){
    free(keys[i]);
    free(elts[i]);
  }
  free(keys);
  free(elts);
  keys = NULL;
  elts = NULL;
}

void insert_search_free_alphas(uint32_t num_inserts,
			       float alphas[],
			       int alphas_count,
			       uint32_t key_size,
			       uint32_t elt_size,
			       int (*cmp_key)(const void *, const void *),
			       void (*new_elt)(void *, uint32_t),
			       uint32_t (*elt_val)(const void *),
			       void (*free_elt)(void *)){
  for (int i = 0; i < alphas_count; i++){
    printf("\tnumber of inserts: %u, load factor upper bound: %.1f\n",
	   num_inserts, alphas[i]);
    insert_search_free(num_inserts,
		       alphas[i],
		       key_size,
		       elt_size,
		       cmp_key,
		       new_elt,
		       elt_val,
		       free_elt);
  }
}

/** 
   Helper functions for the ht_div_uint32_{remove, delete} tests
   across key sizes and load factor upper bounds, on uint32_t and 
   uint32_ptr_t elements.
*/

void remove_key_elts(ht_div_uint32_t *ht,
		     void **keys,
		     void **elts,
		     uint32_t count,
		     int *res,
		     uint32_t (*elt_val)(const void *)){
  uint32_t n = ht->num_elts;
  uint32_t c = 0;
  void *ptr = NULL, *elt = NULL;
  clock_t t;
  elt = malloc_perror(ht->elt_size);
  t = clock();
  for (uint32_t i = 0; i < count; i += 2){
    ht_div_uint32_remove(ht, keys[i], elt);
    *res *= (elt_val(elts[i]) == elt_val(elt));
    //if an element is noncontiguous, it is still accessible from elts[i]
    c++;
  }
  t = clock() - t;
  *res *= (ht->num_elts == n - c);
  printf("\t\tremove 1/2 elements time:       "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint32_t i = 0; i < count; i++){
    if (i % 2){
      ptr = ht_div_uint32_search(ht, keys[i]);
      *res *= (elt_val(elts[i]) == elt_val(ptr));
    }else{
      *res *= (ht_div_uint32_search(ht, keys[i]) == NULL);
    }
  }
  t = clock();
  for (uint32_t i = 1; i < count; i += 2){
    ht_div_uint32_remove(ht, keys[i], elt);
    *res *= (elt_val(elts[i]) == elt_val(elt));
    //if an element is noncontiguous, it is still accessible from elts[i]
  }
  t = clock() - t;
  *res *= (ht->num_elts == 0);
  printf("\t\tremove residual elements time:  "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint32_t i = 0; i < count; i++){
    *res *= (ht_div_uint32_search(ht, keys[i]) == NULL);
  }
  for (uint32_t i = 0; i < ht->count; i++){
    *res *= (ht->key_elts[i] == NULL);
  }
  free(elt);
  elt = NULL;
}

void delete_key_elts(ht_div_uint32_t *ht,
		     void **keys,
		     void **elts,
		     uint32_t count,
		     int *res,
		     uint32_t (*elt_val)(const void *)){
  uint32_t n = ht->num_elts;
  uint32_t c = 0;
  void *ptr;
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < count; i += 2){
    ht_div_uint32_delete(ht, keys[i]);
    c++;
  }
  t = clock() - t;
  *res *= (ht->num_elts == n - c);
  printf("\t\tdelete 1/2 elements time:       "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint32_t i = 0; i < count; i++){
    if (i % 2){
      ptr = ht_div_uint32_search(ht, keys[i]);
      *res *= (elt_val(elts[i]) == elt_val(ptr));
    }else{
      *res *= (ht_div_uint32_search(ht, keys[i]) == NULL);
    }
  }
  t = clock();
  for (uint32_t i = 1; i < count; i += 2){
    ht_div_uint32_delete(ht, keys[i]);
  }
  t = clock() - t;
  *res *= (ht->num_elts == 0);
  printf("\t\tdelete residual elements time:  "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint32_t i = 0; i < count; i++){
    *res *= (ht_div_uint32_search(ht, keys[i]) == NULL);
  }
  for (uint32_t i = 0; i < ht->count; i++){
    *res *= (ht->key_elts[i] == NULL);
  }
}

void remove_delete(uint32_t num_inserts,
		   float alpha,
		   uint32_t key_size,
		   uint32_t elt_size,
		   int (*cmp_key)(const void *, const void *),
		   void (*new_elt)(void *, uint32_t),
		   uint32_t (*elt_val)(const void *),
		   void (*free_elt)(void *)){
  int res = 1;
  void **keys = NULL, **elts = NULL, *ptr = NULL;
  ht_div_uint32_t ht;
  keys = malloc_perror(num_inserts * sizeof(void *));
  elts = malloc_perror(num_inserts * sizeof(void *));
  for (uint32_t i = 0; i < num_inserts; i++){
    keys[i] = malloc_perror(key_size);
    ptr = (char *)(keys[i] + key_size - sizeof(uint32_t));
    *(uint32_t *)ptr = i;
    elts[i] = malloc_perror(elt_size);
    new_elt(elts[i], i);
  }
  ht_div_uint32_init(&ht, key_size, elt_size, alpha, cmp_key, free_elt);
  insert_keys_elts(&ht, keys, elts, num_inserts, &res);
  remove_key_elts(&ht, keys, elts, num_inserts, &res, elt_val);
  insert_keys_elts(&ht, keys, elts, num_inserts, &res);
  delete_key_elts(&ht, keys, elts, num_inserts, &res, elt_val);
  free_ht(&ht);
  printf("\t\tremove and delete correctness:  ");
  print_test_result(res);
  for (uint32_t i = 0; i < num_inserts; i++){
    free(keys[i]);
    free(elts[i]);
  }
  free(keys);
  free(elts);
  keys = NULL;
  elts = NULL;
}

void remove_delete_alphas(uint32_t num_inserts,
			  float alphas[],
			  int alphas_count,
			  uint32_t key_size,
			  uint32_t elt_size,
			  int (*cmp_key)(const void *, const void *),
			  void (*new_elt)(void *, uint32_t),
			  uint32_t (*elt_val)(const void *),
			  void (*free_elt)(void *)){
  for (int i = 0; i < alphas_count; i++){
    printf("\tnumber of inserts: %u, load factor upper bound: %.1f\n",
	   num_inserts, alphas[i]);
    remove_delete(num_inserts,
		  alphas[i],
		  key_size,
		  elt_size,
		  cmp_key,
		  new_elt,
		  elt_val,
		  free_elt);
  }
}

/**
   Runs a corner cases test.
*/
void run_corner_cases_test(){
  int res = 1;
  uint32_t elt;
  uint32_t key_size = 256;
  uint32_t num_inserts = 100000;
  uint32_t ht_count = 1543;
  float alpha = 0.001;
  void *key = NULL;
  ht_div_uint32_t ht;
  ht_div_uint32_init(&ht,
                     key_size,
	             sizeof(uint32_t),
		     alpha,
                     cmp_256_fn,
		     NULL);
  key = malloc_perror(key_size);
  for (uint32_t i = 0; i < key_size; i++){
    *((char *)key + i) = DRAND48() * 0xff;
  }
  for (uint32_t i = 0; i < num_inserts; i++){
    elt = i;
    ht_div_uint32_insert(&ht, key, &elt);
  }
  res *= (ht.count_ix == 0);
  res *= (ht.count == ht_count);
  res *= (ht.num_elts == 1);
  res *= (*(uint32_t *)ht_div_uint32_search(&ht, key) == elt);
  ht_div_uint32_delete(&ht, key);
  res *= (ht.count == ht_count);
  res *= (ht.num_elts == 0);
  res *= (ht_div_uint32_search(&ht, key) == NULL);
  printf("Run corner cases test --> ");
  print_test_result(res);
  free(key);
  key = NULL;
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
  RGENS_SEED();
  run_insert_search_free_uint32_test();
  run_remove_delete_uint32_test();
  run_insert_search_free_uint32_ptr_test();
  run_remove_delete_uint32_ptr_test();
  run_corner_cases_test();
  return 0;
}
