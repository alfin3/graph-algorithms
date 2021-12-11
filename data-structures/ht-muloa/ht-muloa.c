/**
   ht-muloa.c

   A hash table with generic contiguous or non-contiguous keys and generic
   contiguous or non-contiguous elements.
   
   The implementation is based on a multiplication method for hashing into
   upto 2**(size_t width - 1) slots and an open addressing method with
   double hashing for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by 
   the alpha parameters. The expected number of probes in a search is 
   upper-bounded by 1/(1 - load factor bound), under the uniform hashing
   assumption. 

   The alpha parameters do not provide an upper bound after the maximum 
   count of slots in a hash table is reached. After exceeding the load
   factor bound, the load factor is <= 1.0 due to open addressing, and the
   expected number of probes is upper-bounded by 1/(1 - load factor) before
   the full occupancy is reached.

   A distinction is made between a key and a "key_size block", and an
   element and an "elt_size block". During an insertion without update*,
   a contiguous block of size key_size ("key_size block") and a contiguous
   block of size elt_size ("elt_size block") are copied into a hash table.
   A key may be within a contiguous or non-contiguous memory block. Given
   a key, the user decides what is copied into the key_size block of the
   hash table. If the key is within a contiguous memory block, then it can
   be entirely copied as a key_size block, or a pointer to it can be copied
   as a key_size block. If the key is within a non-contiguous memory block,
   then a pointer to it is copied as a key_size block. The same applies to
   an element.

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
   if an integer overflow is attempted** or an allocation is not completed
   due to insufficient resources. The behavior outside the specified
   parameter ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99 with the only requirement that the width of size_t is
   greater or equal to 16, less than 2040, and is even.

   * if there is an update during an insertion, a key_size block is not
   copied and an elt_size block is copied.

   ** except intended wrapping around of unsigned integers in modulo
   operations, which is defined, and overflow detection as a part
   of computing bounds, which is defined by the implementation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ht-muloa.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-lim.h"

static const size_t C_FIRST_PRIME_PARTS[1 + 8 * (2 + 3 + 4)] =
  {0xbe21u,                            /* 2**15 < 48673 < 2**16 */
   0xd8d5u, 0x0002u,                   /* 2**17 < 186581 < 2**18 */
   0x0077u, 0x000cu,                   /* 2**19 < 786551 < 2**20 */
   0x2029u, 0x0031u,                   /* 2**21 < 3219497 < 2**22 */
   0x5427u, 0x00bfu,                   /* 2**23 < 12538919 < 2**24 */
   0x42bbu, 0x030fu,                   /* 2**25 < 51331771 < 2**26 */
   0x96adu, 0x0c98u,                   /* 2**27 < 211326637 < 2**28 */
   0xc10fu, 0x2ecfu,                   /* 2**29 < 785367311 < 2**30 */
   0x72e9u, 0xad16u,                   /* 2**31 < 2903929577 < 2**32 */
   0x9345u, 0xffc8u, 0x0002u,          /* 2**33 < 12881269573 < 2**34 */
   0x1575u, 0x0a63u, 0x000cu,          /* 2**35 < 51713873269 < 2**36 */
   0xc513u, 0x4d6bu, 0x0031u,          /* 2**37 < 211752305939 < 2**38 */
   0xa021u, 0x5460u, 0x00beu,          /* 2**39 < 817459404833 < 2**40 */
   0xeaafu, 0x7c3du, 0x02f5u,          /* 2**41 < 3253374675631 < 2**42 */
   0x6b1fu, 0x29efu, 0x0c24u,          /* 2**43 < 13349461912351 < 2**44 */
   0x57b7u, 0xccbeu, 0x2ffbu,          /* 2**45 < 52758518323127 < 2**46 */
   0x82c3u, 0x2c9fu, 0xc2ccu,          /* 2**47 < 214182177768131 < 2**48 */
   0x60adu, 0x46a1u, 0xf55eu, 0x0002u, /* 2**49 < 832735214133421 < 2**50 */
   0xb24du, 0x6765u, 0x38b5u, 0x000bu, /* 2**51 < 3158576518771277 < 2**52 */
   0x0d35u, 0x5443u, 0xff54u, 0x0030u, /* 2**53 < 13791536538127669 < 2**54 */
   0xd017u, 0x90c7u, 0x37b3u, 0x00c6u, /* 2**55 < 55793289756397591 < 2**56 */
   0x6f8fu, 0x423bu, 0x8949u, 0x0304u, /* 2**57 < 217449629757435791 < 2**58 */
   0xbbc1u, 0x662cu, 0x4d90u, 0x0badu, /* 2**59 < 841413987972987841 < 2**60 */
   0xc647u, 0x3c91u, 0x46b2u, 0x2e9bu, /* 2**61 < 3358355678469146183 < 2**62 */
   0x8969u, 0x4c70u, 0x6dbeu, 0xdad8u  /* 2**63 < 15769474759331449193 < 2**64 */
  }; 

