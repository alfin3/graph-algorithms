/**
   ht-muloa.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic contiguous or non-contiguous keys and generic
   contiguous or non-contiguous elements.
   
   The implementation is based on a multiplication method for hashing into
   upto 2**(size_t width - 1) slots and an open addressing method with
   double hashing for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by 
   the alpha parameters. The expected number of probes in a search is 
   upper-bounded by 1/(1 - load factor bound), under the uniform hashing
   assumption. 

   The alpha parameters do not provide an upper bound after the maximum 
   count of slots in a hash table is reached. After exceeding the load
   factor bound, the load factor is <= 1.0 due to open addressing, and the
   expected number of probes is upper-bounded by 1/(1 - load factor) before
   the full occupancy is reached.

   A distinction is made between a key and a "key_size block", and an
   element and an "elt_size block". During an insertion, a contiguous block
   of size key_size ("key_size block") and a contiguous block of size
   elt_size ("elt_size block") are copied into a hash table. A key may be
   within a contiguous or non-contiguous memory block. Given a key, the user
   decides what is copied into the key_size block of the hash table. If the
   key is within a contiguous memory block, then it can be entirely copied
   as a key_size block, or a pointer to it can be copied as a key_size
   block. If the key is within a non-contiguous memory block, then a pointer
   to it is copied as a key_size block. The same applies to an element. 

   When a pointer to a key is copied into a hash table as a key_size block,
   the user can also decide if only the pointer or the entire key is deleted
   during the delete and free operations. By setting free_key to NULL, only
   the pointer is deleted. Otherwise, the deletion is performed according to
   a non-NULL free_key. For example, when an in-memory set of images are
   used as keys (e.g. with a subset of bits in each image used for hashing)
   and pointers are copied into a hash table, then setting free_key to NULL
   will not affect the original set of images throughout the lifetime of the
   hash table. The same applies to elements and free_elt.

   The implementation only uses integer and pointer operations. Integer
   arithmetic is used in load factor operations, thereby eliminating the
   use of float. Given parameter values within the specified ranges,
   the implementation provides an error message and an exit is executed
   if an integer overflow is attempted* or an allocation is not completed
   due to insufficient resources. The behavior outside the specified
   parameter ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99 with the only requirement that the width of size_t is
   greater or equal to 16, less than 2040, and is even.

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
  size_t alpha_n;
  size_t log_alpha_d;
  size_t max_sum; /* >= 0, < count, represents load factor upper bound */
  size_t max_num_probes;
  size_t num_elts;
  size_t num_phs;
  size_t fprime; /* >2**(n - 1), <2**n, where n is size_t width */
  size_t sprime; /* >2**(n - 1), <2**n, where n is size_t width */
  ke_t *ph;
  ke_t **key_elts;
  int (*cmp_key)(const void *, const void *);
  size_t (*rdc_key)(const void *);
  void (*free_key)(void *);
  void (*free_elt)(void *);
} ht_muloa_t;

