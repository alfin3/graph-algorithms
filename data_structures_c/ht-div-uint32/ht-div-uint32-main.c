/**
   ht-div-uint32-main.c

   Examples of of a hash table with generic hash keys and generic elements.
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
#include "utilities-ds.h"

void print_test_result(int result);

/**
   Run tests of a hash table with keys and elements, each within a 
   continuous memory block. A pointer to an element is passed as elt in 
   ht_div_uint32_insert and the element is fully copied into a block
   pointed from a node of the dll at the slot index computed by a hash 
   function, and a NULL as free_elt_fn is sufficient to delete the element.

   Search and insertion are tested across load factor upper bounds.
*/

int cmp_uint32_fn(void *a, void *b){
  return *(uint32_t *)a - *(uint32_t *)b;
}

int cmp_32_fn(void *a, void *b){
  return memcmp(a, b, 32);
}

int cmp_256_fn(void *a, void *b){
  return memcmp(a, b, 256);
}

static void insert_free_test_helper(uint32_t num_inserts,
				    float alphas[],
				    int num_alphas,
				    int key_size,
				    int (*cmp_key_fn)(void *, void *));
static void insert_free_alpha_test_helper(uint32_t num_inserts,
					  float alpha,
					  int key_size,
					  int (*cmp_key_fn)(void *, void *));

/**
   Runs a ht_div_uint32_{insert, free} test on distinct keys and 
   uint32_t elements.
*/
void run_insert_free_test(){
  uint32_t num_inserts = 1000000; //< 2^31
  float alphas[5] = {0.1, 1.0, 10.0, 100.0, 1000.0};
  int num_alphas = 5;
  int key_sizes[2] = {32, 256}; //>= sizeof(uint32_t) for this test
  int num_key_sizes = 2;
  int (*cmp_fn_arr[2])(void *, void *) = {cmp_32_fn, cmp_256_fn};
  printf("Run a ht_div_uint32_{insert, free} test on distinct %lu-byte "
	 "keys and uint32_t elements\n", sizeof(uint32_t));
  insert_free_test_helper(num_inserts,
			  alphas,
			  num_alphas,
			  sizeof(uint32_t),
			  cmp_uint32_fn);
  for (int i = 0; i < num_key_sizes; i++){
    printf("Run a ht_div_uint32_{insert, free} test on distinct %d-byte "
	   "keys and uint32_t elements\n", key_sizes[i]);
    insert_free_test_helper(num_inserts,
			    alphas,
			    num_alphas,
			    key_sizes[i],
			    cmp_fn_arr[i]);
  }
}

/** Helper functions for the ht_div_uint32_{insert, free} test */

static void insert_free_test_helper(uint32_t num_inserts,
				    float alphas[],
				    int num_alphas,
				    int key_size,
				    int (*cmp_key_fn)(void *, void *)){
  float alpha;
  for (int i = 0; i < num_alphas; i++){
    alpha = alphas[i];
    printf("\tnumber of inserts: %u, load factor upper bound: %.1f\n",
	   num_inserts, alpha);
    insert_free_alpha_test_helper(num_inserts, alpha, key_size, cmp_key_fn);
  }
}

static void insert_free_alpha_test_helper(uint32_t num_inserts,
					float alpha,
					int key_size,
					int (*cmp_key_fn)(void *, void *)){
  assert(key_size >= (int)sizeof(uint32_t));
  ht_div_uint32_t ht;
  void **key_arr, *key, *ptr;
  uint32_t *elt_arr, *elt;
  int result = 1;
  clock_t t;
  key_arr = malloc(num_inserts * sizeof(void *));
  assert(key_arr != NULL);
  elt_arr = malloc(num_inserts * sizeof(uint32_t));
  assert(elt_arr != NULL);
  for (uint32_t i = 0; i < num_inserts; i++){
    key_arr[i] = malloc(key_size);
    assert(key_arr[i] != NULL);
    ptr = (char *)(key_arr[i] + key_size - sizeof(uint32_t));
    *(uint32_t *)ptr = i;
    elt_arr[i] = i;
  }
  t = clock();
  ht_div_uint32_init(&ht,
                     key_size,
	             sizeof(uint32_t),
		     alpha,
                     cmp_key_fn,
		     NULL);
  for (uint32_t i = 0; i < num_inserts; i++){
    key = key_arr[i]; //dereference a pointer to void *
    elt = &(elt_arr[i]);
    ht_div_uint32_insert(&ht, key, elt);
  }
  t = clock() - t;
  printf("\t\tinsert time:           "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  result *= (ht.num_elts == num_inserts);
  t = clock();
  for (uint32_t i = 0; i < num_inserts; i++){
    key = key_arr[i];
    elt = ht_div_uint32_search(&ht, key);
    result *= (elt_arr[i] == *elt);
  }
  t = clock() - t;
  printf("\t\tin ht search time:     "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  result *= (ht.num_elts == num_inserts);
  t = clock();
  assert(num_inserts < (uint32_t)pow_two_uint64(31));
  for (uint32_t i = 0; i < num_inserts; i++){
    ptr = (char *)(key_arr[i] + key_size - sizeof(uint32_t));
    *(uint32_t *)ptr = i + num_inserts;
    key = key_arr[i];
    elt = ht_div_uint32_search(&ht, key);
    result *= (elt == NULL);
  }
  t = clock() - t;
  printf("\t\tnot in ht search time: "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  result *= (ht.num_elts == num_inserts);
  t = clock();
  ht_div_uint32_free(&ht);
  t = clock() - t;
  printf("\t\tfree time:             "
	 "%.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\tsearch correctness --> ");
  print_test_result(result);
  for (uint32_t i = 0; i < num_inserts; i++){
    free(key_arr[i]);
  }
  free(key_arr);
  free(elt_arr);
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
  int num_inserts = 100000;
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
  for (int i = 0; i < num_inserts; i++){
    elt = i;
    ht_div_uint32_insert(&ht, key, &elt);
  }
  result *= (ht_div_uint32_search(&ht, key) != NULL);
  result *= (ht.ht_size_ix == 0);
  result *= (ht.ht_size == ht_size);
  result *= (ht.num_elts == 1);
  result *= (*(uint32_t *)ht_div_uint32_search(&ht, key) == elt);
  ht_div_uint32_delete(&ht, key);
  result *= (ht_div_uint32_search(&ht, key) == NULL);
  printf("Run corner cases test --> ");
  print_test_result(result);
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
  run_insert_free_test();
  run_corner_cases_test();
  return 0;
}
