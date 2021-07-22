/**
   ht-muloa.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic hash keys and generic elements. The implementation
   is based on a multiplication method for hashing into upto
   2**(CHAR_BIT * sizeof(size_t) - 1) slots and an open addressing method
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

   The implementation only uses integer and pointer operations. Integer
   arithmetic is used in load factor operations, thereby eliminating the
   use of float. Given parameter values within the specified ranges,
   the implementation provides an error message and an exit is executed
   if an integer overflow is attempted* or an allocation is not completed
   due to insufficient resources. The behavior outside the specified
   parameter ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99 with the only requirement that CHAR_BIT * sizeof(size_t) is
   greater or equal to 16 and is even (every bit is required to participate
   in the value at this time).

   * except intended wrapping around of unsigned integers in modulo
     operations, which is defined, and overflow detection as a part
     of computing bounds, which is defined by the implementation.
*/

#ifndef HT_MULOA_H  
#define HT_MULOA_H

#include <stddef.h>

typedef struct{
  size_t fval; /* first hash value with first bit only set in placeholder */
  size_t sval; /* second hash value */
} ke_t;        /* given char *p pointer to a ke_t,
                  p - key_offset points to key_size block and 
                  p + elt_offset points to elt_size block;
                  see ke_key_ptr and ke_elt_ptr functions */

typedef struct{
  size_t key_size;
  size_t elt_size;
  size_t key_offset;
  size_t elt_offset;
  size_t elt_alignment;
  size_t log_count;
  size_t count;
  size_t max_sum; /* >= 0, < count, represents alpha */
  size_t max_num_probes;
  size_t num_elts;
  size_t num_phs;
  size_t fprime; /* >2**(n - 1), <2**n, n = CHAR_BIT * sizeof(size_t) */
  size_t sprime; /* >2**(n - 1), <2**n, n = CHAR_BIT * sizeof(size_t) */
  size_t alpha_n;
  size_t log_alpha_d;
  ke_t *ph;
  ke_t **key_elts;
  int (*cmp_key)(const void *, const void *);
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
                 load factor upper bound; denominator is a power of two and
                 is greater or equal to alpha_n
    cmp_key     : - if NULL then a default memcmp-based comparison of keys
                 is performed
                 - otherwise comparison function is applied which returns a
                 zero integer value iff the two keys accessed through the
                 first and the second arguments are equal; each argument is
                 a pointer to a key_size block
   rdc_key     : - if NULL then a default conversion of a bit pattern
                 in the block pointed to by key is performed prior to
                 hashing, which may introduce regularities
                 - otherwise rdc_key is applied to a key prior to hashing;
                 the first argument points to a key and the second argument
                 provides the size of the key
   free_elt    : - if an element is within a contiguous memory block and
                 a copy of the element was inserted, then NULL as free_elt
                 is sufficient to delete the element,
                 - if an element is within a noncontiguous memory block or
                 a pointer to a contiguous element was inserted, then an
                 element-specific free_elt, taking a pointer to a pointer to an
                 element as its argument and leaving a block of size elt_size
                 pointed to by the argument, is necessary to delete the element
*/
void ht_muloa_init(ht_muloa_t *ht,
		   size_t key_size,
		   size_t elt_size,
		   size_t min_num,
		   size_t alpha_n,
		   size_t log_alpha_d,
		   int (*cmp_key)(const void *, const void *),
		   size_t (*rdc_key)(const void *, size_t),
		   void (*free_elt)(void *));

/**
   Aligns each in-table elt_size block to be accessible with a pointer to a 
   type T other than character (in addition to a character pointer). If
   alignment requirement of T is unknown, the size of T can be used
   as a value of the alignment parameter because size of T >= alignment
   requirement of T (due to structure of arrays), which may result in
   overalignment. The hash table keeps the effective type of a copied
   elt_size block, if it had one at the time of insertion, and T must
   be compatible with the type to comply with the strict aliasing rules.
   T can be the same or a cvr-qualified/signed/unsigned version of the
   type. The operation is optionally called after ht_muloa_init is
   completed and before any other operation is called.
   ht          : pointer to an initialized ht_muloa_t struct
   alignment   : alignment requirement or size of the type, a pointer to
                 which is used to access an elt_size block
*/
void ht_muloa_align_elt(ht_muloa_t *ht, size_t alignment);

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL and point to blocks of size key_size and
   elt_size respectively.
*/
void ht_muloa_insert(ht_muloa_t *ht, const void *key, const void *elt);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL and points
   to a block of size key_size. The returned pointer can be dereferenced
   according to ht_muloa_init and ht_muloa_align_elt.
*/
void *ht_muloa_search(const ht_muloa_t *ht, const void *key);

/**
   Removes a key and its associated element from a hash table by copying 
   the element or its pointer into a block of size elt_size pointed to
   by elt. If the key is not in the hash table, leaves the block pointed
   to by elt unchanged. The key and elt parameters are not NULL and point
   to blocks of size key_size and elt_size respectively.
*/
void ht_muloa_remove(ht_muloa_t *ht, const void *key, void *elt);

/**
   If a key is in a hash table, deletes the key and its associated element 
   according to free_elt. The key parameter is not NULL and points
   to a block of size key_size.
*/
void ht_muloa_delete(ht_muloa_t *ht, const void *key);

/**
   Frees a hash table and leaves a block of size sizeof(ht_muloa_t)
   pointed to by the ht parameter.
*/
void ht_muloa_free(ht_muloa_t *ht);

/**
   Help construct a hash table parameter value in algorithms and data
   structures with a hash table parameter, complying with the stict aliasing
   rules and compatibility rules for function types. In each case, a
   (qualified) ht_muloa_t *p0 is converted to (qualified) void * and back
   to a (qualified) ht_muloa_t *p1, thus guaranteeing that the value of p0
   equals the value of p1. An initialization helper is constructed by the
   user. 
*/

void ht_muloa_insert_helper(void *ht, const void *key, const void *elt);

void *ht_muloa_search_helper(const void *ht, const void *key);

void ht_muloa_remove_helper(void *ht, const void *key, void *elt);

void ht_muloa_delete_helper(void *ht, const void *key);

void ht_muloa_free_helper(void *ht);

#endif
