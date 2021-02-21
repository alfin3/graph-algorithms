/**
   ht-mul-uint64-main.c

   Tests of a hash table with generic hash keys and generic elements.
   The implementation is based on a multiplication method for hashing into 
   upto 2^63 slots (the upper range requiring > 2^64 addresses) and an 
   open addressing method for resolving collisions.
   
   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "ht-mul-uint64.h"
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

#define RGENS_SEED() do{srandom(time(0)); srand48(random());}while (0)
#define RANDOM() (random())
#define DRAND48() (drand48())

void insert_search_free_alphas(uint64_t num_inserts,
			       uint64_t key_size,
			       uint64_t elt_size,
			       float alphas[],
			       int alphas_count,
			       void (*rdc_key)(void *, const void *),
			       void (*new_elt)(void *, uint64_t),
			       uint64_t (*val_elt)(const void *),
			       void (*free_elt)(void *));
void remove_delete_alphas(uint64_t num_inserts,
			  uint64_t key_size,
			  uint64_t elt_size,
			  float alphas[],
			  int alphas_count,
			  void (*rdc_key)(void *, const void *),
			  void (*new_elt)(void *, uint64_t),
			  uint64_t (*val_elt)(const void *),
			  void (*free_elt)(void *));
void print_test_result(int res);

/**
   Test hash table operations on distinct keys and uint64_t elements across
   key sizes and load factor upper bounds. A pointer to an element is passed
   as elt in ht_mul_uint64_insert and the element is fully copied into the
   block pointed to from a node of the dll at the slot index computed by a
   hash function. NULL as free_elt is sufficient to delete the element.
*/

void rdc_32(void *t, const void *s){
  uint64_t r = 0;
  uint64_t n = 17858760364399553281U;
  for (int i = 0; i < 32; i += 8){
    r = sum_mod(r, *(uint64_t *)((char *)s + i), n);
  }
  *(uint64_t *)t = r;
}

void rdc_256(void *t, const void *s){
  uint64_t r = 0;
  uint64_t n = 17069408534778722687U;
  for (int i = 0; i < 256; i += 8){
    r = sum_mod(r, *(uint64_t *)((char *)s + i), n);
  }
  *(uint64_t *)t = r;
}


void new_uint64(void *elt, uint64_t val){
  uint64_t *s = elt;
  *s = val;
  s = NULL;
}

uint64_t val_uint64(const void *elt){
  return *(uint64_t *)elt;
}

/**
   Runs a ht_mul_uint64_{insert, search, free} test on distinct keys and 
   uint64_t elements across key sizes and load factor upper bounds.
*/
void run_insert_search_free_uint64_test(){
  int key_sizes_count = 3;
  int alphas_count = 4;
  uint64_t key_sizes[3] = {sizeof(uint64_t), 32, 256}; //>= sizeof(uint64_t)
  uint64_t num_inserts = 1000000; //< 2^63
  float alphas[4] = {0.1, 0.2, 0.4, 0.8};
  void (*rdc_arr[3])(void *, const void *) = {NULL, rdc_32, rdc_256};
  for (int i = 0; i < key_sizes_count; i++){
    printf("Run a ht_mul_uint64_{insert, search, free} test on distinct "
	   "%lu-byte keys and uint64_t elements\n", key_sizes[i]);
    insert_search_free_alphas(num_inserts,
			      key_sizes[i],
			      sizeof(uint64_t),
			      alphas,
			      alphas_count,
			      rdc_arr[i],
			      new_uint64,
			      val_uint64,
			      NULL);
  }
}

/**
   Runs a ht_mul_uint64_{remove, delete} test on distinct keys and uint64_t
   elements across key sizes and load factor upper bounds.
*/
void run_remove_delete_uint64_test(){
  int key_sizes_count = 3;
  int alphas_count = 4;
  uint64_t key_sizes[3] = {sizeof(uint64_t), 32, 256}; //>= sizeof(uint64_t)
  uint64_t num_inserts = 1000000; //< 2^63
  float alphas[4] = {0.1, 0.2, 0.4, 0.8};
  void (*rdc_arr[3])(void *, const void *) = {NULL, rdc_32, rdc_256};
  for (int i = 0; i < key_sizes_count; i++){
    printf("Run a ht_mul_uint64_{remove, delete} test on distinct "
	   "%lu-byte keys and uint64_t elements\n", key_sizes[i]);
    remove_delete_alphas(num_inserts,
			 key_sizes[i],
			 sizeof(uint64_t),
			 alphas,
			 alphas_count,
			 rdc_arr[i],
			 new_uint64,
			 val_uint64,
			 NULL);
  }
}

