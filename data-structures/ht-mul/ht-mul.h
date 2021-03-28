/**
   ht-mul.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic hash keys and generic elements. The implementation
   is based on a multiplication method for hashing into upto
   2^{CHAR_BIT * sizeof(size_t) - 1} slots and an open addressing method
   with double hashing for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by 
   the alpha parameter. The expected number of probes in a search is 
   upper-bounded by 1/(1 - alpha), under the uniform hashing assumption. 

   The alpha parameter does not provide an upper bound after the maximum 
   count of slots in a hash table is reached. After exceeding the alpha
   parameter value, the load factor is <= 1.0 due to open addressing, and the
   expected number of probes is upper-bounded by 1/(1 - load factor) before
   the full occupancy is reached.

   A hash key is an object within a contiguous block of memory (e.g. a basic
   type, array, struct). If the key size is greater than sizeof(size_t)
   bytes, then it is reduced to a sizeof(size_t)-byte block prior to hashing.
   Key size reduction methods may introduce regularities. An element is
   within a contiguous or noncontiguous block of memory.

   The implementation does not use stdint.h and is portable under C89/C90
   with the only requirement that CHAR_BIT * sizeof(size_t) is greater or
   equal to 16 and is even.
*/

#ifndef HT_MUL_H  
#define HT_MUL_H

#include <stddef.h>
#include "dll.h"

typedef struct{
  size_t log_count;
  size_t key_size;
  size_t elt_size;
  size_t count;
  size_t max_count;
  size_t max_num_probes;
  size_t num_elts;
  size_t num_placeholders;
  size_t first_prime; /* >2^{n - 1}, <2^{n}, n = CHAR_BIT * sizeof(size_t) */
  size_t second_prime; /* >2^{n - 1}, <2^{n}, n = CHAR_BIT * sizeof(size_t) */
  float alpha;
  dll_node_t *placeholder; /* node with node->key == NULL */
  dll_node_t **key_elts; /* array of pointers to dlls, each with <= 1 node */
  void (*rdc_key)(void *, const void *);
  void (*free_elt)(void *);
} ht_mul_t;

/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size sizeof(ht_mul_t).
   key_size    : size of a key object
   elt_size    : - size of an element, if the element is within a contiguous
                 memory block,
                 - size of a pointer to an element, if the element is within
                 a noncontiguous memory block
   alpha       : a load factor upper bound that is > 0.0 and < 1.0
   rdc_key     : - if key_size is less or equal to sizeof(size_t) bytes, then
                 rdc_key is NULL
                 - if key_size is greater than sizeof(size_t) bytes, then
                 rdc_key is not NULL and reduces a key to a sizeof(size_t)-
                 byte block prior to hashing; the first argument points to a
                 sizeof(size_t)-byte block, where the reduced form of the
                 block pointed to by the second argument is copied
   free_elt    : - if an element is within a contiguous memory block,
                 as reflected by elt_size, and a pointer to the element is 
                 passed as elt in ht_mul_insert, then the element is
                 fully copied into a hash table, and NULL as free_elt is
                 sufficient to delete the element,
                 - if an element is within a noncontiguous memory block,
                 and a pointer to a pointer to the element is passed
                 as elt in ht_mul_insert, then the pointer to the
                 element is copied into the hash table, and an element-
                 specific free_elt, taking a pointer to a pointer to an
                 element as its argument, is necessary to delete the element
*/
void ht_mul_init(ht_mul_t *ht,
		 size_t key_size,
		 size_t elt_size,
		 float alpha,
		 void (*rdc_key)(void *, const void *),
		 void (*free_elt)(void *));

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL.
*/
void ht_mul_insert(ht_mul_t *ht, const void *key, const void *elt);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_mul_search(const ht_mul_t *ht, const void *key);

/**
   Removes a key and its associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_mul_remove(ht_mul_t *ht, const void *key, void *elt);

/**
   If a key is in a hash table, deletes the key and its associated element 
   according to free_elt. The key parameter is not NULL.
*/
void ht_mul_delete(ht_mul_t *ht, const void *key);

/**
   Frees a hash table and leaves a block of size sizeof(ht_mul_t)
   pointed to by the ht parameter.
*/
void ht_mul_free(ht_mul_t *ht);

#endif
