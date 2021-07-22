/**
   ht-muloa.c

   A hash table with generic hash keys and generic elements. The 
   implementation is based on a multiplication method for hashing into upto 
   2**(CHAR_BIT * sizeof(size_t) - 1) slots and an open addressing method
   with double hashing for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by 
   the alpha parameter. The expected number of probes in a search is 
   upper-bounded by 1/(1 - alpha), under the uniform hashing assumption. 

   The alpha parameter does not provide an upper bound after the maximum 
   count of slots in a hash table is reached. After exceeding the alpha
   parameter value, the load factor is <= 1.0 due to open addressing, and the
   expected number of probes is upper-bounded by 1/(1 - load factor) before
   the full occupancy is reached.

   A hash key is an object within a contiguous block of memory (e.g. a basic
   type, array, struct). If the key size is greater than sizeof(size_t)
   bytes, then it is reduced to a sizeof(size_t)-byte block prior to hashing.
   Key size reduction methods may introduce regularities. An element is
   within a contiguous or noncontiguous block of memory.

   The implementation only uses integer and pointer operations. Integer
   arithmetic is used in load factor operations, thereby eliminating the
   use of float. Given parameter values within the specified ranges,
   the implementation provides an error message and an exit is executed
   if an integer overflow is attempted* or an allocation is not completed
   due to insufficient resources. The behavior outside the specified
   parameter ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99 with the only requirement that CHAR_BIT * sizeof(size_t) is
   greater or equal to 16 and is even (every bit is required to participate
   in the value at this time).

   * except intended wrapping around of unsigned integers in modulo
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

static const size_t C_LAST_PRIME_IX = 1 + 8 * (2 + 3 + 4) - 4;
static const size_t C_PARTS_PER_PRIME[4] = {1, 2, 3, 4};
static const size_t C_PARTS_ACC_COUNTS[4] = {1,
					     1 + 8 * 2,
					     1 + 8 * (2 + 3),
					     1 + 8 * (2 + 3 + 4)};
static const size_t C_BUILD_SHIFT = 16;
static const size_t C_BYTE_BIT = CHAR_BIT;
static const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);
static const size_t C_LOG_COUNT_MIN = 8; /* > 0 */
static const size_t C_LOG_COUNT_MAX = CHAR_BIT * sizeof(size_t) - 1;

/* placeholder handling */
static ke_t *ph_new();
static int is_ph(const ke_t *ke);
static void ph_free(ke_t *ke);

/* key element handling */
static ke_t *ke_new(const ht_muloa_t *ht,
		    size_t fval,
		    size_t sval,
		    const void *key,
		    const void *elt);
static void ke_elt_update(const ht_muloa_t *ht, ke_t *ke, const void *elt);
static void *ke_key_ptr(const ht_muloa_t *ht, const ke_t *ke);
static void *ke_elt_ptr(const ht_muloa_t *ht, const ke_t *ke);
static void ke_free(const ht_muloa_t *ht, ke_t *ke);

/* hashing */
static size_t convert_std_key(const ht_muloa_t *ht, const void *key);
static size_t adjust_dist(size_t dist);

/* hash table operations and maintenance*/
static ke_t **search(const ht_muloa_t *ht, const void *key);
static size_t mul_alpha(size_t n, size_t alpha_n, size_t log_alpha_d);
static int incr_count(ht_muloa_t *ht);
static void ht_grow(ht_muloa_t *ht);
static void ht_clean(ht_muloa_t *ht);
static void reinsert(ht_muloa_t *ht, const ke_t *prev_ke);

/* integer constant construction */
static size_t find_build_prime(const size_t *parts);

