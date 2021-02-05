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
   count of slots in a hash table is reached. After exceeding the alpha
   parameter, the load factor is <= 1.0 due to open addressing, and the
   expected number of probes is upper-bounded by 1/(1 - load factor) before
   the full occupancy is reached.

   A hash key is an object within a contiguous block of memory (e.g. a basic
   type, array, struct). If the key size is greater than 8 bytes, then it
   is reduced to a 8-byte block prior to hashing. Key size reduction 
   methods may introduce regularities. An element is an object within a
   contiguous or noncontiguous block of memory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ht-mul-uint64.h"
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-rand-mod.h"

static const uint64_t FIRST_PRIME = 15769474759331449193U; //2^63 < p < 2^64
static const uint64_t SECOND_PRIME = 18292551137159601919U; //2^63 < p < 2^64
static const size_t UINT64_SIZE = sizeof(uint64_t);
static const size_t UINT64_BIT_COUNT = sizeof(uint64_t) * 8;
static size_t CMP_KEY_SIZE;

//placeholder object handling
static void placeholder_init(dll_node_t *node, int elt_size);
static int is_placeholder(const dll_node_t *node);
static void placeholder_free(dll_node_t *node);

//hashing
static uint64_t convert_std_key(const ht_mul_uint64_t *ht, const void *key);
static uint64_t hash(uint64_t prime, uint64_t std_key);
static uint64_t adjust_hash_dist(uint64_t dist);
static uint64_t probe_dbl_hash(const ht_mul_uint64_t *ht,
			       uint64_t dist,
			       uint64_t ix);

//hash table operations
static dll_node_t **search(const ht_mul_uint64_t *ht, const void *key);
static uint64_t *first_val_ptr(const void *key_block, size_t key_size);
static uint64_t *second_val_ptr(const void *key_block, size_t key_size);
static int cmp_key(const void *a, const void *b);

//hash table maintenance
static void ht_grow(ht_mul_uint64_t *ht);
static void ht_clean(ht_mul_uint64_t *ht);
static void reinsert(ht_mul_uint64_t *ht, const dll_node_t *node);

/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size 
                 sizeof(ht_mul_uint64_t).
   key_size    : size of a key object within a contiguous memory block
   elt_size    : - size of an element object, if the element object is
                 within a contiguous memory block,
                 - size of a pointer to an element object, if the element
                 object is within a noncontiguous memory block
   alpha       : a load factor upper bound that is > 0.0 and < 1.0
   rdc_key     : - if key_size is less or equal to 8 bytes, then rdc_key
                 is NULL
                 - if key_size is greater than 8 bytes, then rdc_key
                 is not NULL and reduces a key to a 8-byte block prior to
                 hashing; the first parameter points to an 8-byte block,
                 where the reduced form of the block pointed to by the second
                 parameter is copied
   free_elt    : - if an element is within a contiguous memory block,
                 as reflected by elt_size, and a pointer to the element is 
                 passed as elt in ht_mul_uint64_insert, then the element is
                 fully copied into a hash table, and NULL as free_elt is
                 sufficient to delete the element,
                 - if an element is an object within a noncontiguous memory
                 block, and a pointer to a pointer to the element is passed
                 as elt in ht_mul_uint64_insert, then the pointer to the
                 element is copied into the hash table, and an element-
                 specific free_elt, taking a pointer to a pointer to an
                 element as its parameter, is necessary to delete the element
