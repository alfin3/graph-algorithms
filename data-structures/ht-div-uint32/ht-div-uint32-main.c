/**
   ht-div-uint32-main.c

   Examples of a hash table with generic hash keys and generic elements.
   The implementation is based on a division method for hashing into 2^32-1 
   slots and a chaining method for resolving collisions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "ht-div-uint32.h"
#include "dll.h"
#include "utilities-rand-mod.h"

void print_test_result(int result);
static void insert_search_free_alphas(uint32_t num_inserts,
				      float alphas[],
				      int num_alphas,
				      int key_size,
				      int elt_size,
				      int (*cmp_key_fn)(void *, void *),
				      void (*cstr_elt_fn)(void *, uint32_t),
				      uint32_t (*elt_val_fn)(void *),
				      void (*free_elt_fn)(void *));
static void remove_delete_alphas(uint32_t num_inserts,
				 float alphas[],
				 int num_alphas,
				 int key_size,
				 int elt_size,
				 int (*cmp_key_fn)(void *, void *),
				 void (*cstr_elt_fn)(void *, uint32_t),
				 uint32_t (*elt_val_fn)(void *),
				 void (*free_elt_fn)(void *));

/**
   Run tests of a hash table on distinct keys and uint32_t elements across 
   key sizes and load factor upper bounds. A pointer to an element is passed 
   as elt in ht_div_uint32_insert and the element is fully copied into a block
   pointed from a node of the dll at the slot index computed by a hash 
   function. A NULL as free_elt_fn is sufficient to delete the element.
*/

int cmp_uint32_fn(void *a, void *b){
  return memcmp(a, b, sizeof(uint32_t));
}

int cmp_32_fn(void *a, void *b){
  return memcmp(a, b, 32);
}

int cmp_256_fn(void *a, void *b){
  return memcmp(a, b, 256);
}

/**
   Constructs an uint32_t element. The elt parameter is a pointer that
   points to a preallocated block of size elt_size, i.e. sizeof(uint32_t).
*/
void cstr_uint32_fn(void *elt, uint32_t val){
  uint32_t *s = elt;
  *s = val;
  s = NULL;
}

uint32_t val_uint32_fn(void *elt){
  return *(uint32_t *)elt;
}

/**
   Runs a ht_div_uint32_{insert, search, free} test on distinct keys and 
   uint32_t elements across key sizes and load factor upper bounds.
*/
void run_insert_search_free_uint32_elt_test(){
  uint32_t num_inserts = 1000000; //< 2^31 for this test
  float alphas[4] = {0.1, 1.0, 10.0, 100.0};
  int num_alphas = 4;
  //key_size >= sizeof(uint32_t) for this test
  int key_sizes[3] = {sizeof(uint32_t), 32, 256}; 
  int num_key_sizes = 3;
  int (*cmp_fn_arr[3])(void *, void *) = {cmp_uint32_fn,
					  cmp_32_fn,
					  cmp_256_fn};
  for (int i = 0; i < num_key_sizes; i++){
    printf("Run a ht_div_uint32_{insert, search, free} test on distinct "
	   "%d-byte keys and uint32_t elements\n", key_sizes[i]);
    insert_search_free_alphas(num_inserts,
			      alphas,
			      num_alphas,
			      key_sizes[i],
			      sizeof(uint32_t),
			      cmp_fn_arr[i],
			      cstr_uint32_fn,
			      val_uint32_fn,
			      NULL);
  }
}

/**
   Runs a ht_div_uint32_{remove, delete} test on distinct keys and 
   uint32_t elements across key sizes and load factor upper bounds.
*/
void run_remove_delete_uint32_elt_test(){
  uint32_t num_inserts = 1000000; //< 2^31 for this test
  float alphas[4] = {0.1, 1.0, 10.0, 100.0};
  int num_alphas = 4;
  //key_size >= sizeof(uint32_t) for this test
  int key_sizes[3] = {sizeof(uint32_t), 32, 256}; 
  int num_key_sizes = 3;
  int (*cmp_fn_arr[3])(void *, void *) = {cmp_uint32_fn,
					  cmp_32_fn,
					  cmp_256_fn};
  for (int i = 0; i < num_key_sizes; i++){
    printf("Run a ht_div_uint32_{remove, delete} test on distinct "
	   "%d-byte keys and uint32_t elements\n", key_sizes[i]);
    remove_delete_alphas(num_inserts,
			 alphas,
			 num_alphas,
			 key_sizes[i],
			 sizeof(uint32_t),
			 cmp_fn_arr[i],
			 cstr_uint32_fn,
			 val_uint32_fn,
			 NULL);
  }
}

