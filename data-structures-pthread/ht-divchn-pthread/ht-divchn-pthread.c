/**
   ht-divchn-pthread.c

   A hash table with generic hash keys and generic elements that is
   concurrently accessible and modifiable.
   
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

#define _XOPEN_SOURCE 600

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include "ht-divchn-pthread.h"
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-pthread.h"

/**
   An array of primes in the increasing order, approximately doubling in 
   magnitude, that are not too close to the powers of 2 and 10 to avoid 
   hashing regularities due to the structure of data.
*/
static const size_t C_PRIME_PARTS[6 * 1 + 16 * (2 + 3 + 4)] =
  {0x0607u,                               /* 1543 */
   0x0c2fu,                               /* 3119 */
   0x1843u,                               /* 6211 */
   0x3037u,                               /* 12343 */
   0x5dadu,                               /* 23981 */
   0xbe21u,                               /* 48673 */
   0x5b0bu, 0x0001u,                      /* 88843 */
   0xd8d5u, 0x0002u,                      /* 186581 */
   0xc219u, 0x0005u,                      /* 377369 */
   0x0077u, 0x000cu,                      /* 786551 */
   0xa243u, 0x0016u,                      /* 1483331 */
   0x2029u, 0x0031u,                      /* 3219497 */
   0xcc21u, 0x005fu,                      /* 6278177 */
   0x5427u, 0x00bfu,                      /* 12538919 */
   0x037fu, 0x0180u,                      /* 25166719 */
   0x42bbu, 0x030fu,                      /* 51331771 */
   0x1c75u, 0x06b7u,                      /* 112663669 */
   0x96adu, 0x0c98u,                      /* 211326637 */
   0x96b7u, 0x1898u,                      /* 412653239 */
   0xc10fu, 0x2ecfu,                      /* 785367311 */
   0x425bu, 0x600fu,                      /* 1611612763 */
   0x0007u, 0xc000u,                      /* 3221225479 */
   0x016fu, 0x8000u, 0x0001u,             /* 6442451311 */
   0x9345u, 0xffc8u, 0x0002u,             /* 12881269573 */
   0x5523u, 0xf272u, 0x0005u,             /* 25542415651 */
   0x1575u, 0x0a63u, 0x000cu,             /* 51713873269 */
   0x22fbu, 0xca07u, 0x001bu,             /* 119353582331 */
   0xc513u, 0x4d6bu, 0x0031u,             /* 211752305939 */
   0xa6cdu, 0x50f3u, 0x0061u,             /* 417969972941 */
   0xa021u, 0x5460u, 0x00beu,             /* 817459404833 */
   0xea29u, 0x7882u, 0x0179u,             /* 1621224516137 */
   0xeaafu, 0x7c3du, 0x02f5u,             /* 3253374675631 */
   0xab5fu, 0x5a69u, 0x05ffu,             /* 6594291673951 */
   0x6b1fu, 0x29efu, 0x0c24u,             /* 13349461912351 */
   0xc81bu, 0x35a7u, 0x17feu,             /* 26380589320219 */
   0x57b7u, 0xccbeu, 0x2ffbu,             /* 52758518323127 */
   0xc8fbu, 0x1da8u, 0x6bf3u,             /* 118691918825723 */
   0x82c3u, 0x2c9fu, 0xc2ccu,             /* 214182177768131 */
   0x3233u, 0x1c54u, 0x7d40u, 0x0001u,    /* 419189283369523 */
   0x60adu, 0x46a1u, 0xf55eu, 0x0002u,    /* 832735214133421 */
   0x6babu, 0x40c4u, 0xf12au, 0x0005u,    /* 1672538661088171 */
   0xb24du, 0x6765u, 0x38b5u, 0x000bu,    /* 3158576518771277 */
   0x789fu, 0xfd94u, 0xc6b2u, 0x0017u,    /* 6692396525189279 */
   0x0d35u, 0x5443u, 0xff54u, 0x0030u,    /* 13791536538127669 */
   0x2465u, 0x74f9u, 0x42d1u, 0x005eu,    /* 26532115188884581 */
   0xd017u, 0x90c7u, 0x37b3u, 0x00c6u,    /* 55793289756397591 */
   0x5055u, 0x5a82u, 0x64dfu, 0x0193u,    /* 113545326073368661 */
   0x6f8fu, 0x423bu, 0x8949u, 0x0304u,    /* 217449629757435791 */
   0xd627u, 0x08e0u, 0x0b2fu, 0x05feu,    /* 431794910914467367 */
   0xbbc1u, 0x662cu, 0x4d90u, 0x0badu,    /* 841413987972987841 */
   0xf7d3u, 0x45a1u, 0x8ccbu, 0x185du,    /* 1755714234418853843 */
   0xc647u, 0x3c91u, 0x46b2u, 0x2e9bu,    /* 3358355678469146183 */
   0x58a1u, 0xbd96u, 0x2836u, 0x5f8cu,    /* 6884922145916737697 */
   0x8969u, 0x4c70u, 0x6dbeu, 0xdad8u};   /* 15769474759331449193 */