/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size
                 sizeof(ht_muloa_t).
   key_size    : non-zero size of a key object.
   elt_size    : - non-zero size of an element, if the element is within a
                 contiguous memory block and a copy of the element is
                 inserted,
                 - size of a pointer to an element, if the element
                 is within a noncontiguous memory block or a pointer to a
                 contiguous element is inserted
   min_num     : minimum number of keys that are known or expected to become 
                 present simultaneously in a hash table, resulting in a
                 speedup by avoiding unnecessary growth steps of a hash
                 table; 0 if a positive value is not specified and all growth
                 steps are to be completed
   alpha_n     : > 0 numerator of load factor upper bound
   log_alpha_d : < CHAR_BIT * sizeof(size_t) log base 2 of denominator of
                 load factor upper bound; denominator is a power of two and
                 is greater or equal to alpha_n
   cmp_key     : - if NULL then a default memcmp-based comparison of keys
                 is performed
                 - otherwise comparison function is applied which returns a
                 zero integer value iff the two keys accessed through the
                 first and the second arguments are equal; each argument is
                 a pointer to a key_size block
   rdc_key     : - if NULL then a default conversion of a bit pattern
                 in the block pointed to by key is performed prior to
                 hashing, which may introduce regularities
                 - otherwise rdc_key is applied to a key prior to hashing;
                 the first argument points to a key and the second argument
                 provides the size of the key
   free_elt    : - if an element is within a contiguous memory block and
                 a copy of the element was inserted, then NULL as free_elt
                 is sufficient to delete the element,
                 - if an element is within a noncontiguous memory block or
                 a pointer to a contiguous element was inserted, then an
                 element-specific free_elt, taking a pointer to a pointer to an
                 element as its argument and leaving a block of size elt_size
                 pointed to by the argument, is necessary to delete the element
*/
void ht_muloa_init(ht_muloa_t *ht,
		   size_t key_size,
		   size_t elt_size,
		   size_t min_num,
		   size_t alpha_n,
		   size_t log_alpha_d,
		   int (*cmp_key)(const void *, const void *),
		   size_t (*rdc_key)(const void *, size_t),
		   void (*free_elt)(void *)){
  size_t i, rem;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  /* align ke_t relative to a malloc's pointer */
  if (key_size <= sizeof(size_t)){
    ht->key_offset = sizeof(size_t);
  }else{
    rem = key_size % sizeof(size_t);
    ht->key_offset = key_size;
    ht->key_offset = add_sz_perror(ht->key_offset,
				   (rem > 0) * (sizeof(size_t) - rem));
  }
  /* elt_size block accessible with a character pointer */
  ht->elt_offset = sizeof(ke_t);
  ht->elt_alignment = 1;
  ht->log_count = C_LOG_COUNT_MIN;
  ht->count = pow_two_perror(C_LOG_COUNT_MIN);
  /* 0 <= max_sum < count */
  ht->max_sum = mul_alpha(ht->count, alpha_n, log_alpha_d);
  if (ht->max_sum == ht->count) ht->max_sum = ht->count - 1;
  while (min_num > ht->max_sum && incr_count(ht));
  ht->max_num_probes = 1; /* at least one probe */
  ht->num_elts = 0;
  ht->num_phs = 0;
  ht->fprime = find_build_prime(C_FIRST_PRIME_PARTS);
  ht->sprime = find_build_prime(C_SECOND_PRIME_PARTS);
  ht->alpha_n = alpha_n;
  ht->log_alpha_d = log_alpha_d;
  ht->ph = ph_new();
  ht->key_elts = malloc_perror(ht->count, sizeof(ke_t *));
  for (i = 0; i < ht->count; i++){
    ht->key_elts[i] = NULL;
  }
  ht->cmp_key = cmp_key;
  ht->rdc_key = rdc_key;
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
   type. The operation is optionally called after ht_muloa_init is
   completed and before any other operation is called.
   ht          : pointer to an initialized ht_muloa_t struct
   alignment   : alignment requirement or size of the type, a pointer to
                 which is used to access an elt_size block
*/
void ht_muloa_align_elt(ht_muloa_t *ht, size_t alignment){
  size_t alloc_ptr_offset = add_sz_perror(ht->key_offset, ht->elt_offset);
  size_t rem;
  /* elt_offset to align elt_size block relative to malloc's pointer */
  if (alloc_ptr_offset <= alignment){
    ht->elt_offset = add_sz_perror(ht->elt_offset,
				   alignment - alloc_ptr_offset);
  }else{
    rem = alloc_ptr_offset % alignment;
    ht->elt_offset = add_sz_perror(ht->elt_offset,
				   (rem > 0) * (alignment - rem));
  }
}

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL and point to blocks of size key_size and
   elt_size respectively.
*/
void ht_muloa_insert(ht_muloa_t *ht, const void *key, const void *elt){
  size_t num_probes = 1;
  size_t std_key;
  size_t fval, sval;
  size_t ix, dist;
  ke_t **ke = NULL;
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
    }else if (ht->log_count < C_LOG_COUNT_MAX){
      ht_grow(ht);
    }
  }
}

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL and points
   to a block of size key_size. The returned pointer can be dereferenced
   according to ht_muloa_init and ht_muloa_align_elt.
