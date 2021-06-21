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
   or equal to 16 and is even, and ii) pthreads API is available.

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
  size_t group_ix;
  size_t count_ix; /* max size_t value if last representable prime reached */
  size_t count;
  size_t num_elts;
  float alpha;
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
  void (*rdc_elt)(void *, const void *, size_t); /* e.g. min, max, add */
  void (*free_elt)(void *);
} ht_divchn_pthread_t;

/**
   Initializes a hash table. The initialization operation is called and
   must return before any thread calls insert, remove, and/or delete,
   or search operation.
   ht               : a pointer to a preallocated block of size
                      sizeof(ht_divchn_pthread_t).
   key_size         : size of a key object
   elt_size         : - size of an element, if the element is within a
                      contiguous memory block and a copy of the element is
                      inserted,
                      - size of a pointer to an element, if the element is
                      within a noncontiguous memory block or a pointer to a
                      contiguous element is inserted
   min_num          : minimum number of keys that are known or expected to
                      become present simultaneously (within a time slice)
                      in a hash table, resulting in a speedup by avoiding
                      unnecessary growth steps of a hash table; 0 if a
                      positive value is not specified and all growth steps
                      are to be completed
   alpha            : > 0.0, a load factor upper bound
   log_num_locks    : log base 2 number of mutex locks for synchronizing
                      insert, remove, and delete operations; a larger number
                      reduces the size of a set of slots that maps to a lock
                      and may reduce the time threads are blocked, depending
                      on the scheduler and at the expense of space
   num_grow_threads : >= 1, number of threads used in growing the hash table
   rdc_elt          : - NULL, if a key is in the hash table when the key is
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
			    float alpha,
			    size_t log_num_locks,
			    size_t num_grow_threads,
			    void (*rdc_elt)(void *, const void *, size_t),
			    void (*free_elt)(void *));

/**
   Inserts a batch of keys and associated elements into a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch. See also the
   specification of rdc_elt in ht_divchn_pthread_init.
*/
void ht_divchn_pthread_insert(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      const void *batch_elts,
			      size_t batch_count);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
   The operation is called before/after all threads started/completed
   insert, remove, and delete operations on ht. This is a non-modifying
   query operation and has no synchronization overhead.
*/
void *ht_divchn_pthread_search(const ht_divchn_pthread_t *ht,
			       const void *key);

/**
   Removes a batch of keys and associated elements from a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch.
*/
void ht_divchn_pthread_remove(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      void *batch_elts,
			      size_t batch_count);

/**
   Deletes a batch of keys and associated elements from a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch.
*/
void ht_divchn_pthread_delete(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      size_t batch_count);

/**
   Frees a hash table. The operation is called after all threads completed
   insert, remove, delete, and search operations.
*/
void ht_divchn_pthread_free(ht_divchn_pthread_t *ht);

#endif