/**
   Run tests of a hash table on distinct keys and multilayered uint32_ptr_t 
   elements across key sizes and load factor upper bounds. A pointer to a 
   pointer to an element is passed as elt in ht_div_uint32_insert, and
   the pointer to the element is copied into a block pointed from a node of 
   the dll at the slot index computed by a hash function. An element-specific
   free_elt_fn is necessary to delete the element.
*/

typedef struct{
  uint32_t *val;
} uint32_ptr_t;

/**
   Constructs an uint32_ptr_t element. The elt parameter is a pointer that
   points to a preallocated block of size elt_size, i.e. 
   sizeof(uint32_ptr_t *).
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

uint32_t val_uint32_ptr_fn(void *elt){
  uint32_ptr_t **s  = elt;
  return *((*s)->val);
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
   Run a ht_div_uint32_{insert, search, free} test on distinct keys and 
   multilayered uint32_ptr_t elements across key sizes and load factor 
   upper bounds.
*/
void run_insert_search_free_uint32_ptr_elt_test(){
  uint32_t num_inserts = 1000000; //< 2^31 for this test
  float alphas[4] = {0.1, 1.0, 10.0, 100.0};
  int num_alphas = 4;
  //key_size >= sizeof(uint32_t) for this test
  int key_sizes[3] = {sizeof(uint32_t), 32, 256}; 
  int num_key_sizes = 3;
  int (*cmp_fn_arr[3])(void *, void *) = {cmp_uint32_fn,
					  cmp_32_fn,
					  cmp_256_fn};
  for (int i = 0; i < num_key_sizes; i++){
    printf("Run a ht_div_uint32_{insert, search, free} test on distinct "
	   "%d-byte keys and multilayered uint32_ptr_t elements\n",
	   key_sizes[i]);
    insert_search_free_alphas(num_inserts,
			      alphas,
			      num_alphas,
			      key_sizes[i],
			      sizeof(uint32_ptr_t *),
			      cmp_fn_arr[i],
			      cstr_uint32_ptr_fn,
			      val_uint32_ptr_fn,
			      free_uint32_ptr_fn);
  }
}

/**
   Runs a ht_div_uint32_{remove, delete} test on distinct keys and 
   multilayered uint32_ptr_t elements across key sizes and load factor 
   upper bounds.
*/
void run_remove_delete_uint32_ptr_elt_test(){
  uint32_t num_inserts = 1000000; //< 2^31 for this test
  float alphas[4] = {0.1, 1.0, 10.0, 100.0};
  int num_alphas = 4;
  //key_size >= sizeof(uint32_t) for this test
  int key_sizes[3] = {sizeof(uint32_t), 32, 256}; 
  int num_key_sizes = 3;
  int (*cmp_fn_arr[3])(void *, void *) = {cmp_uint32_fn,
					  cmp_32_fn,
					  cmp_256_fn};
  for (int i = 0; i < num_key_sizes; i++){
    printf("Run a ht_div_uint32_{remove, delete} test on distinct "
	   "%d-byte keys and multilayered uint32_ptr_t elements\n",
	   key_sizes[i]);
    remove_delete_alphas(num_inserts,
			 alphas,
			 num_alphas,
			 key_sizes[i],
			 sizeof(uint32_ptr_t *),
			 cmp_fn_arr[i],
			 cstr_uint32_ptr_fn,
			 val_uint32_ptr_fn,
			 free_uint32_ptr_fn);
  }
}

/** 
   Helper functions for the ht_div_uint32_{insert, search, free} tests
   across key sizes and load factor upper bounds, on uint32_t and 
   uint32_ptr_t elements.
*/

