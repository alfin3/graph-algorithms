/**
   ht-div-uint32.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic hash keys and generic elements. The implementation is 
   based on a division method for hashing into upto > 2^31 slots (last entry
   in the PRIMES array) and a chaining method for resolving collisions. Due
   to chaining, the number of keys and elements that can be inserted is not
   limited by the hash table implementation.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by the 
   alpha parameter. The alpha parameter does not provide an upper bound 
   after the maximum count of slots in a hash table is reached.

   A hash key is an object within a contiguous block of memory (e.g. a basic 
   type, array, struct). An element is an object within a contiguous or
   noncontiguous block of memory.
*/

#ifndef HT_DIV_UINT32_H  
#define HT_DIV_UINT32_H

#include <stdint.h>
#include "dll.h"

typedef struct{
  int count_ix;
  size_t key_size;
  size_t elt_size;
  uint32_t count;
  uint32_t num_elts;
  float alpha; 
  dll_node_t **key_elts; //array of pointers to nodes
  int (*cmp_key)(const void *, const void *);
  void (*free_elt)(void *);
} ht_div_uint32_t;

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
	                void (*free_elt)(void *));

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and
   elt parameters are not NULL.
*/
void ht_div_uint32_insert(ht_div_uint32_t *ht,
			  const void *key,
			  const void *elt);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_div_uint32_search(const ht_div_uint32_t *ht, const void *key);

/**
   Removes a key and the associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_div_uint32_remove(ht_div_uint32_t *ht, const void *key, void *elt);

/**
   If a key is present in a hash table, deletes the key and its associated 
   element according free_elt. The key parameter is not NULL.
*/
void ht_div_uint32_delete(ht_div_uint32_t *ht, const void *key);

/**
   Frees a hash table and leaves a block of size sizeof(ht_div_uint32_t)
   pointed to by the ht parameter.
*/
void ht_div_uint32_free(ht_div_uint32_t *ht);

#endif