static const size_t C_PRIME_PARTS_COUNT = 6 + 16 * (2 + 3 + 4);
static const size_t C_PARTS_PER_PRIME[4] = {1, 2, 3, 4};
static const size_t C_PARTS_ACC_COUNTS[4] = {6,
					     6 + 16 * 2,
					     6 + 16 * (2 + 3),
					     6 + 16 * (2 + 3 + 4)};
static const size_t C_BUILD_SHIFT = 16;
static const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);
static const size_t C_SIZE_MAX = (size_t)-1;

static size_t hash(const ht_divchn_pthread_t *ht, const void *key);
static void ht_grow(ht_divchn_pthread_t *ht);
static int is_overflow(size_t start, size_t count);
static size_t build_prime(size_t start, size_t count);
static void *ptr(const void *block, size_t i, size_t size);

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
			    void (*free_elt)(void *)){
  size_t i;
  size_t key_locks_count;
  /* hash table */
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->group_ix = 0;
  ht->count_ix = 0;
  ht->count = build_prime(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix]);
  while ((float)min_num / ht->count > alpha){
    ht->count_ix += C_PARTS_PER_PRIME[ht->group_ix];
    if (ht->count_ix == C_PARTS_ACC_COUNTS[ht->group_ix]) ht->group_ix++;
    if (ht->count_ix == C_PRIME_PARTS_COUNT){
      /* the largest prime in C_PRIME_PARTS built */
      break;
    }else if (is_overflow(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix])){
      /* the largest representable prime in C_PRIME_PARTS built */
      ht->count_ix = C_SIZE_MAX;
      break;
    }else{
      ht->count = build_prime(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix]);
    }
  }
  ht->num_elts = 0;
  ht->alpha = alpha;
  ht->key_elts = malloc_perror(ht->count, sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    dll_init(&ht->key_elts[i]);
  }
  /* thread synchronization */
  ht->num_in_threads = 0;
  ht->num_grow_threads = num_grow_threads;
  key_locks_count = pow_two_perror(log_num_locks);
  ht->key_locks_mask = C_SIZE_MAX & (key_locks_count - 1);
  ht->gate_open = TRUE;
  mutex_init_perror(&ht->gate_lock);
  ht->key_locks = malloc_perror(key_locks_count,
				sizeof(pthread_mutex_t));
  for (i = 0; i < key_locks_count; i++){
    mutex_init_perror(&ht->key_locks[i]);
  }
  cond_init_perror(&ht->gate_open_cond);
  cond_init_perror(&ht->grow_cond);
  /* function pointers */
  ht->rdc_elt = rdc_elt;
  ht->free_elt = free_elt;
}

