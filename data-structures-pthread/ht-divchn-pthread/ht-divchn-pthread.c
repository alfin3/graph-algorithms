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
   or equal to 16 and is even (every bit is required to participate
   in the value at this time), and ii) pthreads API is available.

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
static const size_t C_BYTE_BIT = CHAR_BIT;
static const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);
static const size_t C_SIZE_MAX = (size_t)-1;

static size_t hash(const ht_divchn_pthread_t *ht, const void *key);
static size_t mul_alpha_sz_max(size_t n, size_t alpha_n, size_t log_alpha_d);
static void ht_grow(ht_divchn_pthread_t *ht);
static int incr_count(ht_divchn_pthread_t *ht);
static int is_overflow(size_t start, size_t count);
static size_t build_prime(size_t start, size_t count);
static void *ptr(const void *block, size_t i, size_t size);

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
			    void (*free_elt)(void *)){
  size_t i;
  size_t key_locks_count;
  /* hash table */
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->elt_alignment = 1;
  ht->group_ix = 0;
  ht->count_ix = 0;
  ht->count = build_prime(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix]);
  /* 0 <= max_num_elts */
  ht->max_num_elts = mul_alpha_sz_max(ht->count, alpha_n, log_alpha_d);
  while (min_num > ht->max_num_elts && incr_count(ht));
  ht->num_elts = 0;
  ht->alpha_n = alpha_n;
  ht->log_alpha_d = log_alpha_d;
  ht->ll = malloc_perror(1, sizeof(dll_t));
  ht->key_elts = malloc_perror(ht->count, sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    dll_init(ht->ll, &ht->key_elts[i], ht->key_size);
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
  ht->cmp_key = cmp_key;
  ht->rdc_key = rdc_key;
  ht->rdc_elts = rdc_elts;
  ht->free_elt = free_elt;
}

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
void ht_divchn_pthread_align_elt(ht_divchn_pthread_t *ht, size_t alignment){
  ht->elt_alignment = alignment;
  dll_align_elt(ht->ll, alignment);
}

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
			      size_t batch_count){
  size_t i, ix, lock_ix;
  size_t increased = 0;
  const void *key = NULL, *elt = NULL;
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
    key = ptr(batch_keys, i, ht->key_size);
    elt = ptr(batch_elts, i, ht->elt_size);
    ix = hash(ht, key);
    head = &ht->key_elts[ix];
    lock_ix = ix & ht->key_locks_mask;
    mutex_lock_perror(&ht->key_locks[lock_ix]);
    node = dll_search_key(ht->ll, head, key, ht->key_size, ht->cmp_key);
    if (node == NULL){
      /* insert new key element pair */
      dll_prepend_new(ht->ll, head, key, elt, ht->key_size, ht->elt_size);
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
      increased++;
    }else if (ht->rdc_elts != NULL){
      /* reduce new element and current element */
      ht->rdc_elts(dll_elt_ptr(ht->ll, node), elt, ht->elt_size);
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
    }else{
      /* update current element to new element */
      if (ht->free_elt != NULL) ht->free_elt(dll_elt_ptr(ht->ll, node));
      memcpy(dll_elt_ptr(ht->ll, node), elt, ht->elt_size);
      mutex_unlock_perror(&ht->key_locks[lock_ix]);
    }
  }

  /* grow ht if needed, and finish */
  if (ht->count_ix != C_SIZE_MAX &&
      ht->count_ix != C_PRIME_PARTS_COUNT){
    mutex_lock_perror(&ht->gate_lock);
    ht->num_elts += increased;
    if (ht->num_elts > ht->max_num_elts && ht->gate_open){
      ht->gate_open = FALSE;
      /* wait for threads that passed first critical section to finish */
      while (ht->num_in_threads > 1){
	cond_wait_perror(&ht->grow_cond, &ht->gate_lock);
      }
      mutex_unlock_perror(&ht->gate_lock);
      ht_grow(ht); /* single thread */
      mutex_lock_perror(&ht->gate_lock);
      ht->gate_open = TRUE;
      cond_broadcast_perror(&ht->gate_open_cond);
    }else if (!ht->gate_open){
      cond_signal_perror(&ht->grow_cond);
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
   insert, remove, and delete operations on ht and does not require
   thread synchronization overhead.
*/
void *ht_divchn_pthread_search(const ht_divchn_pthread_t *ht,
			       const void *key){
  const dll_node_t *node = dll_search_uq_key(ht->ll,
					     &ht->key_elts[hash(ht, key)],
					     key,
					     ht->key_size,
					     ht->cmp_key);
  if (node == NULL){
    return NULL;
  }else{
    return dll_elt_ptr(ht->ll, node);
  }
}

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
			      size_t batch_count){
  size_t i, ix, lock_ix;
  size_t removed = 0;
  const void *key = NULL;
  void *elt = NULL;
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
    key = ptr(batch_keys, i, ht->key_size);
    elt = ptr(batch_elts, i, ht->elt_size);
    ix = hash(ht, key);
    head = &ht->key_elts[ix];
    lock_ix = ix & ht->key_locks_mask;
    mutex_lock_perror(&ht->key_locks[lock_ix]);
    node = dll_search_key(ht->ll, head, key, ht->key_size, ht->cmp_key);
    if (node != NULL){
      memcpy(elt, dll_elt_ptr(ht->ll, node), ht->elt_size);
      /* if an element is noncontiguous, only the pointer to it is deleted */
      dll_delete(ht->ll, head, node, NULL);
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
   If a key is not in the hash table, no operation with respect to the key
   is performed. The batch_keys parameter is not NULL and points to an array
   of blocks of size key_size. The batch_count parameter is the count of keys
   in a batch.
*/
void ht_divchn_pthread_delete(ht_divchn_pthread_t *ht,
			      const void *batch_keys,
			      size_t batch_count){
  size_t i, ix, lock_ix;
  size_t deleted = 0;
  const void *key = NULL;
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
    key = ptr(batch_keys, i, ht->key_size); 
    ix = hash(ht, key);
    head = &ht->key_elts[ix];
    lock_ix = ix & ht->key_locks_mask;
    mutex_lock_perror(&ht->key_locks[lock_ix]);
    node = dll_search_key(ht->ll, head, key, ht->key_size, ht->cmp_key);
    if (node != NULL){
      dll_delete(ht->ll, head, node, ht->free_elt);
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
   insert, remove, delete, and search operations. Leaves a block of size
   sizeof(ht_divchn_pthread_t) pointed to by the ht parameter.
*/
void ht_divchn_pthread_free(ht_divchn_pthread_t *ht){
  size_t i;
  for (i = 0; i < ht->count; i++){
    dll_free(ht->ll, &ht->key_elts[i], ht->free_elt);
  }
  free(ht->ll);
  free(ht->key_elts);
  free(ht->key_locks);
  ht->ll = NULL;
  ht->key_elts = NULL;
  ht->key_locks = NULL;
}

/**
   Help construct a hash table parameter value in algorithms and data
   structures with a hash table parameter, complying with the stict aliasing
   rules and compatibility rules for function types. In each case, a
   (qualified) ht_divchn_pthread_t *p0 is converted to (qualified) void * 
   and back to a (qualified) ht_divchn_ptrhead_t *p1, thus guaranteeing that
   the value of p0 equals the value of p1. An initialization helper is
   constructed by the user. 
*/

void ht_divchn_pthread_align_elt_helper(void *ht, size_t alignment){
  ht_divchn_pthread_align_elt(ht, alignment);
}

void ht_divchn_pthread_insert_helper(void *ht,
				     const void *batch_keys,
				     const void *batch_elts,
				     size_t batch_count){
  ht_divchn_pthread_insert(ht, batch_keys, batch_elts, batch_count);
}

void *ht_divchn_pthread_search_helper(const void *ht,
				      const void *key){
  return ht_divchn_pthread_search(ht, key);
}

void ht_divchn_pthread_remove_helper(void *ht,
				     const void *batch_keys,
				     void *batch_elts,
				     size_t batch_count){
  ht_divchn_pthread_remove(ht, batch_keys, batch_elts, batch_count);
}

void ht_divchn_pthread_delete_helper(void *ht,
				     const void *batch_keys,
				     size_t batch_count){
  ht_divchn_pthread_delete(ht, batch_keys, batch_count);
}

void ht_divchn_pthread_free_helper(void *ht){
  ht_divchn_pthread_free(ht);
}

/** Auxiliary functions */

/**
   Converts a key to a key of the standard size. This is a safe conversion
   of any bit pattern in the block pointed to by key to size_t.
*/
static size_t convert_std_key(const ht_divchn_pthread_t *ht,
			      const void *key){
  size_t i;
  size_t sz_count, rem_size;
  size_t std_key = 0;
  size_t buf_size = sizeof(size_t);
  unsigned char buf[sizeof(size_t)];
  const char *k = NULL, *k_start = NULL, *k_end = NULL;
  if (ht->rdc_key != NULL) return ht->rdc_key(key, ht->key_size);
  sz_count = ht->key_size / buf_size; /* division by sizeof(size_t) */
  rem_size = ht->key_size - sz_count * buf_size;
  k = key;
  memset(buf, 0, buf_size);
  memcpy(buf, k, rem_size);
  for (i = 0; i < rem_size; i++){
    std_key += (size_t)buf[i] << (i * C_BYTE_BIT);
  }
  k_start = k + rem_size;
  k_end = k_start + sz_count * buf_size;
  for (k = k_start; k != k_end; k += buf_size){
    memcpy(buf, k, buf_size);
    for (i = 0; i < buf_size; i++){
      std_key += (size_t)buf[i] << (i * C_BYTE_BIT);
    }
  }
  return std_key;
}

/**
   Maps a hash key to a slot index in a hash table with a division method. 
*/
static size_t hash(const ht_divchn_pthread_t *ht, const void *key){
  return convert_std_key(ht, key) % ht->count;
}

/**
   Multiplies an unsigned integer n by a load factor upper bound, represented
   by a numerator and log base 2 of a denominator. The denominator is a
   power of two. Returns the product if it is representable as size_t.
   Otherwise returns the maximal value of size_t.
*/
static size_t mul_alpha_sz_max(size_t n, size_t alpha_n, size_t log_alpha_d){
  size_t h, l;
  mul_ext(n, alpha_n, &h, &l);
  if (h >> log_alpha_d) return C_SIZE_MAX; /* overflow after division */
  l >>= log_alpha_d;
  h <<= (C_FULL_BIT - log_alpha_d);
  return l + h;
}

/**
   Increases the count of a hash table to the next prime number in the
   C_PRIME_PARTS array that accomodates alpha as a load factor upper bound.
   The operation is called if i) alpha was exceeded (i.e. num_elts > 
   max_num_elts) and count_ix is not equal to C_SIZE_MAX or
   C_PRIME_PARTS_COUNT, AND ii) it is guaranteed that only the calling thread
   has access to the hash table throughout the operation. A single call:
   i)  lowers the load factor s.t. num_elts <= max_num_elts if a sufficiently
       large prime in the C_PRIME_PARTS array is available and is 
       representable as size_t, or 
   ii) lowers the load factor as low as possible.
   If the largest representable prime is reached, count_ix may not yet be set
   to C_SIZE_MAX or C_PRIME_PARTS_COUNT, which requires one additional call
   that does not increase the count. Otherwise, each call increases the
   count.
*/

typedef struct{
  size_t start;
  size_t count;
  dll_node_t **prev_key_elts;
  const ht_divchn_pthread_t *ht;
} reinsert_arg_t;

static void *reinsert_thread(void *arg){
  size_t i, ix, lock_ix;
  dll_node_t **head = NULL, *node = NULL;
  const reinsert_arg_t *ra = arg;
  for (i = ra->start; i < ra->start + ra->count; i++){
    head = &ra->prev_key_elts[i];
    while (*head != NULL){
      node = *head;
      dll_remove(head, node);
      ix = hash(ra->ht, dll_key_ptr(ra->ht->ll, node));
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
  /* initialize next ht; num_elts can be used without lock */
  while (ht->num_elts > ht->max_num_elts && incr_count(ht));
  if (prev_count == ht->count) return; /* load factor not lowered */
  rids = malloc_perror(ht->num_grow_threads, sizeof(pthread_t));
  ras = malloc_perror(ht->num_grow_threads, sizeof(reinsert_arg_t));
  ht->key_elts = malloc_perror(ht->count, sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    dll_init(ht->ll, &ht->key_elts[i], ht->key_size);
  }
  /* multithreaded reinsertion */
  seg_count = prev_count / ht->num_grow_threads;
  rem_count = prev_count - seg_count * ht->num_grow_threads;
  for (i = 0; i < ht->num_grow_threads; i++){
    ras[i].start = start;
    ras[i].count = seg_count;
    ras[i].count += (rem_count > 0 && rem_count--);
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
   Attempts to increase the count of a hash table. Returns 1 if the count
   was increased. Otherwise returns 0. Updates count_ix, group_ix, count,
   and max_num_elts accordingly. If the largest representable prime is
   reached, count_ix may not yet be set to C_SIZE_MAX or C_PRIME_PARTS_COUNT,
   which requires one additional call that does not increase the count.
   Otherwise, each call increases the count.
*/
static int incr_count(ht_divchn_pthread_t *ht){
  ht->count_ix += C_PARTS_PER_PRIME[ht->group_ix];
  if (ht->count_ix == C_PARTS_ACC_COUNTS[ht->group_ix]) ht->group_ix++;
  if (ht->count_ix == C_PRIME_PARTS_COUNT){
    return 0;
  }else if (is_overflow(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix])){
    ht->count_ix = C_SIZE_MAX;
    return 0;
  }else{
    ht->count = build_prime(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix]);
    /* 0 <= max_num_elts <= C_SIZE_MAX */
    ht->max_num_elts = mul_alpha_sz_max(ht->count,
					ht->alpha_n,
					ht->log_alpha_d);
  }
  return 1;
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