static void insert_keys_elts(ht_div_uint32_t *ht,
			     void **key_arr,
			     void **elt_arr,
			     uint32_t arr_size,
			     int *result){
  uint32_t n = ht->num_elts;
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < arr_size; i++){
    ht_div_uint32_insert(ht, key_arr[i], elt_arr[i]);
  }
  t = clock() - t;
  printf("\t\tinsert time:                    "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (ht->num_elts == n + arr_size);
}

static void search_in_ht(ht_div_uint32_t *ht,
			 void **key_arr,
			 void **elt_arr,
			 uint32_t arr_size,
			 int *result,
			 uint32_t (*elt_val_fn)(void *)){
  uint32_t n = ht->num_elts;
  void *elt;
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < arr_size; i++){
    elt = ht_div_uint32_search(ht, key_arr[i]);
    *result *= (elt_val_fn(elt_arr[i]) == elt_val_fn(elt));
  }
  t = clock() - t;
  printf("\t\tin ht search time:              "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (ht->num_elts == n);
}

static void search_not_in_ht(ht_div_uint32_t *ht,
			     void **key_arr,
			     uint32_t arr_size,
			     int *result){
  uint32_t n = ht->num_elts;
  void *elt;
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < arr_size; i++){
    elt = ht_div_uint32_search(ht, key_arr[i]);
    *result *= (elt == NULL);
  }
  t = clock() - t;
  printf("\t\tnot in ht search time:          "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  *result *= (ht->num_elts == n);
}

