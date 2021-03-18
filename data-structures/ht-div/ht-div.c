/**
   ht-div.c

   A hash table with generic hash keys and generic elements. The 
   implementation is based on a division method for hashing into upto  
   the number of slots determined by the largest number in the C_PRIMES array
   that can be represented as size_t on a given system, and a chaining method
   for resolving collisions. Due to chaining, the number of keys and elements
   that can be inserted is not limited by the hash table implementation.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by the 
   alpha parameter. The alpha parameter does not provide an upper bound 
   after the maximum count of slots in a hash table is reached.

   A hash key is an object within a contiguous block of memory (e.g. a basic 
   type, array, struct). An element is within a contiguous or noncontiguous
   memory block.

   The implementation does not use stdint.h and is portable under C89/C90
   with the only requirement that CHAR_BIT * sizeof(size_t) is greater or
   equal to 16 and is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ht-div.h"
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

/**
   An array of primes in the increasing order, approximately doubling in 
   magnitude, that are not too close to the powers of 2 and 10 to avoid 
   hashing regularities due to the structure of data.
*/
static const size_t C_PRIMES[6 * 1 + 16 * 2 + 16 * 3 + 16 * 4] =
  {0x0607,                            /* 1543 */
   0x0c2f,                            /* 3119 */
   0x1843,                            /* 6211 */
   0x3037,                            /* 12343 */
   0x5dad,                            /* 23981 */
   0xbe21,                            /* 48673 */
   0x5b0b, 0x0001,                    /* 88843 */
   0xd8d5, 0x0002,                    /* 186581 */
   0xc219, 0x0005,                    /* 377369 */
   0x0077, 0x000c,                    /* 786551 */
   0xa243, 0x0016,                    /* 1483331 */
   0x2029, 0x0031,                    /* 3219497 */
   0xcc21, 0x005f,                    /* 6278177 */
   0x5427, 0x00bf,                    /* 12538919 */
   0x037f, 0x0180,                    /* 25166719 */
   0x42bb, 0x030f,                    /* 51331771 */
   0x1c75, 0x06b7,                    /* 112663669 */
   0x96ad, 0x0c98,                    /* 211326637 */
   0x96b7, 0x1898,                    /* 412653239 */
   0xc10f, 0x2ecf,                    /* 785367311 */
   0x425b, 0x600f,                    /* 1611612763 */
   0x0007, 0xc000,                    /* 3221225479 */
   0x016f, 0x8000, 0x0001,            /* 6442451311 */
   0x9345, 0xffc8, 0x0002,            /* 12881269573 */
   0x5523, 0xf272, 0x0005,            /* 25542415651 */
   0x1575, 0x0a63, 0x000c,            /* 51713873269 */
   0x22fb, 0xca07, 0x001b,            /* 119353582331 */
   0xc513, 0x4d6b, 0x0031,            /* 211752305939 */
   0xa6cd, 0x50f3, 0x0061,            /* 417969972941 */
   0xa021, 0x5460, 0x00be,            /* 817459404833 */
   0xea29, 0x7882, 0x0179,            /* 1621224516137 */
   0xeaaf, 0x7c3d, 0x02f5,            /* 3253374675631 */
   0xab5f, 0x5a69, 0x05ff,            /* 6594291673951 */
   0x6b1f, 0x29ef, 0x0c24,            /* 13349461912351 */
   0xc81b, 0x35a7, 0x17fe,            /* 26380589320219 */
   0x57b7, 0xccbe, 0x2ffb,            /* 52758518323127 */
   0xc8fb, 0x1da8, 0x6bf3,            /* 118691918825723 */
   0x82c3, 0x2c9f, 0xc2cc,            /* 214182177768131 */
   0x3233, 0x1c54, 0x7d40, 0x0001,    /* 419189283369523 */
   0x60ad, 0x46a1, 0xf55e, 0x0002,    /* 832735214133421 */
   0x6bab, 0x40c4, 0xf12a, 0x0005,    /* 1672538661088171 */
   0xb24d, 0x6765, 0x38b5, 0x000b,    /* 3158576518771277 */
   0x789f, 0xfd94, 0xc6b2, 0x0017,    /* 6692396525189279 */
   0x0d35, 0x5443, 0xff54, 0x0030,    /* 13791536538127669 */
   0x2465, 0x74f9, 0x42d1, 0x005e,    /* 26532115188884581 */
   0xd017, 0x90c7, 0x37b3, 0x00c6,    /* 55793289756397591 */
   0x5055, 0x5a82, 0x64df, 0x0193,    /* 113545326073368661 */
   0x6f8f, 0x423b, 0x8949, 0x0304,    /* 217449629757435791 */
   0xd627, 0x08e0, 0x0b2f, 0x05fe,    /* 431794910914467367 */
   0xbbc1, 0x662c, 0x4d90, 0x0bad,    /* 841413987972987841 */
   0xf7d3, 0x45a1, 0x8ccb, 0x185d,    /* 1755714234418853843 */
   0xc647, 0x3c91, 0x46b2, 0x2e9b,    /* 3358355678469146183 */
   0x58a1, 0xbd96, 0x2836, 0x5f8c,    /* 6884922145916737697 */
   0x8969, 0x4c70, 0x6dbe, 0xdad8};   /* 15769474759331449193 */