/**
   Inserts a batch of keys and associated elements into a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch. See also the
   specification of rdc_elt in ht_divchn_pthread_init.
*/
void ht_divchn_pthread_insert(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      const void *batch_elts,
			      size_t batch_count){
  size_t i, ix, lock_ix;
  size_t increased = 0;
  dll_node_t **head = NULL, *node = NULL;
  /* first critical section : go through gate or wait */
  mutex_lock_perror(&ht->gate_lock);
  while (!ht->gate_open){
    cond_wait_perror(&ht->gate_open_cond, &ht->gate_lock);
  }
  ht->num_in_threads++;
  mutex_unlock_perror(&ht->gate_lock);

  /* insert */
  for (i = 0; i < batch_count; i++){
    ix = hash(ht, ptr(batch_keys, i, ht->key_size));
    head = &ht->key_elts[ix];
    lock_ix = ix & ht->key_locks_mask;
    mutex_lock_perror(&ht->key_locks[lock_ix]);
    node = dll_search_key(head,
			  ptr(batch_keys, i, ht->key_size),
			  ht->key_size);
    if (node == NULL){
      dll_prepend_new(head,
		      ptr(batch_keys, i, ht->key_size),
		      ptr(batch_elts, i, ht->elt_size),
		      ht->key_size,
		      ht->elt_size);
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
      increased++;
    }else{
      if (ht->rdc_elt != NULL){
	ht->rdc_elt(node->elt,
		    ptr(batch_elts, i, ht->elt_size),
		    ht->elt_size);
      }else{
	if (ht->free_elt != NULL) ht->free_elt(node->elt);
	memcpy(node->elt, ptr(batch_elts, i, ht->elt_size), ht->elt_size);
      }
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
    }
  }

  /* grow ht if needed, and finish */
  if (ht->count_ix != C_SIZE_MAX &&
      ht->count_ix != C_PRIME_PARTS_COUNT){
    mutex_lock_perror(&ht->gate_lock);
    ht->num_elts += increased;
    if ((float)ht->num_elts / ht->count > ht->alpha && ht->gate_open){
      ht->gate_open = FALSE;
      /* wait for threads that passed first critical section to finish */
      while (ht->num_in_threads > 1){
	cond_wait_perror(&ht->grow_cond, &ht->gate_lock);
      }
      mutex_unlock_perror(&ht->gate_lock);
      ht_grow(ht); /* single thread; num_elts can be used without lock */
      mutex_lock_perror(&ht->gate_lock);
      ht->gate_open = TRUE;
      cond_broadcast_perror(&ht->gate_open_cond);
    }else{
      if (!ht->gate_open) cond_signal_perror(&ht->grow_cond);
    }
    ht->num_in_threads--;
    mutex_unlock_perror(&ht->gate_lock);
  }else{
    mutex_lock_perror(&ht->gate_lock);
    ht->num_elts += increased;
    ht->num_in_threads--;
    mutex_unlock_perror(&ht->gate_lock);
  }
}

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
   The operation is called before/after all threads started/completed
   insert, remove, and delete operations on ht. This is a non-modifying
   query operation and has no synchronization overhead.
*/
void *ht_divchn_pthread_search(const ht_divchn_pthread_t *ht,
			       const void *key){
  dll_node_t *node = dll_search_key(&ht->key_elts[hash(ht, key)],
				     key,
				     ht->key_size);
  if (node == NULL){
    return NULL;
  }else{
    return node->elt;
  }
}