*/
void *ht_muloa_search(const ht_muloa_t *ht, const void *key){
  ke_t * const *ke = search(ht, key);
  if (ke != NULL){
    return ke_elt_ptr(ht, *ke);
  }else{
    return NULL;
  }
}

/**
   Removes a key and its associated element from a hash table by copying 
   the element or its pointer into a block of size elt_size pointed to
   by elt. If the key is not in the hash table, leaves the block pointed
   to by elt unchanged. The key and elt parameters are not NULL and point
   to blocks of size key_size and elt_size respectively.
*/
void ht_muloa_remove(ht_muloa_t *ht, const void *key, void *elt){
  ke_t **ke = search(ht, key);
  if (ke != NULL){
    memcpy(elt, ke_elt_ptr(ht, *ke), ht->elt_size);
    /* if an element is noncontiguous, only the pointer to it is deleted */
    free(ke_key_ptr(ht, *ke));
    *ke = ht->ph;
    ht->num_elts--;
    ht->num_phs++;
  }
}

/**
   If a key is in a hash table, deletes the key and its associated element 
   according to free_elt. The key parameter is not NULL and points
   to a block of size key_size.
*/
void ht_muloa_delete(ht_muloa_t *ht, const void *key){
  ke_t **ke = search(ht, key);
  if (ke != NULL){
    ke_free(ht, *ke);
    *ke = ht->ph;
    ht->num_elts--;
    ht->num_phs++;
  }
}

