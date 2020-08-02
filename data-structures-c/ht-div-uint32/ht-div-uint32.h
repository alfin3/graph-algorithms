/**
   ht-div-uint32.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic hash keys and generic elements. The implementation is 
   based on a division method for hashing into 2^32-1 slots and a chaining
   method for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by the 
   alpha parameter. The alpha parameter does not provide an upper bound 
   after the maximum size of a hash table is reached.

   A hash key is an object within a continuous block of memory (e.g. a basic 
   type, array, struct). 

   An element is an object within a continuous block of memory (e.g. a basic 
   type, array, struct), or a multilayered object in memory.
*/

#ifndef HT_DIV_UINT32_H  
#define HT_DIV_UINT32_H

#include <stdint.h>
#include "dll.h"

typedef struct{
  int ht_size_ix;
  int key_size;
  int elt_size;
  uint32_t ht_size;
  uint32_t num_elts;
  float alpha; 
  dll_node_t **key_elts; //array of pointers to nodes
  int (*cmp_key_fn)(void *, void *);
  void (*free_elt_fn)(void *);
} ht_div_uint32_t;

/**
   Initializes a hash table. 
   ht: a pointer to a previously created ht_div_uint32_t instance.
   alpha: > 0.0, a load factor upper bound.
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
	                void (*free_elt_fn)(void *));

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key to the new element. The key and elt 
   parameters are not NULL.
*/
void ht_div_uint32_insert(ht_div_uint32_t *ht, void *key, void *elt);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_div_uint32_search(ht_div_uint32_t *ht, void *key);

/**
   Removes a key and the associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_div_uint32_remove(ht_div_uint32_t *ht, void *key, void *elt);

/**
   Deletes a key and its associated element in a hash table according to 
   free_elt_fn. The key parameter is not NULL.
*/
void ht_div_uint32_delete(ht_div_uint32_t *ht, void *key);

/**
   Frees a hash table.
*/
void ht_div_uint32_free(ht_div_uint32_t *ht);

#endif