/**
   Removes a batch of keys and associated elements from a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch.
*/
void ht_divchn_pthread_remove(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      void *batch_elts,
			      size_t batch_count){
  size_t i, ix, lock_ix;
  size_t removed = 0;
  dll_node_t **head = NULL, *node = NULL;
  /* first critical section : go through gate or wait */
  mutex_lock_perror(&ht->gate_lock);
  while (!ht->gate_open){
    cond_wait_perror(&ht->gate_open_cond, &ht->gate_lock);
  }
  ht->num_in_threads++;
  mutex_unlock_perror(&ht->gate_lock);
  /* remove */
  for (i = 0; i < batch_count; i++){
    ix = hash(ht, ptr(batch_keys, i, ht->key_size));
    head = &ht->key_elts[ix];
    lock_ix = ix & ht->key_locks_mask;
    mutex_lock_perror(&ht->key_locks[lock_ix]);
    node = dll_search_key(head,
			  ptr(batch_keys, i, ht->key_size),
			  ht->key_size);
    if (node != NULL){
      memcpy(ptr(batch_elts, i, ht->elt_size), node->elt, ht->elt_size);
      /* if an element is noncontiguous, only the pointer to it is deleted */
      dll_delete(head, node, NULL);
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
      removed++;
    }else{
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
    }
  }
  /* finish */
  mutex_lock_perror(&ht->gate_lock);
  ht->num_elts -= removed;
  ht->num_in_threads--;
  if (!ht->gate_open) cond_signal_perror(&ht->grow_cond);
  mutex_unlock_perror(&ht->gate_lock);
}

/**
   Deletes a batch of keys and associated elements from a hash table.
   The batch_keys and batch_elts parameters are not NULL. The
   batch_count parameter is the count of keys in a batch.
*/
void ht_divchn_pthread_delete(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      size_t batch_count){
  size_t i, ix, lock_ix;
  size_t deleted = 0;
  dll_node_t **head = NULL, *node = NULL;
  /* first critical section : go through gate or wait */
  mutex_lock_perror(&ht->gate_lock);
  while (!ht->gate_open){
    cond_wait_perror(&ht->gate_open_cond, &ht->gate_lock);
  }
  ht->num_in_threads++;
  mutex_unlock_perror(&ht->gate_lock);
  /* delete */
  for (i = 0; i < batch_count; i++){
    ix = hash(ht, ptr(batch_keys, i, ht->key_size));
    head = &ht->key_elts[ix];
    lock_ix = ix & ht->key_locks_mask;
    mutex_lock_perror(&ht->key_locks[lock_ix]);
    node = dll_search_key(head,
			  ptr(batch_keys, i, ht->key_size),
			  ht->key_size);
    if (node != NULL){
      dll_delete(head, node, ht->free_elt);
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
      deleted++;
    }else{
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
    }
  }
  /* finish */
  mutex_lock_perror(&ht->gate_lock);
  ht->num_elts -= deleted;
  ht->num_in_threads--;
  if (!ht->gate_open) cond_signal_perror(&ht->grow_cond);
  mutex_unlock_perror(&ht->gate_lock);
}

/**
   Frees a hash table. The operation is called after all threads completed
   insert, remove, delete, and search operations.
*/
void ht_divchn_pthread_free(ht_divchn_pthread_t *ht){
  size_t i;
  for (i = 0; i < ht->count; i++){
    dll_free(&ht->key_elts[i], ht->free_elt);
  }
  free(ht->key_elts);
  free(ht->key_locks);
  ht->key_elts = NULL;
  ht->key_locks = NULL;
}

/** Helper functions */

/**
   Maps a hash key to a slot index in a hash table with a division method. 
*/
static size_t hash(const ht_divchn_pthread_t *ht, const void *key){
  return fast_mem_mod(key, ht->key_size, ht->count); 
}

/**
   Increase the size of a hash table to the next prime number in the
   C_PRIME_PARTS array that lowers the load factor below alpha, or if not
   possible to the largest prime number in the C_PRIME_PARTS array
   representable on a system. The operation is called if i) alpha was
   exceeded and the hash table count did not reach the largest
   prime number in the C_PRIME_PARTS array representable on a system, AND
   ii) it is guaranteed that only the calling thread has access to the hash
   table throughout the operation.
*/

typedef struct{
  size_t start;
  size_t count;
  dll_node_t **prev_key_elts;
  ht_divchn_pthread_t *ht;
} reinsert_arg_t;

