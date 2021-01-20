/**
   ht-mul-uint64.c

   A hash table with generic hash keys and generic elements. The 
   implementation is based on a multiplication method for hashing into upto 
   2^63 slots (the upper range requiring > 2^64 addresses) and an open 
   addressing method with double hashing for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by 
   the alpha parameter. The expected number of probes in a search is 
   upper-bounded by 1/(1 - alpha), under the uniform hashing assumption. 

   The alpha parameter does not provide an upper bound after the maximum 
   size of a hash table is reached. After exceeding the alpha parameter, 
   the load factor is <= 1.0 due to open addressing, and the expected number 
   of probes is upper-bounded by 1/(1 - load factor) before the full 
   occupancy is reached.

   A hash key is an object within a continuous block of memory (e.g. a basic 
   type, array, struct). If the key size is <= 8 bytes, then the key size 
   reduction function rdc_key_fn is NULL. Otherwise, rdc_key_fn is not NULL 
   and reduces a key to a 8-byte block prior to hashing. Key size reduction 
   methods may introduce regularities.

   An element is an object within a continuous block of memory (e.g. a basic 
   type, array, struct), or a multilayered object in memory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "ht-mul-uint64.h"
#include "dll.h"
#include "utilities-rand-mod.h"

//primes for double hashing, each prime p is 2^63 < p < 2^64
static const uint64_t first_prime = 15769474759331449193U;
static const uint64_t second_prime = 18292551137159601919U;
//placeholder object handling
static void placeholder_init(dll_node_t *node, int elt_size);
static bool is_placeholder(dll_node_t *node);
static void placeholder_free(dll_node_t *node);
//hash table operations
static uint64_t convert_std_key(ht_mul_uint64_t *ht, void *key);
static uint64_t hash(uint64_t prime, uint64_t std_key);
static uint64_t adjust_hash_dist(uint64_t d);
static uint64_t probe_dbl_hash(ht_mul_uint64_t *ht, uint64_t d, uint64_t ix);
static dll_node_t **search(ht_mul_uint64_t *ht, void *key);
//hash table maintenance
static void ht_grow(ht_mul_uint64_t *ht);
static void ht_clean(ht_mul_uint64_t *ht);
static void reinsert(ht_mul_uint64_t *ht, dll_node_t *node);

/**
   Initializes a hash table. 
   ht: a pointer to a previously created ht_mul_uint64_t instance.
   alpha: 0.0 < alpha < 1.0, a load factor upper bound.
   cmp_key_fn: 0 iff two keys are equal.
   rdc_key_fn: the first parameter points to an 8-byte block, where the 
               reduced form of the second parameter is copied.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in 
                ht_mul_uint64_insert, then the element is fully copied into 
                a block pointed from the node at the slot index computed by 
                hash function(s), and a NULL as free_elt_fn is sufficient to 
                delete the element;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in ht_mul_uint64_insert, then
                the pointer to the element is copied into a block pointed 
                from the node at the slot index computed by hash 
                function(s), and an element-specific free_elt_fn is necessary
                to delete the element.
*/
void ht_mul_uint64_init(ht_mul_uint64_t *ht,
                        int key_size,
	                int elt_size,
			float alpha,
                        int (*cmp_key_fn)(const void *, const void *),
                        void (*rdc_key_fn)(void *, void *),
	                void (*free_elt_fn)(void *)){
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->log_ht_size = 10;
  ht->ht_size = pow_two_uint64(ht->log_ht_size); //a power of two
  ht->max_ht_size = pow_two_uint64(63);
  ht->max_num_probes = 1; //at least one probe
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->alpha = alpha;
  ht->placeholder = malloc(sizeof(dll_node_t));
  assert(ht->placeholder != NULL);
  placeholder_init(ht->placeholder, elt_size);
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
  //clean or grow if E[# keys in a slot] > alpha
  while ((float)(ht->num_elts + ht->num_placeholders) / ht->ht_size >
      ht->alpha){
    if (ht->num_elts < ht->num_placeholders){
      ht_clean(ht);
    }else if (ht->ht_size < ht->max_ht_size){
      //> 1 ht_grow calls, if alpha is small relative to the initial ht_size
      ht_grow(ht);
    }else{
      break;
    }
  }
  dll_node_t **head;
  uint64_t num_probes = 1;
  uint64_t std_key = convert_std_key(ht, key);
  uint64_t first_val = hash(first_prime, std_key); 
  uint64_t second_val = hash(second_prime, std_key);
  uint64_t ix = first_val >> (64 - ht->log_ht_size);
  uint64_t d = adjust_hash_dist(second_val >> (64 - ht->log_ht_size));
  //prepare hash key and hash values for storage as a block
  int key_block_size = ht->key_size + 2 * sizeof(uint64_t);
  void *key_block = malloc(key_block_size);
  assert(key_block != NULL);
  uint64_t *val_ptr = (uint64_t *)((char *)key_block + ht->key_size);
  memcpy(key_block, key, ht->key_size);
  *val_ptr = first_val;
  *(++val_ptr) = second_val;
  head = &(ht->key_elts[ix]);
  while (*head != NULL){
    //dll_search_key is called if is_placeholder returns false
    if (!is_placeholder(*head) &&
	dll_search_key(head, key, ht->cmp_key_fn) != NULL){
      dll_delete(head, *head, ht->free_elt_fn);
      dll_prepend(head, key_block, elt, key_block_size, ht->elt_size);
      return;
    }
    ix = probe_dbl_hash(ht, d, ix);
    head = &(ht->key_elts[ix]);
    num_probes++;
    if (num_probes > ht->max_num_probes){
      ht->max_num_probes = num_probes;
    }
  }
  dll_prepend(head, key_block, elt, key_block_size, ht->elt_size);
  ht->num_elts++;
  free(key_block);
  key_block = NULL;
}

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_mul_uint64_search(ht_mul_uint64_t *ht, void *key){
  dll_node_t **head = search(ht, key);
  if (head != NULL){return (*head)->elt;}
  return NULL;
}