static const size_t C_SECOND_PRIME_PARTS[1 + 8 * (2 + 3 + 4)] =
  {0xc221u,                            /* 2**15 < 49697 < 2**16 */
   0xe04bu, 0x0002u,                   /* 2**17 < 188491 < 2**18 */
   0xf6a7u, 0x000bu,                   /* 2**19 < 784039 < 2**20 */
   0x1b4fu, 0x0030u,                   /* 2**21 < 3152719 < 2**22 */
   0x4761u, 0x00beu,                   /* 2**23 < 12470113 < 2**24 */
   0x3eadu, 0x0312u,                   /* 2**25 < 51527341 < 2**26 */
   0x08e9u, 0x0ca5u,                   /* 2**27 < 212142313 < 2**28 */
   0x06b9u, 0x2eecu,                   /* 2**29 < 787220153 < 2**30 */
   0x5391u, 0xbba6u,                   /* 2**31 < 3148239761 < 2**32 */
   0x3739u, 0xf7fdu, 0x0002u,          /* 2**33 < 12750501689 < 2**34 */
   0x852bu, 0x07f8u, 0x000cu,          /* 2**35 < 51673335083 < 2**36 */
   0xa61bu, 0x457au, 0x0031u,          /* 2**37 < 211619063323 < 2**38 */
   0xb041u, 0xbf9eu, 0x00bdu,          /* 2**39 < 814963667009 < 2**40 */
   0x4515u, 0x3eafu, 0x0308u,          /* 2**41 < 3333946295573 < 2**42 */
   0x6f4fu, 0xc0d9u, 0x0c3cu,          /* 2**43 < 13455073046351 < 2**44 */
   0x0da1u, 0x6600u, 0x3025u,          /* 2**45 < 52937183202721 < 2**46 */
   0xb229u, 0x8facu, 0xc1e5u,          /* 2**47 < 213191702131241 < 2**48 */
   0x58f1u, 0x94e9u, 0xff18u, 0x0002u, /* 2**49 < 843430996039921 < 2**50 */
   0x73abu, 0xda62u, 0x9da8u, 0x000bu, /* 2**51 < 3269573287769003 < 2**52 */
   0x37f1u, 0xd800u, 0x135bu, 0x0031u, /* 2**53 < 13813559045666801 < 2**54 */
   0xd909u, 0xa518u, 0xebc1u, 0x00c4u, /* 2**55 < 55428312366373129 < 2**56 */
   0x03a7u, 0x5cb0u, 0xba89u, 0x0302u, /* 2**57 < 216940831195530151 < 2**58 */
   0x12adu, 0x7477u, 0xb251u, 0x0c10u, /* 2**59 < 869390790998561453 < 2**60 */
   0xe411u, 0x4bacu, 0x9c82u, 0x2f17u, /* 2**61 < 3393352927676261393 < 2**62 */
   0xd047u, 0x33a5u, 0x5cb7u, 0xbd8fu  /* 2**63 < 13659238136753279047 < 2**64 */
  };

static const size_t C_LAST_PRIME_IX = 1u + 8u * (2u + 3u + 4u) - 4u;
static const size_t C_PARTS_PER_PRIME[4] = {1u, 2u, 3u, 4u};
static const size_t C_PARTS_ACC_COUNTS[4] = {1u,
					     1u + 8u * 2u,
					     1u + 8u * (2u + 3u),
					     1u + 8u * (2u + 3u + 4u)};
