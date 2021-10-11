/**
   ht-divchn.c

   A hash table with generic contiguous or non-contiguous keys and generic
   contiguous or non-contiguous elements. 

   The implementation is based on a division method for hashing into upto  
   the number of slots determined by the largest prime number in the
   C_PRIME_PARTS array, representable as size_t on a given system, and a
   chaining method for resolving collisions. Due to chaining, the number
   of keys and elements that can be inserted is not limited by the hash
   table implementation.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by the 
   alpha parameters. The alpha parameters do not provide an upper bound 
   after the maximum count of slots in a hash table is reached.

   A distinction is made between a key and a "key_size block", and an
   element and an "elt_size block". During an insertion, a contiguous block
   of size key_size ("key_size block") and a contiguous block of size
   elt_size ("elt_size block") are copied into a hash table. A key may be
   within a contiguous or non-contiguous memory block. Given a key, the user
   decides what is copied into the key_size block of the hash table. If the
   key is within a contiguous memory block, then it can be entirely copied
   as a key_size block, or a pointer to it can be copied as a key_size
   block. If the key is within a non-contiguous memory block, then a pointer
   to it is copied as a key_size block. The same applies to an element. 

   When a pointer to a key is copied into a hash table as a key_size block,
   the user can also decide if only the pointer or the entire key is deleted
   during the delete and free operations. By setting free_key to NULL, only
   the pointer is deleted. Otherwise, the deletion is performed according to
   a non-NULL free_key. For example, when an in-memory set of images are
   used as keys (e.g. with a subset of bits in each image used for hashing)
   and pointers are copied into a hash table, then setting free_key to NULL
   will not affect the original set of images throughout the lifetime of the
   hash table. The same applies to elements and free_elt.
   
   The implementation only uses integer and pointer operations. Integer
   arithmetic is used in load factor operations, thereby eliminating the
   use of float. Given parameter values within the specified ranges,
   the implementation provides an error message and an exit is executed
   if an integer overflow is attempted* or an allocation is not completed
   due to insufficient resources. The behavior outside the specified
   parameter ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99 with the only requirement that CHAR_BIT * sizeof(size_t) is
   greater or equal to 16 and is even (at this time, every bit is required
   to participate in the value).

   * except intended wrapping around of unsigned integers in modulo
     operations, which is defined, and overflow detection as a part
     of computing bounds, which is defined by the implementation.

   TODO: add division with magic number multiplication.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ht-divchn.h"
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

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

static size_t hash(const ht_divchn_t *ht, const void *key);
static size_t mul_alpha_sz_max(size_t n, size_t alpha_n, size_t log_alpha_d);
static void ht_grow(ht_divchn_t *ht);
static int incr_count(ht_divchn_t *ht);
static int is_overflow(size_t start, size_t count);
static size_t build_prime(size_t start, size_t count);

/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size 
                 sizeof(ht_divchn_t).
   key_size    : non-zero size of a key_size block
   elt_size    : non-zero size of an elt_size block
   min_num     : minimum number of keys that are known to be or expected to
                 be present simultaneously in a hash table; results in a
                 speedup by avoiding unnecessary growth steps of a hash
                 table; 0 if a positive value is not specified and all
                 growth steps are to be completed
   alpha_n     : > 0 numerator of a load factor upper bound
   log_alpha_d : < CHAR_BIT * sizeof(size_t); log base 2 of the denominator
                 of the load factor upper bound; the denominator is a power of
                 two
   cmp_key     : - if NULL then a default memcmp-based comparison of key_size
                 blocks of keys is performed
                 - otherwise comparison function is applied which returns a
                 zero integer value iff the two keys accessed through the
                 first and the second arguments are equal; each argument is
                 a pointer to the key_size block of a key; cmp_key must use
                 the same subset of bits in a key as rdc_key
   rdc_key     : - if NULL then a default conversion of a bit pattern
                 in the key_size block of a key is performed prior to
                 hashing, which may introduce regularities
                 - otherwise rdc_key is applied to a key to reduce the key
                 to a size_t integer value prior to hashing; the argument
                 points to the key_size block of a key; rdc_key must use
                 the same subset of bits in a key as cmp_key
   free_key    : - NULL if only key_size blocks should be deleted throughout
                 the lifetime of the hash table (e.g. because keys were
                 entirely copied as key_size blocks, or because pointers
                 were copied as key_size blocks and only pointers should
                 be deleted)
                 - otherwise takes a pointer to the key_size block of a key
                 as an argument, frees the memory of the key except the
                 key_size block pointed to by the argument
   free_elt    : - NULL if only elt_size blocks should be deleted throughout
                 the lifetime of the hash table (e.g. because elements were
                 entirely copied as elt_size blocks, or because pointers
                 were copied as elt_size blocks and only pointers should
                 be deleted)
                 - otherwise takes a pointer to the elt_size block of an
                 element as an argument, frees the memory of the element
                 except the elt_size block pointed to by the argument
*/
void ht_divchn_init(ht_divchn_t *ht,
		    size_t key_size,
		    size_t elt_size,
		    size_t min_num,
		    size_t alpha_n,
		    size_t log_alpha_d,
		    int (*cmp_key)(const void *, const void *),
		    size_t (*rdc_key)(const void *),
		    void (*free_key)(void *),
		    void (*free_elt)(void *)){
  size_t i;
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
  ht->cmp_key = cmp_key;
  ht->rdc_key = rdc_key;
  ht->free_key = free_key;
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
   type. The operation is optionally called after ht_divchn_init is
   completed and before any other operation is called.
   ht            : pointer to an initialized ht_divchn_t struct
   elt_alignment : alignment requirement or size of the type, a pointer to
                   which is used to access the elt_size block of an element
                   in a hash table
*/
void ht_divchn_align(ht_divchn_t *ht, size_t elt_alignment){
  ht->elt_alignment = elt_alignment;
  dll_align_elt(ht->ll, elt_alignment);
}

/**
   Inserts a key and an associated element into a hash table by copying
   the corresponding key_size and elt_size blocks. If the key pointed to by
   the key parameter is already in the hash table according to cmp_key,
   then deletes the previous element according to free_elt and copies
   the elt_size block pointed to by the elt parameter.
   ht          : pointer to an initialized ht_divchn_t struct   
   key         : non-NULL pointer to the key_size block of a key
   elt         : non-NULL pointer to the elt_size block of an element
*/
void ht_divchn_insert(ht_divchn_t *ht, const void *key, const void *elt){
  size_t ix;
  dll_node_t **head = NULL, *node = NULL;
  ix = hash(ht, key);
  head = &ht->key_elts[ix];
  node = dll_search_key(ht->ll, head, key, ht->key_size, ht->cmp_key);
  if (node == NULL){
    dll_prepend_new(ht->ll, head, key, elt, ht->key_size, ht->elt_size);
    ht->num_elts++;
  }else{
    /* updates the elt_size block */
    if (ht->free_elt != NULL) ht->free_elt(dll_elt_ptr(ht->ll, node));
    memcpy(dll_elt_ptr(ht->ll, node), elt, ht->elt_size);
  }
  /* grow ht after ensuring it was insertion, not update */
  if (ht->num_elts > ht->max_num_elts && 
      ht->count_ix != C_SIZE_MAX &&
      ht->count_ix != C_PRIME_PARTS_COUNT){
    ht_grow(ht);
  }
}

/**
   If a key is present in a hash table, according to cmp_key, then returns a
   pointer to the elt_size block of its associated element in the hash table.
   Otherwise returns NULL. The returned pointer can be dereferenced according
   to the preceding calls to ht_divchn_init and ht_divchn_align_elt.
   ht          : pointer to an initialized ht_divchn_t struct   
   key         : non-NULL pointer to the key_size block of a key
*/
void *ht_divchn_search(const ht_divchn_t *ht, const void *key){
  const dll_node_t *node = dll_search_key(ht->ll,
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
   Removes the element associated with a key in a hash table that equals to
   the key pointed to by the key parameter according to cmp_key, by a)
   copying the elt_size block of the element to the elt_size block pointed
   to by the elt parameter and b) deleting the corresponding key_size and
   elt_size blocks in the hash table. If there is no matching key in the
   hash table according to cmp_key, leaves the hash table and the block
   pointed to by elt unchanged.
   ht          : pointer to an initialized ht_divchn_t struct   
   key         : non-NULL pointer to the key_size block of a key
   elt         : non-NULL pointer to a preallocated elt_size block
*/
void ht_divchn_remove(ht_divchn_t *ht, const void *key, void *elt){
  dll_node_t **head = &ht->key_elts[hash(ht, key)];
  dll_node_t *node =
    dll_search_key(ht->ll, head, key, ht->key_size, ht->cmp_key);
  if (node != NULL){
    memcpy(elt, dll_elt_ptr(ht->ll, node), ht->elt_size);
    /* NULL: only the key_size and elt_size blocks are deleted in ht */
    dll_delete(ht->ll, head, node, NULL, NULL);
    ht->num_elts--;
  }
}

/**
   If there is a key in a hash table that equals to the key pointed to
   by the key parameter according to cmp_key, then deletes the in-table key
   element pair according to free_key and free_elt.
   ht          : pointer to an initialized ht_divchn_t struct   
   key         : non-NULL pointer to the key_size block of a key
*/
void ht_divchn_delete(ht_divchn_t *ht, const void *key){
  dll_node_t **head = &ht->key_elts[hash(ht, key)];
  dll_node_t *node =
    dll_search_key(ht->ll, head, key, ht->key_size, ht->cmp_key);
  if (node != NULL){
    dll_delete(ht->ll, head, node, ht->free_key, ht->free_elt);
    ht->num_elts--;
  }
}

/**
   Frees the memory of all keys and elements that are in a hash table
   according to free_key and free_elt, frees the memory of the hash table,
   and leaves the block of size sizeof(ht_divchn_t) pointed to by the ht
   parameter.
*/
void ht_divchn_free(ht_divchn_t *ht){
  size_t i;
  for (i = 0; i < ht->count; i++){
    dll_free(ht->ll, &ht->key_elts[i], ht->free_key, ht->free_elt);
  }
  free(ht->ll);
  free(ht->key_elts);
  ht->ll = NULL;
  ht->key_elts = NULL;
}

/**
   Help construct a hash table parameter value in algorithms and data
   structures with a hash table parameter, complying with the stict aliasing
   rules and compatibility rules for function types. In each case, a
   (qualified) ht_divchn_t *p0 is converted to (qualified) void * and back
   to a (qualified) ht_divchn_t *p1, thus guaranteeing that the value of p0
   equals the value of p1.
*/

void ht_divchn_init_helper(void *ht,
			   size_t key_size,
			   size_t elt_size,
			   size_t min_num,
			   size_t alpha_n,
			   size_t log_alpha_d,
			   int (*cmp_key)(const void *, const void *),
			   size_t (*rdc_key)(const void *),
			   void (*free_key)(void *),
			   void (*free_elt)(void *)){
  ht_divchn_init(ht,
		 key_size,
		 elt_size,
		 min_num,
		 alpha_n,
		 log_alpha_d,
		 cmp_key,
		 rdc_key,
		 free_key,
		 free_elt);
}

void ht_divchn_align_helper(void *ht, size_t elt_alignment){
  ht_divchn_align(ht, elt_alignment);
}

void ht_divchn_insert_helper(void *ht, const void *key, const void *elt){
  ht_divchn_insert(ht, key, elt);
}

void *ht_divchn_search_helper(const void *ht, const void *key){
  return ht_divchn_search(ht, key);
}

void ht_divchnn_remove_helper(void *ht, const void *key, void *elt){
  ht_divchn_remove(ht, key, elt);
}

void ht_divchn_delete_helper(void *ht, const void *key){
  ht_divchn_delete(ht, key);
}

void ht_divchn_free_helper(void *ht){
  ht_divchn_free(ht);
} 

/** Auxiliary functions */

/**
   Converts a key to a size_t value (standard key). If rdc_key is NULL, 
   applies a safe conversion of any bit pattern in the key_size block of a
   key to reduce it to size_t. Otherwise, returns the value after applying
   rdc_key to the key.
*/
static size_t convert_std_key(const ht_divchn_t *ht, const void *key){
  size_t i;
  size_t sz_count, rem_size;
  size_t std_key = 0;
  size_t buf_size = sizeof(size_t);
  unsigned char buf[sizeof(size_t)];
  const char *k = NULL, *k_start = NULL, *k_end = NULL;
  if (ht->rdc_key != NULL) return ht->rdc_key(key);
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
static size_t hash(const ht_divchn_t *ht, const void *key){
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
   C_PRIME_PARTS array that accomodates a load factor upper bound.
   The operation is called if the load factor upper bound was exceeded (i.e.
   num_elts > max_num_elts) and count_ix is not equal to C_SIZE_MAX or
   C_PRIME_PARTS_COUNT. A single call:
   i)  lowers the load factor s.t. num_elts <= max_num_elts if a sufficiently
       large prime in the C_PRIME_PARTS array is available and is 
       representable as size_t, or 
   ii) lowers the load factor as low as possible.
   If the largest representable prime was reached, count_ix may not yet be set
   to C_SIZE_MAX or C_PRIME_PARTS_COUNT, which requires one additional call.
   Otherwise, each call increases the count.
*/
static void ht_grow(ht_divchn_t *ht){
  size_t i, prev_count = ht->count;
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head = NULL, *node = NULL;
  while (ht->num_elts > ht->max_num_elts && incr_count(ht));
  if (prev_count == ht->count) return; /* load factor not lowered */
  ht->key_elts = malloc_perror(ht->count, sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    dll_init(ht->ll, &ht->key_elts[i], ht->key_size);
  }
  if (ht->elt_alignment > 1) dll_align_elt(ht->ll, ht->elt_alignment);
  for (i = 0; i < prev_count; i++){
    head = &prev_key_elts[i];
    while (*head != NULL){
      node = *head;
      dll_remove(head, node);
      dll_prepend(&ht->key_elts[hash(ht, dll_key_ptr(ht->ll, node))], node);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
}

/**
   Attempts to increase the count of a hash table. Returns 1 if the count
   was increased. Otherwise returns 0. Updates count_ix, group_ix, count,
   and max_num_elts accordingly. If the largest representable prime was
   reached, count_ix may not yet be set to C_SIZE_MAX or
   C_PRIME_PARTS_COUNT, which requires one additional call. Otherwise, each
   call increases the count.
*/
static int incr_count(ht_divchn_t *ht){
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
   Tests if the next prime number results in the overflow of size_t
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