static void free_ht(ht_div_uint32_t *ht){
  clock_t t;
  t = clock();
  ht_div_uint32_free(ht);
  t = clock() - t;
  printf("\t\tfree time:                      "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

static void insert_search_free(uint32_t num_inserts,
			       float alpha,
			       int key_size,
			       int elt_size,
			       int (*cmp_key_fn)(void *, void *),
			       void (*cstr_elt_fn)(void *, uint32_t),
			       uint32_t (*elt_val_fn)(void *),
			       void (*free_elt_fn)(void *)){
  assert(key_size >= (int)sizeof(uint32_t));
  ht_div_uint32_t ht;
  void **key_arr, **elt_arr, *ptr;
  int result = 1;
  //construct keys and elements to avoid construction in timing exps.
  key_arr = malloc(num_inserts * sizeof(void *));
  assert(key_arr != NULL);
  elt_arr = malloc(num_inserts * sizeof(void *));
  assert(elt_arr != NULL);
  for (uint32_t i = 0; i < num_inserts; i++){
    key_arr[i] = malloc(key_size);
    assert(key_arr[i] != NULL);
    ptr = (char *)(key_arr[i] + key_size - sizeof(uint32_t));
    *(uint32_t *)ptr = i;
    elt_arr[i] = malloc(elt_size);
    assert(elt_arr[i] != NULL);
    cstr_elt_fn(elt_arr[i], i);
  }
  ht_div_uint32_init(&ht,
                     key_size,
	             elt_size,
		     alpha,
                     cmp_key_fn,
		     free_elt_fn);
  insert_keys_elts(&ht, key_arr, elt_arr, num_inserts, &result);
  search_in_ht(&ht, key_arr, elt_arr, num_inserts, &result, elt_val_fn);
  assert(num_inserts < (uint32_t)pow_two_uint64(31));
  for (uint32_t i = 0; i < num_inserts; i++){
    ptr = (char *)(key_arr[i] + key_size - sizeof(uint32_t));
    *(uint32_t *)ptr = i + num_inserts;
  }
  search_not_in_ht(&ht, key_arr, num_inserts, &result);
  free_ht(&ht);
  printf("\t\tsearch correctness:             ");
  print_test_result(result);
  //free key_arr and elt_arr
  for (uint32_t i = 0; i < num_inserts; i++){
    free(key_arr[i]);
    free(elt_arr[i]);
  }
  free(key_arr);
  free(elt_arr);
}

static void insert_search_free_alphas(uint32_t num_inserts,
				      float alphas[],
				      int num_alphas,
				      int key_size,
				      int elt_size,
				      int (*cmp_key_fn)(void *, void *),
				      void (*cstr_elt_fn)(void *, uint32_t),
				      uint32_t (*elt_val_fn)(void *),
				      void (*free_elt_fn)(void *)){
  float alpha;
  for (int i = 0; i < num_alphas; i++){
    alpha = alphas[i];
    printf("\tnumber of inserts: %u, load factor upper bound: %.1f\n",
	   num_inserts, alpha);
    insert_search_free(num_inserts,
		       alpha,
		       key_size,
		       elt_size,
		       cmp_key_fn,
		       cstr_elt_fn,
		       elt_val_fn,
		       free_elt_fn);
  }
}

/** 
   Helper functions for the ht_div_uint32_{remove, delete} tests
   across key sizes and load factor upper bounds, on uint32_t and 
   uint32_ptr_t elements.
*/

static void remove_key_elts(ht_div_uint32_t *ht,
			    void **key_arr,
			    void **elt_arr,
			    uint32_t arr_size,
			    int *result,
			    uint32_t (*elt_val_fn)(void *)){
  uint32_t n = ht->num_elts;
  uint32_t c = 0;
  void *ptr;
  void *elt = malloc(ht->elt_size);
  assert(elt != NULL);
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < arr_size; i += 2){
    ht_div_uint32_remove(ht, key_arr[i], elt);
    *result *= (elt_val_fn(elt_arr[i]) == elt_val_fn(elt));
    //if an element is multilayered, it is still accessible from elt_arr[i]
    c++;
  }
  t = clock() - t;
  *result *= (ht->num_elts == n - c);
  printf("\t\tremove 1/2 elements time:       "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint32_t i = 0; i < arr_size; i++){
    if (i % 2){
      ptr = ht_div_uint32_search(ht, key_arr[i]);
      *result *= (elt_val_fn(elt_arr[i]) == elt_val_fn(ptr));
    }else{
      *result *= (ht_div_uint32_search(ht, key_arr[i]) == NULL);
    }
  }
  t = clock();
  for (uint32_t i = 1; i < arr_size; i += 2){
    ht_div_uint32_remove(ht, key_arr[i], elt);
    *result *= (elt_val_fn(elt_arr[i]) == elt_val_fn(elt));
    //if an element is multilayered, it is still accessible from elt_arr[i]
  }
  t = clock() - t;
  *result *= (ht->num_elts == 0);
  printf("\t\tremove residual elements time:  "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint32_t i = 0; i < arr_size; i++){
    *result *= (ht_div_uint32_search(ht, key_arr[i]) == NULL);
  }
  for (uint32_t i = 0; i < ht->ht_size; i++){
    *result *= (ht->key_elts[i] == NULL);
  }
  free(elt);
  elt = NULL;
}

static void delete_key_elts(ht_div_uint32_t *ht,
			    void **key_arr,
			    void **elt_arr,
			    uint32_t arr_size,
			    int *result,
			    uint32_t (*elt_val_fn)(void *)){
  uint32_t n = ht->num_elts;
  uint32_t c = 0;
  void *ptr;
  clock_t t;
  t = clock();
  for (uint32_t i = 0; i < arr_size; i += 2){
    ht_div_uint32_delete(ht, key_arr[i]);
    c++;
  }
  t = clock() - t;
  *result *= (ht->num_elts == n - c);
  printf("\t\tdelete 1/2 elements time:       "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint32_t i = 0; i < arr_size; i++){
    if (i % 2){
      ptr = ht_div_uint32_search(ht, key_arr[i]);
      *result *= (elt_val_fn(elt_arr[i]) == elt_val_fn(ptr));
    }else{
      *result *= (ht_div_uint32_search(ht, key_arr[i]) == NULL);
    }
  }
  t = clock();
  for (uint32_t i = 1; i < arr_size; i += 2){
    ht_div_uint32_delete(ht, key_arr[i]);
  }
  t = clock() - t;
  *result *= (ht->num_elts == 0);
  printf("\t\tdelete residual elements time:  "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  for (uint32_t i = 0; i < arr_size; i++){
    *result *= (ht_div_uint32_search(ht, key_arr[i]) == NULL);
  }
  for (uint32_t i = 0; i < ht->ht_size; i++){
    *result *= (ht->key_elts[i] == NULL);
  }
}

static void remove_delete(uint32_t num_inserts,
			  float alpha,
			  int key_size,
			  int elt_size,
			  int (*cmp_key_fn)(void *, void *),
			  void (*cstr_elt_fn)(void *, uint32_t),
			  uint32_t (*elt_val_fn)(void *),
			  void (*free_elt_fn)(void *)){
  assert(key_size >= (int)sizeof(uint32_t));
  ht_div_uint32_t ht;
  void **key_arr, **elt_arr, *ptr;
  int result = 1;
  //construct keys and elements to avoid construction in timing exps.
  key_arr = malloc(num_inserts * sizeof(void *));
  assert(key_arr != NULL);
  elt_arr = malloc(num_inserts * sizeof(void *));
  assert(elt_arr != NULL);
  for (uint32_t i = 0; i < num_inserts; i++){
    key_arr[i] = malloc(key_size);
    assert(key_arr[i] != NULL);
    ptr = (char *)(key_arr[i] + key_size - sizeof(uint32_t));
    *(uint32_t *)ptr = i;
    elt_arr[i] = malloc(elt_size);
    assert(elt_arr[i] != NULL);
    cstr_elt_fn(elt_arr[i], i);
  }
  ht_div_uint32_init(&ht,
                     key_size,
	             elt_size,
		     alpha,
                     cmp_key_fn,
		     free_elt_fn);
  insert_keys_elts(&ht, key_arr, elt_arr, num_inserts, &result);
  remove_key_elts(&ht, key_arr, elt_arr, num_inserts, &result, elt_val_fn);
  insert_keys_elts(&ht, key_arr, elt_arr, num_inserts, &result);
  delete_key_elts(&ht, key_arr, elt_arr, num_inserts, &result, elt_val_fn);
  free_ht(&ht);
  printf("\t\tremove and delete correctness:  ");
  print_test_result(result);
  //free key_arr and elt_arr
  for (uint32_t i = 0; i < num_inserts; i++){
    free(key_arr[i]);
    free(elt_arr[i]);
  }
  free(key_arr);
  free(elt_arr);
}

static void remove_delete_alphas(uint32_t num_inserts,
				 float alphas[],
				 int num_alphas,
				 int key_size,
				 int elt_size,
				 int (*cmp_key_fn)(void *, void *),
				 void (*cstr_elt_fn)(void *, uint32_t),
				 uint32_t (*elt_val_fn)(void *),
				 void (*free_elt_fn)(void *)){
  float alpha;
  for (int i = 0; i < num_alphas; i++){
    alpha = alphas[i];
    printf("\tnumber of inserts: %u, load factor upper bound: %.1f\n",
	   num_inserts, alpha);
    remove_delete(num_inserts,
		  alpha,
		  key_size,
		  elt_size,
		  cmp_key_fn,
		  cstr_elt_fn,
		  elt_val_fn,
		  free_elt_fn);
  }
}

/**
   Run a corner cases test.
*/
void run_corner_cases_test(){
  ht_div_uint32_t ht;
  void *key;
  uint32_t elt;
  float alpha = 0.001;
  int key_size = 256;
  uint32_t num_inserts = 100000;
  uint32_t ht_size = 1543;
  int result = 1;
  ht_div_uint32_init(&ht,
                     key_size,
	             sizeof(uint32_t),
		     alpha,
                     cmp_256_fn,
		     NULL);
  key = malloc(key_size);
  assert(key != NULL);
  for (int i = 0; i < key_size; i++){
    *((uint8_t *)key + i) = random_range_uint32(pow_two_uint64(8) - 1);
  }
  for (uint32_t i = 0; i < num_inserts; i++){
    elt = i;
    ht_div_uint32_insert(&ht, key, &elt);
  }
  result *= (ht.ht_size_ix == 0);
  result *= (ht.ht_size == ht_size);
  result *= (ht.num_elts == 1);
  result *= (*(uint32_t *)ht_div_uint32_search(&ht, key) == elt);
  ht_div_uint32_delete(&ht, key);
  result *= (ht.ht_size == ht_size);
  result *= (ht.num_elts == 0);
  result *= (ht_div_uint32_search(&ht, key) == NULL);
  printf("Run corner cases test --> ");
  print_test_result(result);
  free(key);
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
  run_insert_search_free_uint32_elt_test();
  run_remove_delete_uint32_elt_test();
  run_insert_search_free_uint32_ptr_elt_test();
  run_remove_delete_uint32_ptr_elt_test();
  run_corner_cases_test();
  return 0;
}