static const size_t C_BUILD_SHIFT = 16u;
static const size_t C_BYTE_BIT = CHAR_BIT;
static const size_t C_FULL_BIT = PRECISION_FROM_ULIMIT((size_t)-1);
static const size_t C_LOG_COUNT_LLIMIT = 8u; /* > 0 */
static const size_t C_LOG_COUNT_ULIMIT = PRECISION_FROM_ULIMIT((size_t)-1) - 1u;

/* placeholder handling */
static struct ke *ph_new();
static int is_ph(const struct ke *ke);
static void ph_free(struct ke *ke);

/* key element handling */
static struct ke *ke_new(const struct ht_muloa *ht,
			 size_t fval,
			 size_t sval,
			 const void *key,
			 const void *elt);
static void ke_elt_update(const struct ht_muloa *ht,
			  struct ke *ke,
			  const void *elt);
static void *ke_key_ptr(const struct ht_muloa *ht, const struct ke *ke);
static void *ke_elt_ptr(const struct ht_muloa *ht, const struct ke *ke);
static void ke_free(const struct ht_muloa *ht, struct ke *ke);

/* hashing */
static size_t convert_std_key(const struct ht_muloa *ht, const void *key);
static size_t adjust_dist(size_t dist);

/* hash table operations and maintenance*/
static struct ke **search(const struct ht_muloa *ht, const void *key);
static size_t mul_alpha(size_t n, size_t alpha_n, size_t log_alpha_d);
static int incr_count(struct ht_muloa *ht);
static void ht_grow(struct ht_muloa *ht);
static void ht_clean(struct ht_muloa *ht);
static void reinsert(struct ht_muloa *ht, const struct ke *prev_ke);

/* integer constant construction */
static size_t find_build_prime(const size_t *parts);

/**
   Initializes a hash table. An in-table elt_size block is guaranteed to
   be accessible only with a pointer to a character, unless additional
   alignment is performed by calling ht_muloa_align.
   ht          : a pointer to a preallocated block of size 
                 sizeof(struct ht_muloa).
   key_size    : non-zero size of a key_size block; must account for internal
                 and trailing padding according to sizeof
   elt_size    : non-zero size of an elt_size block; must account for internal
                 and trailing padding according to sizeof
   min_num     : minimum number of keys that are known to be or expected to
                 be present simultaneously in a hash table; results in a
                 speedup by avoiding unnecessary growth steps of a hash
                 table; 0 if a positive value is not specified and all
                 growth steps are to be completed
   alpha_n     : > 0 numerator of a load factor upper bound
   log_alpha_d : < size_t width; log base 2 of the denominator of the load
                 factor upper bound; the denominator is a power of two and
                 is greater or equal to alpha_n
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
void ht_muloa_init(struct ht_muloa *ht,
		   size_t key_size,
		   size_t elt_size,
		   size_t min_num,
		   size_t alpha_n,
		   size_t log_alpha_d,
		   int (*cmp_key)(const void *, const void *),
		   size_t (*rdc_key)(const void *),
		   void (*free_key)(void *),
		   void (*free_elt)(void *)){
  size_t i, rem;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  /* align ke relative to a malloc's pointer */
  if (key_size <= sizeof(size_t)){
    ht->key_offset = sizeof(size_t);
  }else{
    rem = key_size % sizeof(size_t);
    ht->key_offset = key_size;
    ht->key_offset = add_sz_perror(ht->key_offset,
				   (rem > 0) * (sizeof(size_t) - rem));
  }
  /* elt_size block accessible with a character pointer */
  ht->elt_offset = sizeof(struct ke);
  ht->elt_alignment = 1;
  ht->log_count = C_LOG_COUNT_LLIMIT;
  ht->count = pow_two_perror(C_LOG_COUNT_LLIMIT);
  ht->alpha_n = alpha_n;
  ht->log_alpha_d = log_alpha_d;
  /* 0 <= max_sum < count */
  ht->max_sum = mul_alpha(ht->count, alpha_n, log_alpha_d);
  if (ht->max_sum == ht->count) ht->max_sum = ht->count - 1;
  while (min_num > ht->max_sum && incr_count(ht));
  ht->max_num_probes = 1; /* at least one probe */
  ht->num_elts = 0;
  ht->num_phs = 0;
  ht->fprime = find_build_prime(C_FIRST_PRIME_PARTS);
  ht->sprime = find_build_prime(C_SECOND_PRIME_PARTS);
  ht->ph = ph_new();
  ht->key_elts = malloc_perror(ht->count, sizeof(struct ke *));
  for (i = 0; i < ht->count; i++){
    ht->key_elts[i] = NULL;
  }
  ht->cmp_key = cmp_key;
  ht->rdc_key = rdc_key;
  ht->free_key = free_key;
  ht->free_elt = free_elt;
}

