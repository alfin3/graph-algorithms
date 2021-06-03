/**
   ht-div-pthread.h

*/

#ifndef HT_DIV_PTHREAD_H  
#define HT_DIV_PTHREAD_H

#include <stddef.h>
#include "dll.h"

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
  size_t num_in_threads; /* # admitted threads through the main gate */
  size_t num_key_locks; /* # locks across key_elts array */
  boolean_t one_in;
  boolean_t all_in;
  pthread_mutex_t in_lock; /* main gate lock */
  pthread_cond_t in_cond; /* main gate condition variable */
  pthread_mutex_t *key_locks;
  int (*is_ins)(const void *, const void *); /* predicate for insert */
} ht_div_pthread_t;

/**
   Initializes a hash table.
*/
void ht_div_pthread_init(ht_div_pthread_t *ht,
			 size_t key_size,
			 size_t elt_size,
			 size_t num_key_locks,
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