static void *reinsert_thread(void *arg){
  size_t i, ix, lock_ix;
  dll_node_t **head = NULL, *node = NULL;
  reinsert_arg_t *ra = arg;
  for (i = 0; i < ra->count; i++){
    head = &ra->prev_key_elts[ra->start + i];
    while (*head != NULL){
      node = *head;
      dll_remove(head, node);
      ix = hash(ra->ht, node->key);
      lock_ix = ix & ra->ht->key_locks_mask;
      mutex_lock_perror(&ra->ht->key_locks[lock_ix]);
      dll_prepend(&ra->ht->key_elts[ix], node);
      mutex_unlock_perror(&ra->ht->key_locks[lock_ix]);
    }
  }
  return NULL;
}

static void ht_grow(ht_divchn_pthread_t *ht){
  size_t i, prev_count = ht->count;
  size_t start = 0;
  size_t seg_count, rem_count;
  dll_node_t **prev_key_elts = ht->key_elts;
  pthread_t *rids = NULL;
  reinsert_arg_t *ras = NULL;
  rids = malloc_perror(ht->num_grow_threads, sizeof(pthread_t));
  ras = malloc_perror(ht->num_grow_threads, sizeof(reinsert_arg_t));
  /* initialize next ht; num_elts can be used without lock */
  while ((float)ht->num_elts / ht->count > ht->alpha){
    ht->count_ix += C_PARTS_PER_PRIME[ht->group_ix];
    if (ht->count_ix == C_PARTS_ACC_COUNTS[ht->group_ix]) ht->group_ix++;
    if (ht->count_ix == C_PRIME_PARTS_COUNT){
      /* the largest prime in C_PRIME_PARTS built */
      break;
    }else if (is_overflow(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix])){
      /* the largest representable prime in C_PRIME_PARTS built */
      ht->count_ix = C_SIZE_MAX;
      break;
    }else{
      ht->count = build_prime(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix]);
    }
  }
  ht->key_elts = malloc_perror(ht->count, sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    dll_init(&ht->key_elts[i]);
  }
  /* multithreaded reinsertion */
  seg_count = prev_count / ht->num_grow_threads;
  rem_count = prev_count % ht->num_grow_threads;
  for (i = 0; i < ht->num_grow_threads; i++){
    ras[i].start = start;
    ras[i].count = seg_count;
    if (rem_count > 0){
      ras[i].count++;
      rem_count--;
    }
    ras[i].prev_key_elts = prev_key_elts;
    ras[i].ht = ht;
    if (i > 0) thread_create_perror(&rids[i], reinsert_thread, &ras[i]);
    start += ras[i].count;
  }
  reinsert_thread(&ras[0]); /* use the parent threads as well */
  for (i = 1; i < ht->num_grow_threads; i++){
    thread_join_perror(rids[i], NULL);
  }
  free(prev_key_elts);
  free(rids);
  free(ras);
  prev_key_elts = NULL;
  rids = NULL;
  ras = NULL;
}

/**
   Tests if the next prime number results in an overflow of size_t
   on a given system. Returns 0 if no overflow, otherwise returns 1.
*/
static int is_overflow(size_t start, size_t count){
  size_t c = 0;
  size_t n_shift;
  n_shift = C_PRIME_PARTS[start + (count - 1)];
  while (n_shift){
    n_shift >>= 1;
    c++;
  }
  return (c + (count - 1) * C_BUILD_SHIFT > C_FULL_BIT);
}

/**
   Builds a prime number from parts in the C_PRIME_PARTS array.
*/
static size_t build_prime(size_t start, size_t count){
  size_t p = 0;
  size_t n_shift;
  size_t i;
  for (i = 0; i < count; i++){
    n_shift = C_PRIME_PARTS[start + i];
    n_shift <<= (i * C_BUILD_SHIFT);
    p |= n_shift;
  }
  return p;
}

/**
   Computes a pointer to the ith element of size size in a block.
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}