/**
   Aligns each in-table elt_size block to be accessible with a pointer to a
   type T other than character through ht_muloa_search (in addition to a
   character pointer). If alignment requirement of T is unknown, the size
   of T can be used as a value of the alignment parameter because size of
   T >= alignment requirement of T (due to structure of arrays), which may
   result in overalignment. The hash table keeps the effective type of a
   copied elt_size block, if it had one at the time of insertion, and T must
   be compatible with the type to comply with the strict aliasing rules.
   T can be the same or a cvr-qualified/signed/unsigned version of the
   type. The operation is optionally called after ht_muloa_init is
   completed and before any other operation is called.
   ht            : pointer to an initialized ht_muloa struct
   elt_alignment : alignment requirement or size of the type, a pointer to
                   which is used to access the elt_size block of an element
                   in a hash table; if size, must account for internal
                   and trailing padding according to sizeof
*/
void ht_muloa_align(struct ht_muloa *ht, size_t elt_alignment){
  size_t alloc_ptr_offset = add_sz_perror(ht->key_offset, ht->elt_offset);
  size_t rem;
  /* elt_offset to align elt_size block relative to malloc's pointer */
  if (alloc_ptr_offset <= elt_alignment){
    ht->elt_offset = add_sz_perror(ht->elt_offset,
				   elt_alignment - alloc_ptr_offset);
  }else{
    rem = alloc_ptr_offset % elt_alignment;
    ht->elt_offset = add_sz_perror(ht->elt_offset,
				   (rem > 0) * (elt_alignment - rem));
  }
}

/**
   Inserts a key and an associated element into a hash table by copying
   the corresponding key_size and elt_size blocks. If the key pointed to by
   the key parameter is already in the hash table according to cmp_key,
   then deletes the previous element according to free_elt and copies
   the elt_size block pointed to by the elt parameter.
   ht          : pointer to an initialized ht_muloa struct   
   key         : non-NULL pointer to the key_size block of a key
   elt         : non-NULL pointer to the elt_size block of an element
*/
void ht_muloa_insert(struct ht_muloa *ht, const void *key, const void *elt){
  size_t num_probes = 1;
  size_t std_key;
  size_t fval, sval;
  size_t ix, dist;
  struct ke **ke = NULL;
  std_key = convert_std_key(ht, key);
  fval = ht->fprime * std_key; /* mod 2**C_FULL_BIT */
  sval = ht->sprime * std_key; /* mod 2**C_FULL_BIT */
  ix = fval >> (C_FULL_BIT - ht->log_count);
  dist = adjust_dist(sval >> (C_FULL_BIT - ht->log_count));
  ke = &ht->key_elts[ix];
  while (*ke != NULL){
    if (ht->cmp_key != NULL && /* loop invariant */
	!is_ph(*ke) &&
	ht->cmp_key(ke_key_ptr(ht, *ke), key) == 0){
      ke_elt_update(ht, *ke, elt);
      return;
    }else if (ht->cmp_key == NULL && /* loop invariant */
	      !is_ph(*ke) &&
	      memcmp(ke_key_ptr(ht, *ke), key, ht->key_size) == 0){
      ke_elt_update(ht, *ke, elt);
      return;
    }
    ix = sum_mod(dist, ix, ht->count);
    ke = &ht->key_elts[ix];
    num_probes++;
    if (num_probes > ht->max_num_probes) ht->max_num_probes++;
  }
  fval -= fval & 1; /* 1st bit not used in hashing => 1 as ph identifier */
  *ke = ke_new(ht, fval, sval, key, elt);
  ht->num_elts++;
  /* max_sum < count; grow ht after ensuring it was insertion, not update */
  if (ht->num_elts + ht->num_phs > ht->max_sum){
    if (ht->num_elts < ht->num_phs){
      ht_clean(ht);
    }else if (ht->log_count < C_LOG_COUNT_ULIMIT){
      ht_grow(ht);
    }
  }
}

