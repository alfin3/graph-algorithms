/**
   ht-muloa.h

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

   The implementation only uses integer operations. Integer arithmetic is used
   in load factor operations, thereby eliminating the use of float. Given
   parameter values within the specified ranges, the behavior of the
   implementation is defined as follows: i) a computation is completed or ii)
   an error message is provided and exit is executed if an integer or buffer
   overflow is attempted or an allocation is not completed due to
   insufficient resources. The behavior outside the specified parameter
   ranges is undefined.

   The implementation does not use stdint.h, and is portable under C89/C90
   and C99 with the only requirements that CHAR_BIT * sizeof(size_t) is
   greater or equal to 16 and is even.
*/

#ifndef HT_MULOA_H  
#define HT_MULOA_H

#include <stddef.h>

typedef struct{
  size_t fval; /* first hash value with first bit only set in placeholder */
  size_t sval; /* second hash value */
} key_elt_t; /* given char *p pointer, key is at p + sizeof(key_elt_t) and
                element is at p + sizeof(key_elt_t) + key_size; no key
                and element in a placeholder */

typedef struct{
  size_t key_size;
  size_t elt_size;
  size_t pair_size; /* key_size + elt_size for input iterations by user */
  size_t log_count;
  size_t count;
  size_t max_sum;
  size_t max_num_probes;
  size_t num_elts;
  size_t num_phs;
  size_t fprime; /* >2^{n - 1}, <2^{n}, n = CHAR_BIT * sizeof(size_t) */
  size_t sprime; /* >2^{n - 1}, <2^{n}, n = CHAR_BIT * sizeof(size_t) */
  size_t alpha_n;
  size_t log_alpha_d;
  key_elt_t *ph;
  key_elt_t **key_elts;
  size_t (*rdc_key)(const void *, size_t);
  void (*free_elt)(void *);
} ht_muloa_t;

/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size
                 sizeof(ht_muloa_t).
   key_size    : non-zero size of a key object.
   elt_size    : - non-zero size of an element, if the element is within a
                 contiguous memory block and a copy of the element is
                 inserted,
                 - size of a pointer to an element, if the element
                 is within a noncontiguous memory block or a pointer to a
                 contiguous element is inserted
   min_num     : minimum number of keys that are known or expected to become 
                 present simultaneously in a hash table, resulting in a
                 speedup by avoiding unnecessary growth steps of a hash
                 table; 0 if a positive value is not specified and all growth
                 steps are to be completed
   alpha_n     : > 0 numerator of load factor upper bound
   log_alpha_d : < CHAR_BIT * sizeof(size_t) log base 2 of denominator of
                 load factor upper bound; denominator is a power of two
   rdc_key     : - if NULL and key_size is less or equal to sizeof(size_t),
                 then no reduction operation is performed on a key
                 - if NULL and key_size is greater than sizeof(size_t), then
                 a default mod 2^{CHAR_BIT * sizeof(size_t)} addition routine
                 is performed on a key to reduce it in size
                 - otherwise rdc_key is applied to a key prior to hashing;
                 the first argument points to a key and the second argument
                 provides the size of the key
   free_elt    : - if an element is within a contiguous memory block and
                 a copy of the element was inserted, then NULL as free_elt
                 is sufficient to delete the element,
                 - if an element is within a noncontiguous memory block or
                 a pointer to a contiguous element was inserted, then an
                 element-specific free_elt, taking a pointer to a pointer
                 to an element as its argument and leaving a block of size
                 elt_size pointed to by the argument, is necessary to delete
                 the element
*/
void ht_muloa_init(ht_muloa_t *ht,
		   size_t key_size,
		   size_t elt_size,
		   size_t min_num,
		   size_t alpha_n,
		   size_t log_alpha_d,
		   size_t (*rdc_key)(const void *, size_t),
		   void (*free_elt)(void *));

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL.
*/
void ht_muloa_insert(ht_muloa_t *ht, const void *key, const void *elt);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_muloa_search(const ht_muloa_t *ht, const void *key);

/**
   Removes a key and its associated element from a hash table by copying 
   the element or its pointer into a block of size elt_size pointed to
   by elt. If the key is not in the hash table, leaves the block pointed
   to by elt unchanged. The key and elt parameters are not NULL.
*/
void ht_muloa_remove(ht_muloa_t *ht, const void *key, void *elt);

/**
   If a key is in a hash table, deletes the key and its associated element 
   according to free_elt. The key parameter is not NULL.
*/
void ht_muloa_delete(ht_muloa_t *ht, const void *key);

/**
   Frees a hash table and leaves a block of size sizeof(ht_muloa_t)
   pointed to by the ht parameter.
*/
void ht_muloa_free(ht_muloa_t *ht);

#endif
