/**
   ht-divchn-pthread.c
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

static const size_t C_LAST_PRIME_IX = 6 + 16 * (2 + 3 + 4) - 4;
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
   Initializes a hash table. 
   num_key_locks >= 1
   num_grow_threads >= 1
   Call and return before any thread calls insert, delete and remove.
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
    if (is_overflow(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix])){
      /* last prime representable as size_t on a system reached */
      ht->count_ix = C_SIZE_MAX;
      break;
    }
    ht->count = build_prime(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix]);
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
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and
   elt parameters are not NULL.
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
      ht->count_ix != C_LAST_PRIME_IX){
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
   Call after all threads completed insert, delete and remove operations on 
   ht. This is a non-modifying query operation and has no synchronization
   overhead due to lack of race conditions.
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
   Call after all threads completed insert, delete and remove operations on
   ht.
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
   Increase the size of a hash table by the difference between the ith and 
   (i + 1)th prime numbers in the C_PRIME_PARTS array. Assumes that the
   (i + 1)th prime number in the C_PRIME_PARTS array exists. Makes no changes
   if the (i + 1)th prime number in the C_PRIME_PARTS array is not
   representable as size_t on a given system. Run when guaranteed that only
   the calling thread has access to ht.
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
    if (is_overflow(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix])){
      /* last prime representable as size_t on a system reached */
      ht->count_ix = C_SIZE_MAX;
      return;
    }
    ht->count = build_prime(ht->count_ix, C_PARTS_PER_PRIME[ht->group_ix]);
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

static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}
