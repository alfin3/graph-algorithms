/**
   ht-divchn-pthread.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic hash keys and generic elements that is concurrently
   accessible and modifiable.

   The implementation is based on a division method for hashing into upto  
   the number of slots determined by the largest prime number in the
   C_PRIME_PARTS array, representable as size_t on a given system, and a
   chaining method for resolving collisions. 

   A hash key is an object within a contiguous block of memory (e.g. a basic 
   type, array, struct). An element is within a contiguous or noncontiguous
   memory block.

   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by the 
   alpha parameter. The alpha parameter does not provide an upper bound 
   after the maximum representable count of slots in a hash table is
   reached.

   A hash table is modified by threads calling insert, remove, and/or delete
   operations concurrently. The design provides the following guarantees
   with respect to the final state of a hash table, defined as a pair of
   i) a load factor, and ii) the set of sets of key-element pairs for each
   slot of the hash table, after all operations are completed:
     - a single final state is guaranteed with respect to concurrent insert,
     remove, and/or delete operations if the sets of keys used by threads
     are disjoint,
     - if insert operations are called by more than one thread concurrently
     and the sets of keys used by threads are not disjoint, then a single
     final state of the hash table is guaranteed according to a user-defined
     reduction function (e.g. min, max, add, multiply, and, or, xor of key-
     associated elements),
     - because chaining does not limit the number of insertions, each thread
     is theoretically guaranteed to complete its batch operation i) before a
     load factor upper bound (alpha) is exceeded, ii) before the hash table
     grows as a part of a called insert operation when alpha is temporarily*
     exceeded**, or iii) after the hash table reaches its maximum count of
     slots on a given system and alpha no longer bounds the load factor.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99. The requirements are: i) CHAR_BIT * sizeof(size_t) is greater
   or equal to 16 and is even (every bit is required to participate
   in the value at this time), and ii) pthreads API is available.

   * unless the growth step that follows does not lower the load factor
   below alpha because the maximum count of slots is reached during the
   growth step, in which case alpha no longer bounds the load factor

   ** to the extent dependent on the number of threads that passed the
   first critical section and their batch sizes
*/

#ifndef HT_DIVCHN_PTHREAD_H  
#define HT_DIVCHN_PTHREAD_H

#define _XOPEN_SOURCE 600

#include <stddef.h>
#include <pthread.h>
#include "dll.h"

typedef enum{FALSE, TRUE} boolean_t;

typedef struct{
  /* hash table */
  size_t key_size;
  size_t elt_size;
  size_t elt_alignment;
  size_t group_ix;
  size_t count_ix; /* max size_t value if last representable prime reached */
  size_t count;
  size_t max_num_elts; /*  >= 0, <= C_SIZE_MAX, represents alpha */
  size_t num_elts;
  size_t alpha_n;
  size_t log_alpha_d;
  dll_t *ll;
  dll_node_t **key_elts; /* array of pointers to nodes */

  /* thread synchronization */
  size_t num_in_threads; /* passed gate_lock's first critical section */
  size_t num_grow_threads;
  size_t key_locks_mask; /* -> probability of waiting at a slot */
  boolean_t gate_open;
  pthread_mutex_t gate_lock;
  pthread_mutex_t *key_locks; /* locks, each covering a subset of slots */
  pthread_cond_t gate_open_cond;
  pthread_cond_t grow_cond;

  /* function pointers */
  int (*cmp_key)(const void *, const void *);
  size_t (*rdc_key)(const void *, size_t);
  void (*rdc_elts)(void *, const void *, size_t); /* e.g. min, max, add */
  void (*free_elt)(void *);
} ht_divchn_pthread_t;

