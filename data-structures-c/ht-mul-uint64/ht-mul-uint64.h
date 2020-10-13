/**
   ht-mul-uint64.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic hash keys and generic elements. The implementation is 
   based on a multiplication method for hashing into upto 2^63 slots 
   (the upper range requiring > 2^64 addresses) and an open addressing
   method (with linear probing at this time) for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by 
   the alpha parameter. The expected number of probes in a search is 
   upper-bounded by 1/(1 - alpha), under the uniform hashing assumption. 

   The alpha parameter does not provide an upper bound after the maximum 
   size of a hash table is reached. After exceeding the alpha parameter, 
   the load factor is < 1.0 due to open addressing, and the expected number 
   of probes is upper-bounded by 1/(1 - load factor).

   A hash key is an object within a continuous block of memory (e.g. a basic 
   type, array, struct). If the key size is <= 8 bytes, then the key size 
   reduction function rdc_key_fn is NULL. Otherwise, rdc_key_fn is not NULL 
   and reduces a key to a 8-byte block prior to hashing. Key size reduction 
   methods may introduce regularities.

   An element is an object within a continuous block of memory (e.g. a basic 
   type, array, struct), or a multilayered object in memory.
*/

#ifndef HT_MUL_UINT64_H  
#define HT_MUL_UINT64_H

#include <stdint.h>
#include "dll.h"

typedef struct{
  int key_size;
  int elt_size;
  int log_ht_size;
  uint64_t ht_size;
  uint64_t max_ht_size;
  uint64_t max_num_probes;
  uint64_t num_elts;
  uint64_t num_placeholders;
  float alpha;
  dll_node_t *placeholder; //node with key == NULL
  dll_node_t **key_elts; //array of pointers to dlls, each with <= 1 node
  int (*cmp_key_fn)(void *, void *);
  void (*rdc_key_fn)(void *, void *);
  void (*free_elt_fn)(void *);
} ht_mul_uint64_t;

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
                a block pointed from the node at the slot index computed by 
                a hash function, and a NULL as free_elt_fn is sufficient to 
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
	                void (*free_elt_fn)(void *));

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL.
*/
void ht_mul_uint64_insert(ht_mul_uint64_t *ht, void *key, void *elt);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_mul_uint64_search(ht_mul_uint64_t *ht, void *key);

/**
   Removes a key and its associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_mul_uint64_remove(ht_mul_uint64_t *ht, void *key, void *elt);

/**
   Deletes a key and its associated element in a hash table according to 
   free_elt_fn. The key parameter is not NULL.
*/
void ht_mul_uint64_delete(ht_mul_uint64_t *ht, void *key);

/**
   Frees a hash table.
*/
void ht_mul_uint64_free(ht_mul_uint64_t *ht);

#endif
