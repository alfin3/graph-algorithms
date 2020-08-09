/**
   ht-mul-uint64.c

   A hash table with generic hash keys and generic elements. The 
   implementation is based on a multiplication method for hashing into upto 
   2^63 slots (the upper range requiring > 2^64 addresses) and an open 
   addressing method (with linear probing at this time) for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by 
   the alpha parameter. The expected number of probes in a search is 
   upper-bounded by 1/(1 - alpha), under the uniform hashing assumption. 

   The alpha parameter does not provide an upper bound after the maximum size 
   of a hash table is reached. Due to open addressing, the load factor 
   is < 1.0 even after the alpha parameter is exceeded.

   A hash key is an object within a continuous block of memory (e.g. a basic 
   type, array, struct). If the key size is <= 8 bytes, then the key size 
   reduction function rdc_key_fn is NULL. Otherwise, rdc_key_fn is not NULL 
   and reduces a key to a 8-byte block prior to hashing. Key size reduction 
   methods may introduce regularities prior to hashing.

   An element is an object within a continuous block of memory (e.g. a basic 
   type, array, struct), or a multilayered object in memory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "ht-mul-uint64.h"
#include "dll.h"
#include "utilities-ds.h"

static const uint64_t prime = 15769474759331449193U; //2^63 < prime < 2^64
static uint64_t hash(ht_mul_uint64_t *ht, uint64_t key);
static uint64_t probe_linear(ht_mul_uint64_t *ht, uint64_t ix);
static uint64_t convert_std_key(ht_mul_uint64_t *ht, void *key);
static void ht_grow(ht_mul_uint64_t *ht);

/**
   Initializes a hash table. 
   ht: a pointer to a previously created ht_mul_uint64_t instance.
   alpha: 0.0 < alpha < 1.0, a load factor upper bound.
   cmp_key_fn: 0 if two keys are equal.
   rdc_key_fn: the first parameter points to an 8-byte block, where the 
               reduced form of the second parameter is copied.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in 
                ht_mul_uint64_insert, then the element is fully copied into 
                a block pointed from the node at the slot index computed by a 
                hash function, and a NULL as free_elt_fn is sufficient to 
                delete the element;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in ht_mul_uint64_insert, then
                the pointer to the element is copied into a block pointed 
                from the node at the slot index computed by a hash function, 
                and an element-specific free_elt_fn is necessary to delete 
                the element.
*/
void ht_mul_uint64_init(ht_mul_uint64_t *ht,
                        int key_size,
	                int elt_size,
			float alpha,
                        int (*cmp_key_fn)(void *, void *),
                        void (*rdc_key_fn)(void *, void *),
	                void (*free_elt_fn)(void *)){
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->ht_log_size = 10;
  ht->ht_size = pow_two_uint64(ht->ht_log_size); //a power of two
  ht->max_ht_size = pow_two_uint64(63);
  ht->max_num_probes = 1; //at least one probe
  ht->num_elts = 0;
  ht->alpha = alpha;
  ht->key_elts = malloc(ht->ht_size * sizeof(dll_node_t *));
  assert(ht->key_elts != NULL);
  for (uint64_t i = 0; i < ht->ht_size; i++){
    dll_init(&(ht->key_elts[i]));
  }
  ht->cmp_key_fn = cmp_key_fn;
  ht->rdc_key_fn = rdc_key_fn;
  ht->free_elt_fn = free_elt_fn;
}

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL.
*/
void ht_mul_uint64_insert(ht_mul_uint64_t *ht, void *key, void *elt){
  //grow ht if E[# keys in a slot] > alpha
  if ((float)(ht->num_elts) / ht->ht_size > ht->alpha){ht_grow(ht);}
  dll_node_t **head, *node;
  uint64_t std_key = convert_std_key(ht, key);
  uint64_t num_probes = 1;
  uint64_t ix = hash(ht, std_key);
  head = &(ht->key_elts[ix]);
  while (*head != NULL){
    //search over a dll with a single node
    node = dll_search_key(head, key, ht->cmp_key_fn);
    if (node != NULL){
      dll_delete(head, node, ht->free_elt_fn);
      dll_insert(head, key, elt, ht->key_size, ht->elt_size);
      return;
    }
    ix = probe_linear(ht, ix);
    head = &(ht->key_elts[ix]);
    num_probes++;
    if (num_probes > ht->max_num_probes){
      ht->max_num_probes = num_probes;
    }
  }
  dll_insert(head, key, elt, ht->key_size, ht->elt_size);
  ht->num_elts++;
}

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_mul_uint64_search(ht_mul_uint64_t *ht, void *key){
  dll_node_t **head, *node;
  uint64_t std_key = convert_std_key(ht, key);
  uint64_t num_probes = 1;
  uint64_t ix = hash(ht, std_key);
  head = &(ht->key_elts[ix]);
  while (*head != NULL){
    //search over a dll with a single node
    node = dll_search_key(head, key, ht->cmp_key_fn);
    if (node != NULL){
      return node->elt;
    }else if (num_probes == ht->max_num_probes){
      break;
    }else{
      ix = probe_linear(ht, ix);
      head = &(ht->key_elts[ix]);
      num_probes++;
    }
  }
  return NULL;
}