/**
   Initializes a hash table. The initialization operation is called and
   must return before any thread calls insert, remove, and/or delete,
   or search operation.
   ht               : a pointer to a preallocated block of size
                      sizeof(ht_divchn_pthread_t).
   key_size         : non-zero size of a key object
   elt_size         : - non-zero size of an element, if the element is
                      within a contiguous memory block and a copy of the
                      element is inserted,
                      - size of a pointer to an element, if the element
                      is within a noncontiguous memory block or a pointer to
                      a contiguous element is inserted
   min_num          : minimum number of keys that are known or expected to
                      become present simultaneously in a hash table,
                      resulting in a speedup by avoiding unnecessary growth
                      steps of a hash table; 0 if a positive value is not
                      specified and all growth steps are to be completed
   alpha_n           : > 0 numerator of load factor upper bound
   log_alpha_d      : < CHAR_BIT * sizeof(size_t) log base 2 of denominator
                      of load factor upper bound; denominator is a power of
                      two
   log_num_locks    : log base 2 number of mutex locks for synchronizing
                      insert, remove, and delete operations; a larger number
                      reduces the size of a set of slots that maps to a lock
                      and may reduce the time threads are blocked, depending
                      on the scheduler and at the expense of space
   num_grow_threads : >= 1, number of threads used in growing the hash table
   cmp_key          : - if NULL then a default memcmp-based comparison of 
                      keys is performed
                      - otherwise comparison function is applied which
                      returns a zero integer value iff the two keys accessed
                      through the first and the second arguments are equal;
                      each argument is a pointer to a key_size block
   rdc_key          : - if NULL then a default conversion of a bit pattern
                      in the block pointed to by key is performed prior to
                      hashing, which may introduce regularities
                      - otherwise rdc_key is applied to a key prior to
                      hashing; the first argument points to a key and the
                      second argument provides the size of the key
   rdc_elts         : - NULL, if a key is in the hash table when the key is
                      inserted, the key-associated element in the hash table
                      is updated to the inserted element
                      - non-NULL, if a key is in the hash table when the
                      key is inserted, performs a reduction of the element
                      already in the hash table and the inserted element;
                      the result of the reduction is associated with the key
                      in the hash table; the first argument points to an
                      elt_size-sized block in the hash table, the second
                      argument points to an elt_size-sized block of the
                      inserted element, and the third argument is equal to a
                      elt_size value
   free_elt         : - if an element is within a contiguous memory block and
                      a copy of the element was inserted, then NULL as
                      free_elt is sufficient to delete the element,
                      - if an element is within a noncontiguous memory block
                      or a pointer to a contiguous element was inserted, then
                      an element-specific free_elt, taking a pointer to a
                      pointer to an element as its argument and leaving a
                      block of size elt_size pointed to by the argument, is
                      necessary to delete the element
*/
void ht_divchn_pthread_init(ht_divchn_pthread_t *ht,
			    size_t key_size,
			    size_t elt_size,
			    size_t min_num,
			    size_t alpha_n,
			    size_t log_alpha_d,
			    size_t log_num_locks,
			    size_t num_grow_threads,
			    int (*cmp_key)(const void *, const void *),
			    size_t (*rdc_key)(const void *, size_t),
			    void (*rdc_elts)(void *, const void *, size_t),
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
   type. The operation is optionally called after ht_divchn_pthread_init is
   completed and before any other operation is called.
   ht          : pointer to an initialized ht_divchn_pthread_t struct
   alignment   : alignment requirement or size of the type, a pointer to
                 which is used to access an elt_size block
*/
void ht_divchn_pthread_align_elt(ht_divchn_pthread_t *ht, size_t alignment);

/**
   Inserts a batch of keys and associated elements into a hash table.
   The batch_keys and batch_elts parameters are not NULL and point to
   arrays of blocks of size key_size and elt_size respectively. The
   batch_count parameter is the count of keys in a batch. See also the
   specification of rdc_elts in ht_divchn_pthread_init.
*/
void ht_divchn_pthread_insert(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      const void *batch_elts,
			      size_t batch_count);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
   The operation is called before/after all threads started/completed
   insert, remove, and delete operations on ht and does not require
   thread synchronization overhead.
*/
void *ht_divchn_pthread_search(const ht_divchn_pthread_t *ht,
			       const void *key);

/**
   Removes a batch of keys and associated elements from a hash table by
   copying the elements or its pointers into the array of elt_size blocks
   pointed to by batch_elts. If a key is not in the hash table, leaves the
   corresponding elt_size block unchanged. The batch_keys and batch_elts
   parameters are not NULL and point to arrays of blocks of size key_size and
   elt_size respectively. The batch_count parameter is the count of keys in a
   batch.
*/
void ht_divchn_pthread_remove(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      void *batch_elts,
			      size_t batch_count);

/**
   Deletes a batch of keys and associated elements from a hash table.
   If a key is not in the hash table, no operation with respect to the key
   is performed. The batch_keys parameter is not NULL and points to an array
   of blocks of size key_size. The batch_count parameter is the count of keys
   in a batch.
*/
void ht_divchn_pthread_delete(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      size_t batch_count);

/**
   Frees a hash table. The operation is called after all threads completed
   insert, remove, delete, and search operations. Leaves a block of size
   sizeof(ht_divchn_pthread_t) pointed to by the ht parameter.
*/
void ht_divchn_pthread_free(ht_divchn_pthread_t *ht);

/**
   Help construct a hash table parameter value in algorithms and data
   structures with a hash table parameter, complying with the stict aliasing
   rules and compatibility rules for function types. In each case, a
   (qualified) ht_divchn_pthread_t *p0 is converted to (qualified) void * 
   and back to a (qualified) ht_divchn_ptrhead_t *p1, thus guaranteeing that
   the value of p0 equals the value of p1. An initialization helper is
   constructed by the user. 
*/

void ht_divchn_pthread_align_elt_helper(void *ht, size_t alignment);

void ht_divchn_pthread_insert_helper(void *ht,
				     const void *batch_keys,
				     const void *batch_elts,
				     size_t batch_count);

void *ht_divchn_pthread_search_helper(const void *ht,
				      const void *key);

void ht_divchn_pthread_remove_helper(void *ht,
				     const void *batch_keys,
				     void *batch_elts,
				     size_t batch_count);

void ht_divchn_pthread_delete_helper(void *ht,
				     const void *batch_keys,
				     size_t batch_count);

void ht_divchn_pthread_free_helper(void *ht);

#endif