*/
void ht_mul_uint64_init(ht_mul_uint64_t *ht,
                        size_t key_size,
	                size_t elt_size,
			float alpha,
                        void (*rdc_key)(void *, const void *),
	                void (*free_elt)(void *)){
  ht->log_count = 10;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->count = pow_two_uint64(ht->log_count);
  ht->max_count = pow_two_uint64(63);
  ht->max_num_probes = 1; //at least one probe
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->alpha = alpha;
  ht->placeholder = malloc_perror(sizeof(dll_node_t));
  placeholder_init(ht->placeholder, elt_size);
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (uint64_t i = 0; i < ht->count; i++){
    dll_init(&ht->key_elts[i]);
  }
  ht->rdc_key = rdc_key;
  ht->free_elt = free_elt;
  CMP_KEY_SIZE = key_size;
}

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL.
*/
void ht_mul_uint64_insert(ht_mul_uint64_t *ht,
			  const void *key,
			  const void *elt){
  uint64_t num_probes = 1;
  uint64_t std_key, first_val, second_val, ix, dist;
  size_t key_block_size = ht->key_size + 2 * UINT64_SIZE;
  void *key_block = NULL;
  dll_node_t **head = NULL;
  while ((float)(ht->num_elts + ht->num_placeholders) / ht->count >
	 ht->alpha){
    //clean or grow if E[# keys in a slot] > alpha
    if (ht->num_elts < ht->num_placeholders){
      ht_clean(ht);
    }else if (ht->count < ht->max_count){
      ht_grow(ht);
    }else{
      break;
    }
  }
  std_key = convert_std_key(ht, key);
  first_val = hash(FIRST_PRIME, std_key); 
  second_val = hash(SECOND_PRIME, std_key);
  //prepare a hash key and two hash values for storage as a block
  key_block = malloc_perror(key_block_size);
  memcpy(key_block, key, ht->key_size);
  *first_val_ptr(key_block, ht->key_size) = first_val;
  *second_val_ptr(key_block, ht->key_size) = second_val;
  ix = first_val >> (UINT64_BIT_COUNT - ht->log_count);
  dist = adjust_hash_dist(second_val >> (UINT64_BIT_COUNT - ht->log_count));
  head = &ht->key_elts[ix];
  while (*head != NULL){
    if (!is_placeholder(*head) &&
	dll_search_key(head, key, cmp_key) != NULL){
      dll_delete(head, *head, ht->free_elt);
      dll_prepend(head, key_block, elt, key_block_size, ht->elt_size);
      free(key_block);
      key_block = NULL;
      return;
    }
    ix = probe_dbl_hash(ht, dist, ix);
    head = &ht->key_elts[ix];
    num_probes++;
    if (num_probes > ht->max_num_probes){
      ht->max_num_probes++;
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
void *ht_mul_uint64_search(const ht_mul_uint64_t *ht, const void *key){
  dll_node_t **head = search(ht, key);
  if (head != NULL){
    return (*head)->elt;
  }else{
    return NULL;
  }
}

/**
   Removes a key and its associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_mul_uint64_remove(ht_mul_uint64_t *ht, const void *key, void *elt){
  dll_node_t **head = search(ht, key);
  if (head != NULL){
    memcpy(elt, (*head)->elt, ht->elt_size);
    //if an element is noncontiguous, only the pointer to it is deleted
    dll_delete(head, *head, NULL);
    *head = ht->placeholder;
    ht->num_elts--;
    ht->num_placeholders++;
  }
}

/**
   If a key is in a hash table, deletes the key and its associated element 
   according to free_elt. The key parameter is not NULL.
*/
void ht_mul_uint64_delete(ht_mul_uint64_t *ht, const void *key){
  dll_node_t **head = search(ht, key);
  if (head != NULL){
    dll_delete(head, *head, ht->free_elt);
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
  dll_node_t **head = NULL;
  for (uint64_t i = 0; i < ht->count; i++){
    head = &ht->key_elts[i];
    if (*head != NULL && !is_placeholder(*head)){
      dll_free(head, ht->free_elt);
    }
  }
  free(ht->key_elts);
  placeholder_free(ht->placeholder);
  free(ht->placeholder);
  ht->key_elts = NULL;
  ht->placeholder = NULL;
  head = NULL;
}

/** Helper functions */

/**
   Initializes a placeholder node. The node parameter points to a
   preallocated block of size sizeof(dll_node_t).
*/
static void placeholder_init(dll_node_t *node, int elt_size){
  node->key_size = 0;
  node->elt_size = elt_size;
  node->key = NULL; //no element in a hash table can have node->key == NULL
  node->elt = calloc_perror(1, elt_size); //element for consistency purposes
  node->next = node;
  node->prev = node;
}

/**
   Tests if a node is a placeholder node.
*/
static int is_placeholder(const dll_node_t *node){
  if (node->key == NULL){
    return 1;
  }else{
    return 0;
  }
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
static uint64_t convert_std_key(const ht_mul_uint64_t *ht, const void *key){
  uint64_t std_key = 0; //initialize all bits
  if (ht->key_size > sizeof(uint64_t)){
    ht->rdc_key(&std_key, key);
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
static uint64_t adjust_hash_dist(uint64_t dist){
  uint64_t ret = dist;
  if (!(dist & 1)){
    if (dist == 0){
      ret++;
    }else{
      ret--;
    }
  }
  return ret;
}

/**
   Returns the next index in a hash table, based on double hashing.
*/
static uint64_t probe_dbl_hash(const ht_mul_uint64_t *ht,
			       uint64_t dist,
			       uint64_t ix){
  return sum_mod_uint64(dist, ix, ht->count);
}

/**
   If a key is present in a hash table, returns a head pointer to the dll
   with a single node containing the key, otherwise returns NULL.
*/
static dll_node_t **search(const ht_mul_uint64_t *ht, const void *key){
  uint64_t num_probes = 1;
  uint64_t std_key, first_val, second_val, ix, dist;
  dll_node_t **head = NULL;
  std_key = convert_std_key(ht, key);
  first_val = hash(FIRST_PRIME, std_key); 
  second_val = hash(SECOND_PRIME, std_key); 
  ix = first_val >> (UINT64_BIT_COUNT - ht->log_count);
  dist = adjust_hash_dist(second_val >> (UINT64_BIT_COUNT - ht->log_count));
  head = &ht->key_elts[ix];
  while (*head != NULL){
    if (!is_placeholder(*head) &&
	dll_search_key(head, key, cmp_key) != NULL){
      return head;
    }else if (num_probes == ht->max_num_probes){
      break;
    }else{
      ix = probe_dbl_hash(ht, dist, ix);
      head = &ht->key_elts[ix];
      num_probes++;
    }
  }
  return NULL;
}

/**
   Doubles the count of a hash table. Makes no changes if the maximum count
   has been reached.
*/
static void ht_grow(ht_mul_uint64_t *ht){
  uint64_t prev_count = ht->count;
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head = NULL;
  if (ht->count == ht->max_count) return;
  ht->log_count++;
  ht->count *= 2;
  ht->max_num_probes = 1;
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (uint64_t i = 0; i < ht->count; i++){
    head = &ht->key_elts[i];
    dll_init(head);
  }
  for (uint64_t i = 0; i < prev_count; i++){
    head = &prev_key_elts[i];
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
  dll_node_t **head = NULL;
  ht->max_num_probes = 1;
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (uint64_t i = 0; i < ht->count; i++){
    head = &ht->key_elts[i];
    dll_init(head);
  }
  for (uint64_t i = 0; i < ht->count; i++){
    head = &prev_key_elts[i];
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
   ht_grow and ht_clean operations by recomputing the hash values with 
   bit shifting and without multiplication.
*/
static void reinsert(ht_mul_uint64_t *ht, const dll_node_t *node){
  uint64_t num_probes = 1;
  uint64_t first_val, second_val, ix, dist;
  dll_node_t **head = NULL;
  first_val = *first_val_ptr(node->key, ht->key_size);
  second_val = *second_val_ptr(node->key, ht->key_size);
  ix = first_val >> (UINT64_BIT_COUNT - ht->log_count);
  dist = adjust_hash_dist(second_val >> (UINT64_BIT_COUNT - ht->log_count));
  head = &ht->key_elts[ix];
  while (*head != NULL){
    ix = probe_dbl_hash(ht, dist, ix);
    head = &ht->key_elts[ix];
    num_probes++;
    if (num_probes > ht->max_num_probes){
      ht->max_num_probes++;
    }
  }
  *head = (dll_node_t *)node;
  ht->num_elts++;
}

/**
   Computes pointers to the first and second hash values in a key block.
*/
static uint64_t *first_val_ptr(const void* key_block, size_t key_size){
  return (uint64_t *)((char *)key_block + key_size);
}

static uint64_t *second_val_ptr(const void* key_block, size_t key_size){
  return (uint64_t *)((char *)key_block + key_size + UINT64_SIZE);
}

/**
   Compares two hash keys.
*/
static int cmp_key(const void *a, const void *b){
  return memcmp(a, b, CMP_KEY_SIZE);
}