/**
   Test hash table operations on distinct keys and noncontiguous uint64_ptr_t
   elements across key sizes and load factor upper bounds. A pointer to a
   pointer to an element is passed as elt in ht_mul_uint64_insert, and
   the pointer to the element is copied into the block pointed to from a node
   of the dll at the slot index computed by a hash function. An element-
   specific free_elt is necessary to delete the element.
*/

typedef struct{
  uint64_t *val;
} uint64_ptr_t;

void new_uint64_ptr(void *elt, uint64_t val){
  uint64_ptr_t **s = elt;
  *s = malloc_perror(sizeof(uint64_ptr_t));
  (*s)->val = malloc_perror(sizeof(uint64_t));
  *((*s)->val) = val;
  s = NULL;
}

uint64_t val_uint64_ptr(const void *elt){
  uint64_ptr_t **s  = (uint64_ptr_t **)elt;
  return *((*s)->val);
}

void free_uint64_ptr(void *elt){
  uint64_ptr_t **s = elt;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

/**
   Runs a ht_mul_uint64_{insert, search, free} test on distinct keys and 
   noncontiguous uint64_ptr_t elements across key sizes and load factor 
   upper bounds.
*/
void run_insert_search_free_uint64_ptr_test(){
  int key_sizes_count = 3;
  int alphas_count = 4;
  uint64_t key_sizes[3] = {sizeof(uint64_t), 32, 256}; // >= sizeof(uint64_t)
  uint64_t num_inserts = 1000000; //< 2^63
  float alphas[4] = {0.1, 0.2, 0.4, 0.8};
  void (*rdc_arr[3])(void *, const void *) = {NULL, rdc_32, rdc_256};
  for (int i = 0; i < key_sizes_count; i++){
    printf("Run a ht_mul_uint64_{insert, search, free} test on distinct "
	   "%lu-byte keys and noncontiguous uint64_ptr_t elements\n",
	   key_sizes[i]);
    insert_search_free_alphas(num_inserts,
			      key_sizes[i],
			      sizeof(uint64_ptr_t *),
			      alphas,
			      alphas_count,
			      rdc_arr[i],
			      new_uint64_ptr,
			      val_uint64_ptr,
			      free_uint64_ptr);
  }
}

/**
   Runs a ht_mul_uint64_{remove, delete} test on distinct keys and 
   noncontiguous uint64_ptr_t elements across key sizes and load factor 
   upper bounds.
*/
void run_remove_delete_uint64_ptr_test(){
  int key_sizes_count = 3;
  int alphas_count = 4;
  uint64_t key_sizes[3] = {sizeof(uint64_t), 32, 256}; //>= sizeof(uint64_t)
  uint64_t num_inserts = 1000000; //< 2^63
  float alphas[4] = {0.1, 0.2, 0.4, 0.8};
  void (*rdc_arr[3])(void *, const void *) = {NULL, rdc_32, rdc_256};
  for (int i = 0; i < key_sizes_count; i++){
    printf("Run a ht_mul_uint64_{remove, delete} test on distinct "
	   "%lu-byte keys and noncontiguous uint64_ptr_t elements\n",
	   key_sizes[i]);
    remove_delete_alphas(num_inserts,
			 key_sizes[i],
			 sizeof(uint64_ptr_t *),
			 alphas,
			 alphas_count,
			 rdc_arr[i],
			 new_uint64_ptr,
			 val_uint64_ptr,
			 free_uint64_ptr);
  }
}

/** 
   Helper functions for the ht_mul_uint64_{insert, search, free} tests
   across key sizes and load factor upper bounds, on uint64_t and 
   uint64_ptr_t elements.
*/

void insert_keys_elts(ht_mul_uint64_t *ht,
		      void **keys,
		      void **elts,
		      uint64_t count,
		      int *res){
  uint64_t n = ht->num_elts;
  clock_t t;
  t = clock();
  for (uint64_t i = 0; i < count; i++){
    ht_mul_uint64_insert(ht, keys[i], elts[i]);
  }
  t = clock() - t;
  printf("\t\tinsert time:                    "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n + count);
}

void search_in_ht(const ht_mul_uint64_t *ht,
		  void **keys,
		  void **elts,
		  uint64_t count,
		  int *res,
		  uint64_t (*val_elt)(const void *)){
  uint64_t n = ht->num_elts;
  void *elt = NULL;
  clock_t t;
  t = clock();
  for (uint64_t i = 0; i < count; i++){
    elt = ht_mul_uint64_search(ht, keys[i]);
    *res *= (val_elt(elts[i]) == val_elt(elt));
  }
  t = clock() - t;
  printf("\t\tin ht search time:              "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void search_not_in_ht(const ht_mul_uint64_t *ht,
		      void **keys,
		      uint64_t count,
		      int *res){
  uint64_t n = ht->num_elts;
  void *elt = NULL;
  clock_t t;
  t = clock();
  for (uint64_t i = 0; i < count; i++){
    elt = ht_mul_uint64_search(ht, keys[i]);
    *res *= (elt == NULL);
  }
  t = clock() - t;
  printf("\t\tnot in ht search time:          "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *res *= (ht->num_elts == n);
}

void free_ht(ht_mul_uint64_t *ht){
  clock_t t;
  t = clock();
  ht_mul_uint64_free(ht);
  t = clock() - t;
  printf("\t\tfree time:                      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

void insert_search_free(uint64_t num_inserts,
			uint64_t key_size,
			uint64_t elt_size,
			float alpha,
			void (*rdc_key)(void *, const void *),
			void (*new_elt)(void *, uint64_t),
			uint64_t (*val_elt)(const void *),
			void (*free_elt)(void *)){
  int res = 1;
  void **keys = NULL, **elts = NULL, *ptr = NULL;
  ht_mul_uint64_t ht;
  keys = malloc_perror(num_inserts * sizeof(void *));
  elts = malloc_perror(num_inserts * sizeof(void *));
  for (uint64_t i = 0; i < num_inserts; i++){
    keys[i] = malloc_perror(key_size);
    ptr = (char *)(keys[i] + key_size - sizeof(uint64_t));
    *(uint64_t *)ptr = i;
    elts[i] = malloc_perror(elt_size);
    new_elt(elts[i], i);
  }
  ht_mul_uint64_init(&ht,
		     key_size,
		     elt_size,
		     alpha,
		     rdc_key,
		     free_elt);
  insert_keys_elts(&ht, keys, elts, num_inserts, &res);
  search_in_ht(&ht, keys, elts, num_inserts, &res, val_elt);
  for (uint64_t i = 0; i < num_inserts; i++){
    ptr = (char *)(keys[i] + key_size - sizeof(uint64_t));
    *(uint64_t *)ptr = i + num_inserts;
  }
  search_not_in_ht(&ht, keys, num_inserts, &res);
  free_ht(&ht);
  printf("\t\tsearch correctness:             ");
  print_test_result(res);
  for (uint64_t i = 0; i < num_inserts; i++){
    free(keys[i]);
    free(elts[i]);
  }
  free(keys);
  free(elts);
  keys = NULL;
  elts = NULL;
}

void insert_search_free_alphas(uint64_t num_inserts,
			       uint64_t key_size,
			       uint64_t elt_size,
			       float alphas[],
			       int alphas_count,
			       void (*rdc_key)(void *, const void *),
			       void (*new_elt)(void *, uint64_t),
			       uint64_t (*val_elt)(const void *),
			       void (*free_elt)(void *)){
  for (int i = 0; i < alphas_count; i++){
    printf("\tnumber of inserts: %lu, load factor upper bound: %.1f\n",
	   num_inserts, alphas[i]);
    insert_search_free(num_inserts,
		       key_size,
		       elt_size,
		       alphas[i],
		       rdc_key,
		       new_elt,
		       val_elt,
		       free_elt);
  }
}

/** 
   Helper functions for the ht_mul_uint64_{remove, delete} tests
   across key sizes and load factor upper bounds, on uint64_t and 
   uint64_ptr_t elements.
*/

void remove_key_elts(ht_mul_uint64_t *ht,
		     void **keys,
		     void **elts,
		     uint64_t count,
		     int *res,
		     uint64_t (*val_elt)(const void *)){
  uint64_t n = ht->num_elts;
  uint64_t c = 0;
  void *ptr = NULL, *elt = NULL;
  clock_t t;
  elt = malloc_perror(ht->elt_size);
  t = clock();
  for (uint64_t i = 0; i < count; i += 2){
    ht_mul_uint64_remove(ht, keys[i], elt);
    *res *= (val_elt(elts[i]) == val_elt(elt));
    //if an element is noncontiguous, it is still accessible from elts[i]
    c++;
  }
  t = clock() - t;
  *res *= (ht->num_elts == n - c);
  printf("\t\tremove 1/2 elements time:       "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint64_t i = 0; i < count; i++){
    if (i % 2){
      ptr = ht_mul_uint64_search(ht, keys[i]);
      *res *= (val_elt(elts[i]) == val_elt(ptr));
    }else{
      *res *= (ht_mul_uint64_search(ht, keys[i]) == NULL);
    }
  }
  t = clock();
  for (uint64_t i = 1; i < count; i += 2){
    ht_mul_uint64_remove(ht, keys[i], elt);
    *res *= (val_elt(elts[i]) == val_elt(elt));
    //if an element is noncontiguous, it is still accessible from elts[i]
  }
  t = clock() - t;
  *res *= (ht->num_elts == 0);
  printf("\t\tremove residual elements time:  "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint64_t i = 0; i < count; i++){
    *res *= (ht_mul_uint64_search(ht, keys[i]) == NULL);
  }
  free(elt);
  elt = NULL;
}

void delete_key_elts(ht_mul_uint64_t *ht,
		     void **keys,
		     void **elts,
		     uint64_t count,
		     int *res,
		     uint64_t (*val_elt)(const void *)){
  uint64_t n = ht->num_elts;
  uint64_t c = 0;
  void *ptr;
  clock_t t;
  t = clock();
  for (uint64_t i = 0; i < count; i += 2){
    ht_mul_uint64_delete(ht, keys[i]);
    c++;
  }
  t = clock() - t;
  *res *= (ht->num_elts == n - c);
  printf("\t\tdelete 1/2 elements time:       "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint64_t i = 0; i < count; i++){
    if (i % 2){
      ptr = ht_mul_uint64_search(ht, keys[i]);
      *res *= (val_elt(elts[i]) == val_elt(ptr));
    }else{
      *res *= (ht_mul_uint64_search(ht, keys[i]) == NULL);
    }
  }
  t = clock();
  for (uint64_t i = 1; i < count; i += 2){
    ht_mul_uint64_delete(ht, keys[i]);
  }
  t = clock() - t;
  *res *= (ht->num_elts == 0);
  printf("\t\tdelete residual elements time:  "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint64_t i = 0; i < count; i++){
    *res *= (ht_mul_uint64_search(ht, keys[i]) == NULL);
  }
}

void remove_delete(uint64_t num_inserts,
		   uint64_t key_size,
		   uint64_t elt_size,
		   float alpha,
		   void (*rdc_key)(void *, const void *),
		   void (*new_elt)(void *, uint64_t),
		   uint64_t (*val_elt)(const void *),
		   void (*free_elt)(void *)){
  int res = 1;
  void **keys = NULL, **elts = NULL, *ptr = NULL;
  ht_mul_uint64_t ht;
  keys = malloc_perror(num_inserts * sizeof(void *));
  elts = malloc_perror(num_inserts * sizeof(void *));
  for (uint64_t i = 0; i < num_inserts; i++){
    keys[i] = malloc_perror(key_size);
    ptr = (char *)(keys[i] + key_size - sizeof(uint64_t));
    *(uint64_t *)ptr = i;
    elts[i] = malloc_perror(elt_size);
    new_elt(elts[i], i);
  }
  ht_mul_uint64_init(&ht,
		     key_size,
		     elt_size,
		     alpha,
		     rdc_key,
		     free_elt);
  insert_keys_elts(&ht, keys, elts, num_inserts, &res);
  remove_key_elts(&ht, keys, elts, num_inserts, &res, val_elt);
  insert_keys_elts(&ht, keys, elts, num_inserts, &res);
  delete_key_elts(&ht, keys, elts, num_inserts, &res, val_elt);
  free_ht(&ht);
  printf("\t\tremove and delete correctness:  ");
  print_test_result(res);
  for (uint64_t i = 0; i < num_inserts; i++){
    free(keys[i]);
    free(elts[i]);
  }
  free(keys);
  free(elts);
  keys = NULL;
  elts = NULL;
}

void remove_delete_alphas(uint64_t num_inserts,
			  uint64_t key_size,
			  uint64_t elt_size,
			  float alphas[],
			  int alphas_count,
			  void (*rdc_key)(void *, const void *),
			  void (*new_elt)(void *, uint64_t),
			  uint64_t (*val_elt)(const void *),
			  void (*free_elt)(void *)){
  for (int i = 0; i < alphas_count; i++){
    printf("\tnumber of inserts: %lu, load factor upper bound: %.1f\n",
	   num_inserts, alphas[i]);
    remove_delete(num_inserts,
		  key_size,
		  elt_size,
		  alphas[i],
		  rdc_key,
		  new_elt,
		  val_elt,
		  free_elt);
  }
}

/**
   Run a corner cases test.
*/

void run_corner_cases_test(){
  int res = 1;
  uint8_t key_a, key_b;
  uint64_t elt;
  uint64_t num_inserts = 100;
  uint64_t ht_count = pow_two(10);
  float alpha = 0.001;
  ht_mul_uint64_t ht;
  ht_mul_uint64_init(&ht,
                     sizeof(uint8_t),
	             sizeof(uint64_t),
		     alpha,
		     NULL,
		     NULL);
  key_a = DRAND48() * 0xff;
  key_b = DRAND48() * 0xff;
  while (key_b == key_a){
    key_b = DRAND48() * 0xff;
  }
  for (uint64_t i = 0; i < num_inserts; i++){
    elt = i;
    ht_mul_uint64_insert(&ht, &key_a, &elt);
  }
  res *= (ht.count == ht_count);
  res *= (ht.num_elts == 1);
  res *= (*(uint64_t *)ht_mul_uint64_search(&ht, &key_a) == elt);
  res *= (ht_mul_uint64_search(&ht, &key_b) == NULL);
  ht_mul_uint64_insert(&ht, &key_b, &elt);
  res *= (ht.count == ht_count);
  res *= (ht.num_elts == 2);
  res *= (*(uint64_t *)ht_mul_uint64_search(&ht, &key_a) == elt);
  res *= (*(uint64_t *)ht_mul_uint64_search(&ht, &key_b) == elt);
  ht_mul_uint64_delete(&ht, &key_a);
  res *= (ht.count == ht_count);
  res *= (ht.num_elts == 1);
  res *= (ht_mul_uint64_search(&ht, &key_a) == NULL);
  res *= (*(uint64_t *)ht_mul_uint64_search(&ht, &key_b) == elt);
  ht_mul_uint64_delete(&ht, &key_b);
  res *= (ht.count == ht_count);
  res *= (ht.num_elts == 0);
  res *= (ht_mul_uint64_search(&ht, &key_a) == NULL);
  res *= (ht_mul_uint64_search(&ht, &key_b) == NULL);
  printf("Run corner cases test --> ");
  print_test_result(res);
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
  run_insert_search_free_uint64_test();
  run_remove_delete_uint64_test();
  run_insert_search_free_uint64_ptr_test();
  run_remove_delete_uint64_ptr_test();
  run_corner_cases_test();
  return 0;
}