static const size_t C_PRIME_ONE_COUNT = 6;
static const size_t C_PRIME_TWO_COUNT = 6 * 1 + 16 * 2;
static const size_t C_PRIME_THREE_COUNT = 6 * 1 + 16 * 2 + 16 * 3;
static const size_t C_PRIME_FOUR_COUNT = 6 * 1 + 16 * 2 + 16 * 3 + 16 * 4;

static size_t hash(const ht_div_t *ht, const void *key);
static void ht_grow(ht_div_t *ht);
static void copy_reinsert(ht_div_t *ht, const dll_node_t *node);

/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size sizeof(ht_div_t).
   key_size    : size of a key object
   elt_size    : - size of an element, if the element is within a contiguous
                 memory block,
                 - size of a pointer to an element, if the element is within
                 a noncontiguous memory block
   alpha       : > 0.0, a load factor upper bound
   free_elt    : - if an element is within a contiguous memory block,
                 as reflected by elt_size, and a pointer to the element is 
                 passed as elt in ht_div_insert, then the element is
                 fully copied into a hash table, and NULL as free_elt is
                 sufficient to delete the element,
                 - if an element is within a noncontiguous memory block, and
                 a pointer to a pointer to the element is passed as elt in
                 ht_div_insert, then the pointer to the element is copied into
                 the hash table, and an element-specific free_elt, taking a
                 pointer to a pointer to an element as its argument, is
                 necessary to delete the element
*/
void ht_div_init(ht_div_t *ht,
		 size_t key_size,
		 size_t elt_size,
		 float alpha,
		 void (*free_elt)(void *)){
  size_t i;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->count_ix = 0;
  ht->count = C_PRIMES[ht->count_ix];
  ht->num_elts = 0;
  ht->alpha = alpha;
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    dll_init(&ht->key_elts[i]);
  }
  ht->free_elt = free_elt;
}

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and
   elt parameters are not NULL.
*/
void ht_div_insert(ht_div_t *ht, const void *key, const void *elt){
  size_t ix;
  dll_node_t **head = NULL, *node = NULL;
  /* grow hash table if E[# keys in a slot] > alpha */
  while ((float)ht->num_elts / ht->count > ht->alpha &&
	 ht->count_ix < PRIMES_COUNT - 1){
    ht_grow(ht);
  }
  ix = hash(ht, key);
  head = &ht->key_elts[ix];
  node = dll_search_key(head, key, ht->key_size);
  if (node == NULL){
    dll_prepend(head, key, elt, ht->key_size, ht->elt_size);
    ht->num_elts++;
  }else{
    dll_delete(head, node, ht->free_elt);
    dll_prepend(head, key, elt, ht->key_size, ht->elt_size);
  }   
}

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_div_search(const ht_div_t *ht, const void *key){
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
   Removes a key and the associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_div_remove(ht_div_t *ht, const void *key, void *elt){
  dll_node_t **head = &ht->key_elts[hash(ht, key)];
  dll_node_t *node = dll_search_key(head, key, ht->key_size);
  if (node != NULL){
    memcpy(elt, node->elt, ht->elt_size);
    /* if an element is noncontiguous, only the pointer to it is deleted */
    dll_delete(head, node, NULL);
    ht->num_elts--;
  }
}

/**
   If a key is present in a hash table, deletes the key and its associated 
   element according free_elt. The key parameter is not NULL.
*/
void ht_div_delete(ht_div_t *ht, const void *key){
  dll_node_t **head = &ht->key_elts[hash(ht, key)];
  dll_node_t *node = dll_search_key(head, key, ht->key_size);
  if (node != NULL){
    dll_delete(head, node, ht->free_elt);
    ht->num_elts--;
  }
}

/**
   Frees a hash table and leaves a block of size sizeof(ht_div_t)
   pointed to by the ht parameter.
*/
void ht_div_free(ht_div_t *ht){
  size_t i;
  for (i = 0; i < ht->count; i++){
    dll_free(&ht->key_elts[i], ht->free_elt);
  }
  free(ht->key_elts);
  ht->key_elts = NULL;
}

/** Helper functions */

/**
   Maps a hash key to a slot index in a hash table with a division method. 
*/
static size_t hash(const ht_div_t *ht, const void *key){
  return fast_mem_mod(key, ht->key_size, ht->count); 
}

/**
   Increases the size of a hash table by the difference between the ith and 
   (i + 1)th prime numbers in the C_PRIMES array. Makes no changes if the
   last prime number representable as size_t on a given system was reached.
*/
static void ht_grow(ht_div_t *ht){
  size_t i, prev_count = ht->count;
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head = NULL;
  /* if the largest size is reached, alpha is not a bound for expectation */
  if (ht->count_ix == PRIMES_COUNT - 1) return;
  ht->count_ix++;
  ht->count = PRIMES[ht->count_ix];
  ht->num_elts = 0;
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    head = &ht->key_elts[i];
    dll_init(head);
  }
  for (i = 0; i < prev_count; i++){
    head = &prev_key_elts[i];
    while (*head != NULL){
      copy_reinsert(ht, *head);
      /* if an element is noncontiguous, only the pointer to it is deleted */
      dll_delete(head, *head, NULL);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
  head = NULL;
}

/**
   Reinserts a copy of a node into a new hash table during an ht_grow 
   operation. In contrast to ht_div_insert, no search is performed.
*/
static void copy_reinsert(ht_div_t *ht, const dll_node_t *node){
  dll_prepend(&ht->key_elts[hash(ht, node->key)],
	      node->key,
	      node->elt,
	      ht->key_size,
	      ht->elt_size);
  ht->num_elts++;   
}
