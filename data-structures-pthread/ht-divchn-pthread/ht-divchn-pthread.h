/**
   ht-divchn-pthread.h

   The hash table does not spawn new threads, except during doubling when
   all but one user threads are blocked.
   
   Hash table state guarantees:
     - the final state of the hash table is guaranteed with respect to
       concurrent insert, remove, and/or delete ops if there is no key
       overlap between threads
     - if insert ops are called by more than one thread and keys overlap,
       then the implementation guarantees the final hash table state
       according to the insertion predicate (e.g. element max, or
       element min), unless it is NULL
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
  void (*ins_elt)(void *, const void *, size_t); /* e.g. min, conditional */
  void (*free_elt)(void *);
} ht_divchn_pthread_t;

/**
   Initializes a hash table.
*/
void ht_divchn_pthread_init(ht_divchn_pthread_t *ht,
			    size_t key_size,
			    size_t elt_size,
			    size_t log_num_key_locks,
			    size_t num_grow_threads,
			    float alpha,
			    void (*ins_elt)(void *, const void *, size_t),
			    void (*free_elt)(void *));

void ht_divchn_pthread_insert(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      const void *batch_elts,
			      size_t batch_count);

void *ht_divchn_pthread_search(const ht_divchn_pthread_t *ht,
			       const void *key);

void ht_divchn_pthread_remove(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      void *batch_elts,
			      size_t batch_count);

void ht_divchn_pthread_delete(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      size_t batch_count);

void ht_divchn_pthread_free(ht_divchn_pthread_t *ht);

#endif
