/**
   ht-muloa-pthread.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic hash keys and generic elements that is concurrently
   accessible and modifiable.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99. The requirements are: i) CHAR_BIT * sizeof(size_t) is greater
   or equal to 16 and is even, and ii) pthreads API is available.
*/

#ifndef HT_MULOA_PTHREAD_H  
#define HT_MULOA_PTHREAD_H

#define _XOPEN_SOURCE 600

#include <stddef.h>
#include <pthread.h>
#include "dll.h"

typedef enum{FALSE, TRUE} boolean_t;

typedef struct{
  boolean_t is_ph;
  size_t fval;
  size_t sval;
  void *key;
  void *elt;
} key_elt_t; /* first and second hash values, key and element pointers */

typedef struct{
  /* hash table */
  size_t key_size;
  size_t elt_size;
  size_t log_count;
  size_t count;
  size_t max_count;
  size_t max_num_probes;
  size_t num_elts;
  size_t num_phs;
  size_t fprime; /* >2**(n - 1), <2**n, n = CHAR_BIT * sizeof(size_t) */
  size_t sprime; /* >2**(n - 1), <2**n, n = CHAR_BIT * sizeof(size_t) */
  float alpha;
  key_elt_t *ph;
  key_elt_t **key_elts;

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
  size_t (*rdc_key)(const void *, size_t);
  void (*rdc_elt)(void *, const void *, size_t); /* e.g. min, max, add */
  void (*free_elt)(void *);
} ht_muloa_pthread_t;

/**
   Initializes a hash table. The initialization operation is called and
   must return before any thread calls insert, remove, and/or delete,
   or search operation.
*/
void ht_muloa_pthread_init(ht_muloa_pthread_t *ht,
			   size_t key_size,
			   size_t elt_size,
			   size_t min_num,
			   float alpha,
			   size_t log_num_locks,
			   size_t num_grow_threads,
			   size_t (*rdc_key)(const void *, size_t),
			   void (*rdc_elt)(void *, const void *, size_t),
			   void (*free_elt)(void *));

/**
   Inserts a batch of keys and associated elements into a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch. See also the
   specification of rdc_elt in ht_muloa_pthread_init.
*/
void ht_muloa_pthread_insert(ht_muloa_pthread_t *ht,
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
void *ht_muloa_pthread_search(const ht_muloa_pthread_t *ht,
			      const void *key);

/**
   Removes a batch of keys and associated elements from a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch.
*/
void ht_muloa_pthread_remove(ht_muloa_pthread_t *ht,
			     const void *batch_keys,
			     void *batch_elts,
			     size_t batch_count);

/**
   Deletes a batch of keys and associated elements from a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch.
*/
void ht_muloa_pthread_delete(ht_muloa_pthread_t *ht,
			     const void *batch_keys,
			     size_t batch_count);

/**
   Frees a hash table. The operation is called after all threads completed
   insert, remove, delete, and search operations.
*/
void ht_muloa_pthread_free(ht_muloa_pthread_t *ht);

#endif