/**
   Frees a hash table.
*/
void ht_mul_uint64_free(ht_mul_uint64_t *ht){
  for (uint64_t i = 0; i < ht->ht_size; i++){
    dll_free(&(ht->key_elts[i]), ht->free_elt_fn);
  }
  free(ht->key_elts);
  ht->key_elts = NULL;
}

/** Helper functions */

/**
   Converts a key to a key of standard size of 8 bytes.
*/
static uint64_t convert_std_key(ht_mul_uint64_t *ht, void *key){
  uint64_t std_key;
  if (ht->key_size > (int)sizeof(uint64_t)){
    ht->rdc_key_fn(&std_key, key);
  }else{ 
    memcpy(&std_key, key, ht->key_size);
  }
  return std_key;
}

/**
   Maps a hash key to a slot index in a hash table with a multiplication method. 
*/
static uint64_t hash(ht_mul_uint64_t *ht, uint64_t key){
  return mul_mod_pow_two_64(prime, key) >> (64 - ht->ht_log_size);
}

/**
   Returns the next index in a hash table, based on linear probing.
*/
static uint64_t probe_linear(ht_mul_uint64_t *ht, uint64_t ix){
  return sum_mod_uint64(1, ix, ht->ht_size);
}

/**
   Doubles the size of a hash table. Makes no changes if the maximum size
   has been reached.
*/
static void ht_grow(ht_mul_uint64_t *ht){
  //if the maximum size is reached, alpha is not a bound for expectation
  if (ht->ht_size == ht->max_ht_size){return;} 
  uint64_t prev_ht_size = ht->ht_size;
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head;
  ht->ht_log_size++;
  ht->ht_size *= 2;
  ht->max_num_probes = 1;
  ht->num_elts = 0;
  ht->key_elts = malloc(ht->ht_size * sizeof(dll_node_t *));
  assert(ht->key_elts != NULL);
  for (uint64_t i = 0; i < ht->ht_size; i++){
    head = &(ht->key_elts[i]);
    dll_init(head);
  }
  for (uint64_t i = 0; i < prev_ht_size; i++){
    head = &(prev_key_elts[i]);
    if (*head != NULL){
      //assert that ht_size increased so that there is no mutual recursion
      assert(!((float)(ht->num_elts) / ht->ht_size > ht->alpha));
      ht_mul_uint64_insert(ht, (*head)->key, (*head)->elt);
      //NULL: if an element is multilayered, the pointer to it is deleted
      dll_delete(head, *head, NULL);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
  head = NULL;
}