/**
   Frees a hash table and leaves a block of size sizeof(ht_muloa_t)
   pointed to by the ht parameter.
*/
void ht_muloa_free(ht_muloa_t *ht){
  size_t i;
  ke_t * const *ke = NULL;
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
   (qualified) ht_muloa_t *p0 is converted to (qualified) void * and back
   to a (qualified) ht_muloa_t *p1, thus guaranteeing that the value of p0
   equals the value of p1. An initialization helper is constructed by the
   user. 
*/

void ht_muloa_align_elt_helper(void *ht, size_t alignment){
  ht_muloa_align_elt(ht, alignment);
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

static ke_t *ph_new(){
  ke_t *ke = malloc_perror(1, sizeof(ke_t));
  ke->fval = 1;
  ke->sval = 0;
  return ke;
}

static int is_ph(const ke_t *ke){
  return (ke->fval == 1);
}

static void ph_free(ke_t *ke){
  free(ke);
  ke = NULL;
}

/**
   Create, update, and free a key element. These functions cannot be used on
   a placeholder.
*/

static ke_t *ke_new(const ht_muloa_t *ht,
		    size_t fval,
		    size_t sval,
		    const void *key,
		    const void *elt){
  void *ke_block = NULL;
  ke_t *ke = NULL;
  ke_block =  
    malloc_perror(1, add_sz_perror(ht->key_offset,
				   add_sz_perror(ht->elt_offset,
						 ht->elt_size)));
  ke = (ke_t *)((char *)ke_block + ht->key_offset);
  ke->fval = fval;
  ke->sval = sval;
  memcpy(ke_key_ptr(ht, ke), key, ht->key_size);
  memcpy(ke_elt_ptr(ht, ke), elt, ht->elt_size);
  return ke;
}

static void ke_elt_update(const ht_muloa_t *ht, ke_t *ke, const void *elt){
  if (ht->free_elt != NULL) ht->free_elt(ke_elt_ptr(ht, ke));
  memcpy(ke_elt_ptr(ht, ke), elt, ht->elt_size);
}

static void *ke_key_ptr(const ht_muloa_t *ht, const ke_t *ke){
  return (void *)((char *)ke - ht->key_offset);
}

static void *ke_elt_ptr(const ht_muloa_t *ht, const ke_t *ke){
  return (void *)((char *)ke + ht->elt_offset);
}

static void ke_free(const ht_muloa_t *ht, ke_t *ke){
  if (ht->free_elt != NULL) ht->free_elt(ke_elt_ptr(ht, ke));
  free(ke_key_ptr(ht, ke));
  ke = NULL;
}

/**
   Converts a key to a key of the standard size. This is a safe conversion
   of any bit pattern in the block pointed to by key to size_t.
*/
static size_t convert_std_key(const ht_muloa_t *ht, const void *key){
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
   in the key_elts array that stores a pointer to ke_t with the
   key, otherwise returns NULL.
*/
static ke_t **search(const ht_muloa_t *ht, const void *key){
  size_t num_probes = 1;
  size_t std_key, fval, sval, ix, dist;
  ke_t * const *ke = NULL;
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
      return (ke_t **)ke;
    }else if (ht->cmp_key == NULL && /* loop invariant */
	      !is_ph(*ke) &&
	      memcmp(ke_key_ptr(ht, *ke), key, ht->key_size) == 0){
      return (ke_t **)ke;
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
   accomodates alpha as a load factor upper bound. The operation is called
   if alpha was exceeded (i.e. num_elts + num_phs > max_sum) and log_count
   is not equal to C_LOG_COUNT_MAX. A single call:
   i)  lowers the load factor s.t. num_elts + num_phs <= max_sum if a
       sufficient power of two is available, or
   ii) lowers the load factor as low as possible.
   The count is doubled at least once. If 2**C_LOG_COUNT_MAX is reached
   log_count is set to C_LOG_COUNT_MAX.
*/
static void ht_grow(ht_muloa_t *ht){
  size_t i, prev_count = ht->count;
  ke_t **prev_key_elts = ht->key_elts;
  ke_t * const *ke = NULL;
  while (ht->num_elts + ht->num_phs > ht->max_sum && incr_count(ht));
  ht->max_num_probes = 1;
  ht->num_phs = 0;
  ht->key_elts = malloc_perror(ht->count, sizeof(ke_t *));
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
   of the hash table accordingly. If 2**C_LOG_COUNT_MAX is reached, log_count
   is set to C_LOG_COUNT_MAX.
*/
static int incr_count(ht_muloa_t *ht){
  if (ht->log_count == C_LOG_COUNT_MAX) return 0;
  ht->log_count++;
  ht->count <<= 1;
  ht->max_sum = mul_alpha(ht->count, ht->alpha_n, ht->log_alpha_d);
  /* 0 <= max_sum < count; count >= 2**C_LOG_COUNT_MIN */
  if (ht->max_sum == ht->count) ht->max_sum = ht->count - 1;
  return 1;
}

/**
   Eliminates the placeholders left by delete and remove operations.
   If called when num_elts < num_placeholders, then for every delete/remove 
   operation at most one rehashing operation is performed, resulting in a 
   constant overhead of at most one rehashing per delete/remove operation.
*/
static void ht_clean(ht_muloa_t *ht){
  size_t i;
  ke_t **prev_key_elts = ht->key_elts;
  ke_t * const *ke = NULL;
  ht->max_num_probes = 1;
  ht->num_phs = 0;
  ht->key_elts = malloc_perror(ht->count, sizeof(ke_t *));
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
static void reinsert(ht_muloa_t *ht, const ke_t *prev_ke){
  size_t num_probes = 1;
  size_t ix, dist;
  ke_t **ke = NULL;
  ix = prev_ke->fval >> (C_FULL_BIT - ht->log_count);
  dist = adjust_dist(prev_ke->sval >> (C_FULL_BIT - ht->log_count));
  ke = &ht->key_elts[ix];
  while (*ke != NULL){
    ix = sum_mod(dist, ix, ht->count);
    ke = &ht->key_elts[ix];
    num_probes++;
    if (num_probes > ht->max_num_probes) ht->max_num_probes++;
  }
  *ke = (ke_t *)prev_ke;
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
   n = CHAR_BIT * sizeof(size_t), from parts in the C_FIRST_PRIME_PARTS or
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