/**
   Initializes a hash table. An in-table elt_size block is guaranteed to
   be accessible only with a pointer to a character, unless additional
   alignment is performed by calling ht_divchn_align.
   ht          : a pointer to a preallocated block of size 
                 sizeof(ht_muloa_t).
   key_size    : non-zero size of a key_size block; must account for internal
                 and trailing padding according to sizeof
   elt_size    : non-zero size of an elt_size block; must account for internal
                 and trailing padding according to sizeof
   min_num     : minimum number of keys that are known to be or expected to
                 be present simultaneously in a hash table; results in a
                 speedup by avoiding unnecessary growth steps of a hash
                 table; 0 if a positive value is not specified and all
                 growth steps are to be completed
   alpha_n     : > 0 numerator of a load factor upper bound
   log_alpha_d : < size_t width; log base 2 of the denominator of the load
                 factor upper bound; the denominator is a power of two and
                 is greater or equal to alpha_n
   cmp_key     : - if NULL then a default memcmp-based comparison of key_size
                 blocks of keys is performed
                 - otherwise comparison function is applied which returns a
                 zero integer value iff the two keys accessed through the
                 first and the second arguments are equal; each argument is
                 a pointer to the key_size block of a key; cmp_key must use
                 the same subset of bits in a key as rdc_key
   rdc_key     : - if NULL then a default conversion of a bit pattern
                 in the key_size block of a key is performed prior to
                 hashing, which may introduce regularities
                 - otherwise rdc_key is applied to a key to reduce the key
                 to a size_t integer value prior to hashing; the argument
                 points to the key_size block of a key; rdc_key must use
                 the same subset of bits in a key as cmp_key
   free_key    : - NULL if only key_size blocks should be deleted throughout
                 the lifetime of the hash table (e.g. because keys were
                 entirely copied as key_size blocks, or because pointers
                 were copied as key_size blocks and only pointers should
                 be deleted)
                 - otherwise takes a pointer to the key_size block of a key
                 as an argument, frees the memory of the key except the
                 key_size block pointed to by the argument
   free_elt    : - NULL if only elt_size blocks should be deleted throughout
                 the lifetime of the hash table (e.g. because elements were
                 entirely copied as elt_size blocks, or because pointers
                 were copied as elt_size blocks and only pointers should
                 be deleted)
                 - otherwise takes a pointer to the elt_size block of an
                 element as an argument, frees the memory of the element
                 except the elt_size block pointed to by the argument
*/
void ht_muloa_init(ht_muloa_t *ht,
		   size_t key_size,
		   size_t elt_size,
		   size_t min_num,
		   size_t alpha_n,
		   size_t log_alpha_d,
		   int (*cmp_key)(const void *, const void *),
		   size_t (*rdc_key)(const void *),
		   void (*free_key)(void *),
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
   ht            : pointer to an initialized ht_muloa_t struct
   elt_alignment : alignment requirement or size of the type, a pointer to
                   which is used to access the elt_size block of an element
                   in a hash table; if size, must account for internal
                   and trailing padding according to sizeof
*/
void ht_muloa_align(ht_muloa_t *ht, size_t elt_alignment);

/**
   Inserts a key and an associated element into a hash table by copying
   the corresponding key_size and elt_size blocks. If the key pointed to by
   the key parameter is already in the hash table according to cmp_key,
   then deletes the previous element according to free_elt and copies
   the elt_size block pointed to by the elt parameter.
   ht          : pointer to an initialized ht_muloa_t struct   
   key         : non-NULL pointer to the key_size block of a key
   elt         : non-NULL pointer to the elt_size block of an element
*/
void ht_muloa_insert(ht_muloa_t *ht, const void *key, const void *elt);

/**
   If a key is present in a hash table, according to cmp_key, then returns a
   pointer to the elt_size block of its associated element in the hash table.
   Otherwise returns NULL. The returned pointer can be dereferenced according
   to the preceding calls to ht_muloa_init and ht_muloa_align_elt.
   ht          : pointer to an initialized ht_muloa_t struct   
   key         : non-NULL pointer to the key_size block of a key
*/
void *ht_muloa_search(const ht_muloa_t *ht, const void *key);

/**
   Removes the element associated with a key in a hash table that equals to
   the key pointed to by the key parameter according to cmp_key, by a)
   copying the elt_size block of the element to the elt_size block pointed
   to by the elt parameter and b) deleting the corresponding key_size and
   elt_size blocks in the hash table. If there is no matching key in the
   hash table according to cmp_key, leaves the hash table and the block
   pointed to by elt unchanged.
   ht          : pointer to an initialized ht_muloa_t struct   
   key         : non-NULL pointer to the key_size block of a key
   elt         : non-NULL pointer to a preallocated elt_size block
*/
void ht_muloa_remove(ht_muloa_t *ht, const void *key, void *elt);

/**
   If there is a key in a hash table that equals to the key pointed to
   by the key parameter according to cmp_key, then deletes the in-table key
   element pair according to free_key and free_elt.
   ht          : pointer to an initialized ht_muloa_t struct   
   key         : non-NULL pointer to the key_size block of a key
*/
void ht_muloa_delete(ht_muloa_t *ht, const void *key);

/**
   Frees the memory of all keys and elements that are in a hash table
   according to free_key and free_elt, frees the memory of the hash table,
   and leaves the block of size sizeof(ht_muloa_t) pointed to by the ht
   parameter.
*/
void ht_muloa_free(ht_muloa_t *ht);

/**
   Help construct a hash table parameter value in algorithms and data
   structures with a hash table parameter, complying with the stict aliasing
   rules and compatibility rules for function types. In each case, a
   (qualified) ht_muloa_t *p0 is converted to (qualified) void * and back
   to a (qualified) ht_muloa_t *p1, thus guaranteeing that the value of p0
   equals the value of p1.
*/

void ht_muloa_init_helper(void *ht,
			  size_t key_size,
			  size_t elt_size,
			  size_t min_num,
			  size_t alpha_n,
			  size_t log_alpha_d,
			  int (*cmp_key)(const void *, const void *),
			  size_t (*rdc_key)(const void *),
			  void (*free_key)(void *),
			  void (*free_elt)(void *));

void ht_muloa_align_helper(void *ht, size_t elt_alignment);

void ht_muloa_insert_helper(void *ht, const void *key, const void *elt);

void *ht_muloa_search_helper(const void *ht, const void *key);

void ht_muloa_remove_helper(void *ht, const void *key, void *elt);

void ht_muloa_delete_helper(void *ht, const void *key);

void ht_muloa_free_helper(void *ht);

#endif