/**
   Removes a key and its associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_mul_uint64_remove(ht_mul_uint64_t *ht, void *key, void *elt){
  dll_node_t **head = search(ht, key);
  if (head != NULL){
    memcpy(elt, (*head)->elt, ht->elt_size);
    //NULL: if an element is multilayered, only the pointer to it is deleted
    dll_delete(head, *head, NULL);
    *head = ht->placeholder;
    ht->num_elts--;
    ht->num_placeholders++;
  }
}

/**
   If a key is in a hash table, deletes the key and its associated element 
   according to free_elt_fn. The key parameter is not NULL.
*/
void ht_mul_uint64_delete(ht_mul_uint64_t *ht, void *key){
  dll_node_t **head = search(ht, key);
  if (head != NULL){
    dll_delete(head, *head, ht->free_elt_fn);
    *head = ht->placeholder;
    ht->num_elts--;
    ht->num_placeholders++;
  }
}

/**
   Frees a hash table and leaves a block of size sizeof(ht_mul_uint64_t)
   pointed to by the ht parameter.
*/
void ht_mul_uint64_free(ht_mul_uint64_t *ht){
  dll_node_t **head;
  for (uint64_t i = 0; i < ht->ht_size; i++){
    head = &(ht->key_elts[i]);
    if (*head != NULL && !is_placeholder(*head)){
      dll_free(head, ht->free_elt_fn);
    }
  }
  free(ht->key_elts);
  ht->key_elts = NULL;
  placeholder_free(ht->placeholder);
  free(ht->placeholder);
  ht->placeholder = NULL;
  head = NULL;
}

/** Helper functions */

/**
   Initializes a placeholder node. The node parameter points to a 
   preallocated block of size sizeof(dll_node_t).
*/
static void placeholder_init(dll_node_t *node, int elt_size){
  node->key = NULL; //no element in a hash table can have node->key == NULL
  node->key_size = 0;
  node->elt = calloc(1, elt_size); //element for node consistency purposes
  assert(node->elt != NULL);
  node->elt_size = elt_size;
  node->next = NULL;
  node->prev = NULL;
}

/**
   Tests if a node is a placeholder node.
*/
static bool is_placeholder(dll_node_t *node){
  if (node->key == NULL){return true;}
  return false;
}

/**
   Frees a placeholder node and leaves a block of size sizeof(dll_node_t) 
   pointed to by the node parameter.
*/
static void placeholder_free(dll_node_t *node){
  free(node->elt);
  node->elt = NULL;
}

/**
   Converts a key to a key of the standard size of 8 bytes.
*/
static uint64_t convert_std_key(ht_mul_uint64_t *ht, void *key){
  uint64_t std_key = 0; //initialize all bits if key_size < sizeof(uint64_t)
  if (ht->key_size > (int)sizeof(uint64_t)){
    assert(ht->rdc_key_fn != NULL);
    ht->rdc_key_fn(&std_key, key);
  }else{ 
    memcpy(&std_key, key, ht->key_size);
  }
  return std_key;
}

