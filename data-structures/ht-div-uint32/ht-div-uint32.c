/**
   ht-div-uint32.c

   A hash table with generic hash keys and generic elements. The 
   implementation is based on a division method for hashing into upto
   > 2^31 slots (last entry in the PRIMES array) and a chaining method for 
   resolving collisions. Due to chaining, the number of keys and elements
   that can be hashed is not limited by the hash table implementation.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by the 
   alpha parameter. The alpha parameter does not provide an upper bound 
   after the maximum size of a hash table is reached.

   A hash key is an object within a contiguous block of memory (e.g. a basic 
   type, array, struct). An element is an object within a contiguous or
   noncontiguous block of memory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ht-div-uint32.h"
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-rand-mod.h"

static uint32_t hash(const ht_div_uint32_t *ht, const void *key);
static void ht_grow(ht_div_uint32_t *ht);
static void copy_reinsert(ht_div_uint32_t *ht, const dll_node_t *node);

/**
   An array of primes in the increasing order, approximately doubling in 
   magnitude, that are not too close to the powers of 2 and 10 to avoid 
   hashing regularities due to the structure of data.
 */
static const uint32_t PRIMES[22] = {1543, 3119, 6211,
				    12343, 23981, 48673,
				    88843, 186581, 377369,
				    786551, 1483331, 3219497,
				    6278177, 12538919, 25166719,
				    51331771, 112663669, 211326637,
				    412653239, 785367311, 1611612763,
				    3221225479};
static const int PRIMES_COUNT = 22;


/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size 
                 sizeof(ht_div_uint32_t).
   key_size    : size of a key object
   elt_size    : - size of an element object, if the element object is
                 within a contiguous memory block,
                 - size of a pointer to an element object, if the element
                 object is within a noncontiguous memory block
   alpha       : > 0.0, a load factor upper bound.
   cmp_key     : comparison function which returns a zero integer value iff
                 the two key objects pointed to by the first and second
                 arguments are equal
   free_elt    : - if an element is within a contiguous memory block,
                 as reflected by elt_size, and a pointer to the element is 
                 passed as elt in ht_div_uint32_insert, then the element is
                 fully copied into a hash table, and NULL as free_elt is
                 sufficient to delete the element,
                 - if an element is an object within a noncontiguous memory
                 block, and a pointer to a pointer to the element is passed
                 as elt in ht_div_uint32_insert, then the pointer to the
                 element is copied into the hash table, and an element-specific
                 free_elt, taking a pointer to a pointer to an element as its
                 parameter, is necessary to delete the element
*/
void ht_div_uint32_init(ht_div_uint32_t *ht,
                        size_t key_size,
	                size_t elt_size,
			float alpha,
                        int (*cmp_key)(const void *, const void *),
	                void (*free_elt)(void *)){
  ht->count_ix = 0;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->count = PRIMES[ht->count_ix];
  ht->num_elts = 0;
  ht->alpha = alpha;
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (uint32_t i = 0; i < ht->count; i++){
    dll_init(&ht->key_elts[i]);
  }
  ht->cmp_key = cmp_key;
  ht->free_elt = free_elt;
}

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and
   elt parameters are not NULL.
*/
void ht_div_uint32_insert(ht_div_uint32_t *ht,
			  const void *key,
			  const void *elt){
  uint32_t ix;
  dll_node_t **head = NULL, *node = NULL;
  //grow hash table if E[# keys in a slot] > alpha
  while ((float)ht->num_elts / ht->count > ht->alpha &&
	 ht->count_ix < PRIMES_COUNT - 1){
    ht_grow(ht);
  }
  ix = hash(ht, key);
  head = &ht->key_elts[ix];
  node = dll_search_key(head, key, ht->cmp_key);
  if (node == NULL){
    dll_prepend(head, key, elt, ht->key_size, ht->elt_size);
    ht->num_elts++;
  }else{
    dll_delete(head, node, ht->free_elt);
    dll_prepend(head, key, elt, ht->key_size, ht->elt_size);
  }   
}

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_div_uint32_search(const ht_div_uint32_t *ht, const void *key){
  dll_node_t *node = dll_search_key(&ht->key_elts[hash(ht, key)],
				     key,
				     ht->cmp_key);
  if (node == NULL){
    return NULL;
  }else{
    return node->elt;
  }
}

/**
   Removes a key and the associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_div_uint32_remove(ht_div_uint32_t *ht, const void *key, void *elt){
  dll_node_t **head = &ht->key_elts[hash(ht, key)];
  dll_node_t *node = dll_search_key(head, key, ht->cmp_key);
  if (node != NULL){
    memcpy(elt, node->elt, ht->elt_size);
    //if an element is noncontiguous, only the pointer to it is deleted
    dll_delete(head, node, NULL);
    ht->num_elts--;
  }
}

/**
   If a key is present in a hash table, deletes the key and its associated 
   element according free_elt. The key parameter is not NULL.
*/
void ht_div_uint32_delete(ht_div_uint32_t *ht, const void *key){
  dll_node_t **head = &ht->key_elts[hash(ht, key)];
  dll_node_t *node = dll_search_key(head, key, ht->cmp_key);
  if (node != NULL){
    dll_delete(head, node, ht->free_elt);
    ht->num_elts--;
  }
}

/**
   Frees a hash table and leaves a block of size sizeof(ht_div_uint32_t)
   pointed to by the ht parameter.
*/
void ht_div_uint32_free(ht_div_uint32_t *ht){
  for (uint32_t i = 0; i < ht->count; i++){
    dll_free(&ht->key_elts[i], ht->free_elt);
  }
  free(ht->key_elts);
  ht->key_elts = NULL;
}

/** Helper functions */

/**
   Maps a hash key to a slot index in a hash table with a division method. 
*/
static uint32_t hash(const ht_div_uint32_t *ht, const void *key){
  //TODO: change fast_mem_mod to work on 32bit systems
  return fast_mem_mod_uint32(key, (uint64_t)ht->key_size, ht->count);
}

/**
   Increases the size of a hash table by the difference between the ith and 
   (i + 1)th prime numbers in the PRIMES array. Makes no changes if the last
   prime number in the PRIMES array was reached.
*/
static void ht_grow(ht_div_uint32_t *ht){
  uint32_t prev_count = ht->count;
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head = NULL;
  //if the largest size is reached, alpha is not a bound for expectation
  if (ht->count_ix == PRIMES_COUNT - 1) return;
  ht->count_ix++;
  ht->count = PRIMES[ht->count_ix];
  ht->num_elts = 0;
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (uint32_t i = 0; i < ht->count; i++){
    head = &ht->key_elts[i];
    dll_init(head);
  }
  for (uint32_t i = 0; i < prev_count; i++){
    head = &prev_key_elts[i];
    while (*head != NULL){
      copy_reinsert(ht, *head);
      //if an element is noncontiguous, only the pointer to it is deleted
      dll_delete(head, *head, NULL);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
  head = NULL;
}

/**
   Reinserts a copy of a node into a new hash table during a ht_grow 
   operation. In contrast to ht_div_uint32_insert, no search is performed.
*/
static void copy_reinsert(ht_div_uint32_t *ht, const dll_node_t *node){
  dll_prepend(&ht->key_elts[hash(ht, node->key)],
	      node->key,
	      node->elt,
	      ht->key_size,
	      ht->elt_size);
  ht->num_elts++;   
}