/**
   If a key is present in a hash table, according to cmp_key, then returns a
   pointer to the elt_size block of its associated element in the hash table.
   Otherwise returns NULL. The returned pointer can be dereferenced according
   to the preceding calls to ht_muloa_init and ht_muloa_align_elt.
   ht          : pointer to an initialized ht_muloa struct   
   key         : non-NULL pointer to the key_size block of a key
*/
void *ht_muloa_search(const struct ht_muloa *ht, const void *key){
  struct ke * const *ke = search(ht, key);
  if (ke != NULL){
    return ke_elt_ptr(ht, *ke);
  }else{
    return NULL;
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
   ht          : pointer to an initialized ht_muloa struct   
   key         : non-NULL pointer to the key_size block of a key
   elt         : non-NULL pointer to a preallocated elt_size block
*/
void ht_muloa_remove(struct ht_muloa *ht, const void *key, void *elt){
  struct ke **ke = search(ht, key);
  if (ke != NULL){
    memcpy(elt, ke_elt_ptr(ht, *ke), ht->elt_size);
    /* only the key_size and elt_size blocks are deleted in ht */
    free(ke_key_ptr(ht, *ke));
    *ke = ht->ph;
    ht->num_elts--;
    ht->num_phs++;
  }
}

/**
   If there is a key in a hash table that equals to the key pointed to
   by the key parameter according to cmp_key, then deletes the in-table key
   element pair according to free_key and free_elt.
   ht          : pointer to an initialized ht_muloa struct   
   key         : non-NULL pointer to the key_size block of a key
*/
void ht_muloa_delete(struct ht_muloa *ht, const void *key){
  struct ke **ke = search(ht, key);
  if (ke != NULL){
    ke_free(ht, *ke);
    *ke = ht->ph;
    ht->num_elts--;
    ht->num_phs++;
  }
}

/**
   Frees the memory of all keys and elements that are in a hash table
   according to free_key and free_elt, frees the memory of the hash table,
   and leaves the block of size sizeof(struct ht_muloa) pointed to by the ht
   parameter.
*/
void ht_muloa_free(struct ht_muloa *ht){
  size_t i;
  struct ke * const *ke = NULL;
  for (i = 0; i < ht->count; i++){
    ke = &ht->key_elts[i];
    if (*ke != NULL && !is_ph(*ke)){
      ke_free(ht, *ke);
    }
  }
  ph_free(ht->ph);
  free(ht->key_elts);
  ht->ph = NULL;
  ht->key_elts = NULL;
}

/**
   Help construct a hash table parameter value in algorithms and data
   structures with a hash table parameter, complying with the stict aliasing
   rules and compatibility rules for function types. In each case, a
   (qualified) struct ht_muloa *p0 is converted to (qualified) void * and
   back to a (qualified) struct ht_muloa *p1, thus guaranteeing that the
   value of p0 equals the value of p1.
*/

void ht_muloa_init_helper(void *ht,
			  size_t key_size,
			  size_t elt_size,
			  size_t min_num,
			  size_t alpha_n,
			  size_t log_alpha_d,
			  int (*cmp_key)(const void *, const void *),
			  size_t (*rdc_key)(const void *),
			  void (*free_key)(void *),
			  void (*free_elt)(void *)){
  ht_muloa_init(ht,
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

void ht_muloa_align_helper(void *ht, size_t elt_alignment){
  ht_muloa_align(ht, elt_alignment);
}

void ht_muloa_insert_helper(void *ht, const void *key, const void *elt){
  ht_muloa_insert(ht, key, elt);
}

void *ht_muloa_search_helper(const void *ht, const void *key){
  return ht_muloa_search(ht, key);
}

void ht_muloa_remove_helper(void *ht, const void *key, void *elt){
  ht_muloa_remove(ht, key, elt);
}

void ht_muloa_delete_helper(void *ht, const void *key){
  ht_muloa_delete(ht, key);
}

void ht_muloa_free_helper(void *ht){
  ht_muloa_free(ht);
}

/** Auxiliary functions */

/**
   Create, test, and free a placeholder. The is_ph function can be used on
   a non-placeholder.
*/

static struct ke *ph_new(){
  struct ke *ke = malloc_perror(1, sizeof(struct ke));
  ke->fval = 1;
  ke->sval = 0;
  return ke;
}

static int is_ph(const struct ke *ke){
  return (ke->fval == 1);
}

static void ph_free(struct ke *ke){
  free(ke);
  ke = NULL;
}

/**
   Create, update, and free a key element. These functions cannot be used on
   a placeholder.
*/

static struct ke *ke_new(const struct ht_muloa *ht,
			 size_t fval,
			 size_t sval,
			 const void *key,
			 const void *elt){
  void *ke_block = NULL;
  struct ke *ke = NULL;
  ke_block =  
    malloc_perror(1, add_sz_perror(ht->key_offset,
				   add_sz_perror(ht->elt_offset,
						 ht->elt_size)));
  ke = (struct ke *)((char *)ke_block + ht->key_offset);
  ke->fval = fval;
  ke->sval = sval;
  memcpy(ke_key_ptr(ht, ke), key, ht->key_size);
  memcpy(ke_elt_ptr(ht, ke), elt, ht->elt_size);
  return ke;
}

static void ke_elt_update(const struct ht_muloa *ht,
			  struct ke *ke,
			  const void *elt){
  if (ht->free_elt != NULL) ht->free_elt(ke_elt_ptr(ht, ke));
  memcpy(ke_elt_ptr(ht, ke), elt, ht->elt_size);
}

static void *ke_key_ptr(const struct ht_muloa *ht, const struct ke *ke){
  return (void *)((char *)ke - ht->key_offset);
}

static void *ke_elt_ptr(const struct ht_muloa *ht, const struct ke *ke){
  return (void *)((char *)ke + ht->elt_offset);
}

static void ke_free(const struct ht_muloa *ht, struct ke *ke){
  if (ht->free_key != NULL) ht->free_key(ke_key_ptr(ht, ke));
  if (ht->free_elt != NULL) ht->free_elt(ke_elt_ptr(ht, ke));
  free(ke_key_ptr(ht, ke));
  ke = NULL;
}

/**
   Converts a key to a size_t value (standard key). If rdc_key is NULL, 
   applies a safe conversion of any bit pattern in the key_size block of a
   key to reduce it to size_t. Otherwise, returns the value after applying
   rdc_key to the key.
*/
static size_t convert_std_key(const struct ht_muloa *ht, const void *key){
  size_t i;
  size_t sz_count, rem_size;
  size_t std_key = 0;
  size_t buf_size = sizeof(size_t);
  unsigned char buf[sizeof(size_t)];
  const void *k = NULL, *k_start = NULL, *k_end = NULL;
  if (ht->rdc_key != NULL) return ht->rdc_key(key);
  sz_count = ht->key_size / buf_size; /* division by sizeof(size_t) */
  rem_size = ht->key_size - sz_count * buf_size;
  k = key;
  memset(buf, 0, buf_size);
  memcpy(buf, k, rem_size);
  for (i = 0; i < rem_size; i++){
    std_key += (size_t)buf[i] << (i * C_BYTE_BIT);
  }
  k_start = (char *)k + rem_size;
  k_end = (char *)k_start + sz_count * buf_size;
  for (k = k_start; k != k_end; k = (char *)k + buf_size){
    memcpy(buf, k, buf_size);
    for (i = 0; i < buf_size; i++){
      std_key += (size_t)buf[i] << (i * C_BYTE_BIT);
    }
  }
  return std_key;
}

/**
   Adjusts a probe distance to an odd distance, if necessary. 
*/
static size_t adjust_dist(size_t dist){
  size_t ret = dist;
  if (!(dist & 1)){
    if (dist == 0){
      ret++;
    }else{
      ret--;
    }
  }
  return ret;
}

/**
   If a key is present in a hash table, returns a pointer to a slot
   in the key_elts array that stores a pointer to ke with the
   key, otherwise returns NULL.
*/
static struct ke **search(const struct ht_muloa *ht, const void *key){
  size_t num_probes = 1;
  size_t std_key, fval, sval, ix, dist;
  struct ke * const *ke = NULL;
  std_key = convert_std_key(ht, key);
  fval = ht->fprime * std_key; /* mod 2**FULL_BIT */
  sval = ht->sprime * std_key; /* mod 2**FULL_BIT */
  ix = fval >> (C_FULL_BIT - ht->log_count);
  dist = adjust_dist(sval >> (C_FULL_BIT - ht->log_count));
  ke = &ht->key_elts[ix];
  while (*ke != NULL){
    if (ht->cmp_key != NULL && /* loop invariant */
	!is_ph(*ke) &&
	ht->cmp_key(ke_key_ptr(ht, *ke), key) == 0){
      return (struct ke **)ke;
    }else if (ht->cmp_key == NULL && /* loop invariant */
	      !is_ph(*ke) &&
	      memcmp(ke_key_ptr(ht, *ke), key, ht->key_size) == 0){
      return (struct ke **)ke;
    }else if (num_probes == ht->max_num_probes){
      break;
    }else{
      ix = sum_mod(dist, ix, ht->count);
      ke = &ht->key_elts[ix];
      num_probes++;
    }
  }
  return NULL;
}

/**
   Multiplies an unsigned integer n by a load factor upper bound, represented
   by a numerator and log base 2 of a denominator. The denominator is a
   power of two.
*/
static size_t mul_alpha(size_t n, size_t alpha_n, size_t log_alpha_d){
  size_t h, l;
  mul_ext(n, alpha_n, &h, &l);
  l >>= log_alpha_d;
  h <<= (C_FULL_BIT - log_alpha_d);
  return l + h;
}

/**
   Increases the count of a hash table to the next power of two that
   accomodates a load factor upper bound. The operation is called
   if the load factor upper bound was exceeded (i.e. num_elts + num_phs >
   max_sum) and log_count is not equal to C_LOG_COUNT_ULIMIT. A single call:
   i)  lowers the load factor s.t. num_elts + num_phs <= max_sum if a
       sufficient power of two is available, or
   ii) lowers the load factor as low as possible.
   The count is doubled at least once. If 2**C_LOG_COUNT_ULIMIT is reached
   log_count is set to C_LOG_COUNT_ULIMIT.
*/
static void ht_grow(struct ht_muloa *ht){
  size_t i, prev_count = ht->count;
  struct ke **prev_key_elts = ht->key_elts;
  struct ke * const *ke = NULL;
  while (ht->num_elts + ht->num_phs > ht->max_sum && incr_count(ht));
  ht->max_num_probes = 1;
  ht->num_phs = 0;
  ht->key_elts = malloc_perror(ht->count, sizeof(struct ke *));
  for (i = 0; i < ht->count; i++){
    ht->key_elts[i] = NULL;
  }
  for (i = 0; i < prev_count; i++){
    ke = &prev_key_elts[i];
    if (*ke != NULL && !is_ph(*ke)){
      reinsert(ht, *ke);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
}
		      
/**
   Attempts to increase the count of a hash table. Returns 1 if the count
   was increased. Otherwise returns 0. Updates count, log_count, and max_sum
   of the hash table accordingly. If 2**C_LOG_COUNT_ULIMIT is reached,
   log_count is set to C_LOG_COUNT_ULIMIT.
*/
static int incr_count(struct ht_muloa *ht){
  if (ht->log_count == C_LOG_COUNT_ULIMIT) return 0;
  ht->log_count++;
  ht->count <<= 1;
  ht->max_sum = mul_alpha(ht->count, ht->alpha_n, ht->log_alpha_d);
  /* 0 <= max_sum < count; count >= 2**C_LOG_COUNT_LLIMIT */
  if (ht->max_sum == ht->count) ht->max_sum = ht->count - 1;
  return 1;
}

/**
   Eliminates the placeholders left by delete and remove operations.
   If called when num_elts < num_placeholders, then for every delete/remove 
   operation at most one rehashing operation is performed, resulting in a 
   constant overhead of at most one rehashing per delete/remove operation.
*/
static void ht_clean(struct ht_muloa *ht){
  size_t i;
  struct ke **prev_key_elts = ht->key_elts;
  struct ke * const *ke = NULL;
  ht->max_num_probes = 1;
  ht->num_phs = 0;
  ht->key_elts = malloc_perror(ht->count, sizeof(struct ke *));
  for (i = 0; i < ht->count; i++){
    ht->key_elts[i] = NULL;
  }
  for (i = 0; i < ht->count; i++){
    ke = &prev_key_elts[i];
    if (*ke != NULL && !is_ph(*ke)){
      reinsert(ht, *ke);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
}

/**
   Reinserts a key and an associated element into a new hash table during 
   ht_grow and ht_clean operations by recomputing the hash values with 
   bit shifting and without multiplication.
*/
static void reinsert(struct ht_muloa *ht, const struct ke *prev_ke){
  size_t num_probes = 1;
  size_t ix, dist;
  struct ke **ke = NULL;
  ix = prev_ke->fval >> (C_FULL_BIT - ht->log_count);
  dist = adjust_dist(prev_ke->sval >> (C_FULL_BIT - ht->log_count));
  ke = &ht->key_elts[ix];
  while (*ke != NULL){
    ix = sum_mod(dist, ix, ht->count);
    ke = &ht->key_elts[ix];
    num_probes++;
    if (num_probes > ht->max_num_probes) ht->max_num_probes++;
  }
  *ke = (struct ke *)prev_ke;
}

/**
   Tests if a prime number in the C_FIRST_PRIME_PARTS or C_SECOND_PRIME_PARTS
   array results in an overflow of size_t on a given system. Returns 0 if no
   overflow, otherwise returns 1.
*/
static int is_overflow(const size_t *parts, size_t start, size_t count){
  size_t c = 0;
  size_t n_shift;
  n_shift = parts[start + (count - 1)];
  while (n_shift){
    n_shift >>= 1;
    c++;
  }
  return (c + (count - 1) * C_BUILD_SHIFT > C_FULL_BIT);
}

/**
   Builds a prime number from parts in the C_FIRST_PRIME_PARTS or
   C_SECOND_PRIME_PARTS array.
*/
static size_t build_prime(const size_t *parts, size_t start, size_t count){
  size_t p = 0;
  size_t n_shift;
  size_t i;
  for (i = 0; i < count; i++){
    n_shift = parts[start + i];
    n_shift <<= (i * C_BUILD_SHIFT);
    p |= n_shift;
  }
  return p;
}

/**
   Finds and builds a prime number p, s.t. 2**(n - 1) < p < 2**n where
   n is size_t width, from parts in the C_FIRST_PRIME_PARTS or
   C_SECOND_PRIME_PARTS array.
*/
static size_t find_build_prime(const size_t *parts){
  size_t p;
  size_t i = 0, j = 0;
  p = build_prime(parts, i, C_PARTS_PER_PRIME[j]);
  i += C_PARTS_PER_PRIME[j];
  if (i == C_PARTS_ACC_COUNTS[j]) j++;
  while (i <= C_LAST_PRIME_IX &&
	 !is_overflow(parts, i, C_PARTS_PER_PRIME[j])){
    p = build_prime(parts, i, C_PARTS_PER_PRIME[j]);
    i += C_PARTS_PER_PRIME[j];
    if (i == C_PARTS_ACC_COUNTS[j]) j++;
  }
  return p;
}