/**
   Maps a hash key to a hash value without subsequent bit shifting. The 
   latter is necessary to determine the index of a slot in a hash table or 
   a probe distance.
*/
static uint64_t hash(uint64_t prime, uint64_t std_key){
  return mul_mod_pow_two_64(prime, std_key);
}

/**
   Adjusts a probe distance to an odd distance, if necessary. 
*/
static uint64_t adjust_hash_dist(uint64_t d){
  if (!(d & 1)){
    if (d == 0){
      d++;
    }else{
      d--;
    }
  }
  return d;
}

/**
   Returns the next index in a hash table, based on double hashing.
*/
static uint64_t probe_dbl_hash(ht_mul_uint64_t *ht, uint64_t d, uint64_t ix){
  return sum_mod_uint64(d, ix, ht->ht_size);
}

/**
   If a key is present in a hash table, returns a head pointer to the dll
   with a single node containing the key, otherwise returns NULL.
*/
static dll_node_t **search(ht_mul_uint64_t *ht, void *key){
  dll_node_t **head;
  uint64_t std_key = convert_std_key(ht, key);
  uint64_t first_val = hash(first_prime, std_key); 
  uint64_t second_val = hash(second_prime, std_key); 
  uint64_t ix = first_val >> (64 - ht->log_ht_size);
  uint64_t d = adjust_hash_dist(second_val >> (64 - ht->log_ht_size));
  uint64_t num_probes = 1;
  head = &(ht->key_elts[ix]);
  while (*head != NULL){
    //dll_search_key is called if is_placeholder returns false
    if (!is_placeholder(*head) &&
	dll_search_key(head, key, ht->cmp_key_fn) != NULL){
      return head;
    }else if (num_probes == ht->max_num_probes){
      break;
    }else{
      ix = probe_dbl_hash(ht, d, ix);
      head = &(ht->key_elts[ix]);
      num_probes++;
    }
  }
  return NULL;
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
  ht->log_ht_size++;
  ht->ht_size *= 2;
  ht->max_num_probes = 1;
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->key_elts = malloc(ht->ht_size * sizeof(dll_node_t *));
  assert(ht->key_elts != NULL);
  for (uint64_t i = 0; i < ht->ht_size; i++){
    head = &(ht->key_elts[i]);
    dll_init(head);
  }
  for (uint64_t i = 0; i < prev_ht_size; i++){
    head = &(prev_key_elts[i]);
    if (*head != NULL && !is_placeholder(*head)){
      reinsert(ht, *head);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
  head = NULL;
}

/**
   Eliminates the placeholders left by delete and remove operations.
   If called when num_elts < num_placeholders, then for every delete/remove 
   operation at most one rehashing operation is performed, resulting in a 
   constant overhead of at most one rehashing per delete/remove operation.
*/
static void ht_clean(ht_mul_uint64_t *ht){
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head;
  ht->max_num_probes = 1;
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->key_elts = malloc(ht->ht_size * sizeof(dll_node_t *));
  assert(ht->key_elts != NULL);
  for (uint64_t i = 0; i < ht->ht_size; i++){
    head = &(ht->key_elts[i]);
    dll_init(head);
  }
  for (uint64_t i = 0; i < ht->ht_size; i++){
    head = &(prev_key_elts[i]);
    if (*head != NULL && !is_placeholder(*head)){
      reinsert(ht, *head);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
  head = NULL;
}

/**
   Reinserts a key and an associated element into a new hash table during 
   ht_grow and ht_clean operations by recomputing the hash values with a 
   bit shifts and without multiplication.
*/
static void reinsert(ht_mul_uint64_t *ht, dll_node_t *node){
  dll_node_t **head;
  uint64_t *val_ptr = (uint64_t *)((char *)node->key + ht->key_size);
  uint64_t first_val = *val_ptr;
  uint64_t second_val = *(++val_ptr);
  uint64_t ix = first_val >> (64 - ht->log_ht_size);
  uint64_t d = adjust_hash_dist(second_val >> (64 - ht->log_ht_size));
  uint64_t num_probes = 1;
  head = &(ht->key_elts[ix]);
  while (*head != NULL){
    ix = probe_dbl_hash(ht, d, ix);
    head = &(ht->key_elts[ix]);
    num_probes++;
    if (num_probes > ht->max_num_probes){
      ht->max_num_probes = num_probes;
    }
  }
  *head = node;
  ht->num_elts++;
}
