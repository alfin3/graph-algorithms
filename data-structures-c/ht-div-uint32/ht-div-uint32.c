/**
   ht-div-uint32.c

   A hash table with generic hash keys and generic elements. The 
   implementation is based on a division method for hashing into 2^32-1 
   slots and a chaining method for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by the 
   alpha parameter. The alpha parameter does not provide an upper bound 
   after the maximum size of a hash table is reached.

   A hash key is an object within a continuous block of memory (e.g. a basic 
   type, array, struct). 

   An element is an object within a continuous block of memory (e.g. a basic 
   type, array, struct), or a multilayered object in memory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "ht-div-uint32.h"
#include "dll.h"
#include "utilities-ds.h"

static uint32_t hash(ht_div_uint32_t *ht, void *key);
static void ht_grow(ht_div_uint32_t *ht);

/**
   An array of primes in the increasing order, approximately doubling in 
   magnitude, that are not too close to the powers of 2 and 10 to avoid 
   hashing regularities due to the structure of data.
 */
static const uint32_t primes[22] = {1543, 3119, 6211,
				    12343, 23981, 48673,
				    88843, 186581, 377369,
				    786551, 1483331, 3219497,
				    6278177, 12538919, 25166719,
				    51331771, 112663669, 211326637,
				    412653239, 785367311, 1611612763,
				    3221225479};
static const int num_primes = 22;


/**
   Initializes a hash table. 
   ht: a pointer to a previously created ht_div_uint32_t instance.
   alpha: > 0.0, a load factor upper bound.
   cmp_key_fn: 0 if two keys are equal.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in 
                ht_div_uint32_insert, then the element is fully copied into 
                a block pointed from a node of the dll at the slot index 
                computed by a hash function, and a NULL as free_elt_fn is 
                sufficient to delete the element;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in ht_div_uint32_insert, then
                the pointer to the element is copied into a block pointed 
                from a node of the dll at the slot index computed by a hash 
                function, and an element-specific free_elt_fn is necessary 
                to delete the element.
*/
void ht_div_uint32_init(ht_div_uint32_t *ht,
                        int key_size,
	                int elt_size,
			float alpha,
                        int (*cmp_key_fn)(void *, void *),
	                void (*free_elt_fn)(void *)){
  ht->ht_size_ix = 0;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->ht_size = primes[0];
  ht->num_elts = 0;
  ht->alpha = alpha;
  ht->key_elts = malloc(ht->ht_size * sizeof(dll_node_t *));
  assert(ht->key_elts != NULL);
  for (uint32_t i = 0; i < ht->ht_size; i++){
    dll_init(&(ht->key_elts[i]));
  }
  ht->cmp_key_fn = cmp_key_fn;
  ht->free_elt_fn = free_elt_fn;
}

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key to the new element. The key and elt 
   parameters are not NULL.
*/
void ht_div_uint32_insert(ht_div_uint32_t *ht, void *key, void *elt){
  //grow ht if E[# keys in a slot] > alpha
  if ((float)(ht->num_elts) / ht->ht_size > ht->alpha){ht_grow(ht);}
  uint32_t ix = hash(ht, key);
  dll_node_t **head = &(ht->key_elts[ix]);
  dll_node_t *node = dll_search_key(head, key, ht->cmp_key_fn);
  if (node == NULL){
    dll_insert(head, key, elt, ht->key_size, ht->elt_size);
    ht->num_elts++;
  }else{
    dll_delete(head, node, ht->free_elt_fn);
    dll_insert(head, key, elt, ht->key_size, ht->elt_size);
  }   
}

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_div_uint32_search(ht_div_uint32_t *ht, void *key){
  uint32_t ix = hash(ht, key);
  dll_node_t **head = &(ht->key_elts[ix]);
  dll_node_t *node = dll_search_key(head, key, ht->cmp_key_fn);
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
void ht_div_uint32_remove(ht_div_uint32_t *ht, void *key, void *elt){
  uint32_t ix = hash(ht, key);
  dll_node_t **head = &(ht->key_elts[ix]);
  dll_node_t *node = dll_search_key(head, key, ht->cmp_key_fn);
  if (node != NULL){
    memcpy(elt, node->elt, ht->elt_size);
    //NULL: if an element is multilayered, the pointer to it is deleted
    dll_delete(head, node, NULL);
    ht->num_elts--;
  }
}

/**
   Deletes a key and its associated element in a hash table according to 
   free_elt_fn. The key parameter is not NULL.
*/
void ht_div_uint32_delete(ht_div_uint32_t *ht, void *key){
  uint32_t ix = hash(ht, key);
  dll_node_t **head = &(ht->key_elts[ix]);
  dll_node_t *node = dll_search_key(head, key, ht->cmp_key_fn);
  if (node != NULL){
    dll_delete(head, node, ht->free_elt_fn);
    ht->num_elts--;
  }
}

/**
   Frees a hash table.
*/
void ht_div_uint32_free(ht_div_uint32_t *ht){
  for (uint32_t i = 0; i < ht->ht_size; i++){
    dll_free(&(ht->key_elts[i]), ht->free_elt_fn);
  }
  free(ht->key_elts);
  ht->key_elts = NULL;
}

/** Helper functions */

/**
   Maps a hash key to a slot index in a hash table with a division method. 
   The hash key is an object of size key_size within a continuous block of 
   memory pointed to by key.
*/
static uint32_t hash(ht_div_uint32_t *ht, void *key){
  return fast_mem_mod_uint32(key, (uint64_t)ht->key_size, ht->ht_size);
}

/**
   Increases the size of a hash table by the difference between the ith and 
   (i+1)th prime numbers in the primes array. Makes no changes if the last
   prime number in the primes array was reached.
*/
static void ht_grow(ht_div_uint32_t *ht){
  //if the largest size is reached, alpha is not a bound for expectation
  if (ht->ht_size_ix == num_primes - 1){return;}
  uint32_t prev_ht_size = ht->ht_size;
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head;
  ht->ht_size_ix++;
  ht->ht_size = primes[ht->ht_size_ix];
  ht->num_elts = 0;
  ht->key_elts = malloc(ht->ht_size * sizeof(dll_node_t *));
  assert(ht->key_elts != NULL);
  for (uint32_t i = 0; i < ht->ht_size; i++){
    head = &(ht->key_elts[i]);
    dll_init(head);
  }
  for (uint32_t i = 0; i < prev_ht_size; i++){
    head = &(prev_key_elts[i]);
    while (*head != NULL){
      //assert that ht_size increased so that there is no mutual recursion
      assert(!((float)(ht->num_elts) / ht->ht_size > ht->alpha));
      ht_div_uint32_insert(ht, (*head)->key, (*head)->elt);
      //NULL: if an element is multilayered, the pointer to it is deleted
      dll_delete(head, *head, NULL);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
  head = NULL;
}
