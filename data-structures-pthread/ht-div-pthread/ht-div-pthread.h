/**
   ht-div-pthread.h

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

#ifndef HT_DIV_PTHREAD_H  
#define HT_DIV_PTHREAD_H

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
  void (*free_elt)(void *);

  /* thread synchronization */
  size_t num_elts;
  size_t num_in_threads; /* passed gate_lock's first critical section */
  size_t num_key_locks; /* -> probability of waiting at a slot */
  size_t num_grow_threads;
  boolean_t gate_open;
  pthread_mutex_t gate_lock;
  pthread_mutex_t *key_locks; /* locks, each covering a sector of key_elts */
  pthread_cond_t gate_open_cond;
  pthread_cond_t grow_cond;
  int (*is_ins)(const void *, const void *); /* predicate for insert */
} ht_div_pthread_t;

/**
   Initializes a hash table.
*/
void ht_div_pthread_init(ht_div_pthread_t *ht,
			 size_t key_size,
			 size_t elt_size,
			 size_t num_key_locks,
			 size_t num_grow_threads,
			 float alpha,
			 void (*free_elt)(void *),
			 int (*is_ins)(const void *, const void *));

void ht_div_pthread_insert(ht_div_pthread_t *ht,
			   const void *key,
			   const void *elt);

void *ht_div_pthread_search(const ht_div_pthread_t *ht, const void *key);

void ht_div_pthread_remove(ht_div_pthread_t *ht, const void *key, void *elt);

void ht_div_pthread_delete(ht_div_pthread_t *ht, const void *key);

void ht_div_pthread_free(ht_div_pthread_t *ht);

#endif
