/**
   ht-mul.c

   A hash table with generic hash keys and generic elements. The 
   implementation is based on a multiplication method for hashing into upto 
   2^{CHAR_BIT * sizeof(size_t) - 1} slots and an open addressing method
   with double hashing for resolving collisions.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by 
   the alpha parameter. The expected number of probes in a search is 
   upper-bounded by 1/(1 - alpha), under the uniform hashing assumption. 

   The alpha parameter does not provide an upper bound after the maximum 
   count of slots in a hash table is reached. After exceeding the alpha
   parameter, the load factor is <= 1.0 due to open addressing, and the
   expected number of probes is upper-bounded by 1/(1 - load factor) before
   the full occupancy is reached.

   A hash key is an object within a contiguous block of memory (e.g. a basic
   type, array, struct). If the key size is greater than sizeof(size_t)
   bytes, then it is reduced to a sizeof(size_t)-byte block prior to hashing.
   Key size reduction methods may introduce regularities. An element is
   within a contiguous or noncontiguous block of memory.

   The implementation does not use stdint.h and is portable under C89/C90
   with the only requirement that CHAR_BIT * sizeof(size_t) is greater or
   equal to 16 and is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ht-mul.h"
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

static const size_t C_FIRST_PRIME_PARTS[1 + 8 * (2 + 3 + 4)] =
  {0xbe21u,                            /* 2^15 < 48673 < 2^16 */
   0xd8d5u, 0x0002u,                   /* 2^17 < 186581 < 2^18 */
   0x0077u, 0x000cu,                   /* 2^19 < 786551 < 2^20 */
   0x2029u, 0x0031u,                   /* 2^21 < 3219497 < 2^22 */
   0x5427u, 0x00bfu,                   /* 2^23 < 12538919 < 2^24 */
   0x42bbu, 0x030fu,                   /* 2^25 < 51331771 < 2^26 */
   0x96adu, 0x0c98u,                   /* 2^27 < 211326637 < 2^28 */
   0xc10fu, 0x2ecfu,                   /* 2^29 < 785367311 < 2^30 */
   0x0007u, 0xc000u,                   /* 2^31 < 3221225479 < 2^32 */
   0x9345u, 0xffc8u, 0x0002u,          /* 2^33 < 12881269573 < 2^34 */
   0x1575u, 0x0a63u, 0x000cu,          /* 2^35 < 51713873269 < 2^36 */
   0xc513u, 0x4d6bu, 0x0031u,          /* 2^37 < 211752305939 < 2^38 */
   0xa021u, 0x5460u, 0x00beu,          /* 2^39 < 817459404833 < 2^40 */
   0xeaafu, 0x7c3du, 0x02f5u,          /* 2^41 < 3253374675631 < 2^42 */
   0x6b1fu, 0x29efu, 0x0c24u,          /* 2^43 < 13349461912351 < 2^44 */
   0x57b7u, 0xccbeu, 0x2ffbu,          /* 2^45 < 52758518323127 < 2^46 */
   0x82c3u, 0x2c9fu, 0xc2ccu,          /* 2^47 < 214182177768131 < 2^48 */
   0x60adu, 0x46a1u, 0xf55eu, 0x0002u, /* 2^49 < 832735214133421 < 2^50 */
   0xb24du, 0x6765u, 0x38b5u, 0x000bu, /* 2^51 < 3158576518771277 < 2^52 */
   0x0d35u, 0x5443u, 0xff54u, 0x0030u, /* 2^53 < 13791536538127669 < 2^54 */
   0xd017u, 0x90c7u, 0x37b3u, 0x00c6u, /* 2^55 < 55793289756397591 < 2^56 */
   0x6f8fu, 0x423bu, 0x8949u, 0x0304u, /* 2^57 < 217449629757435791 < 2^58 */
   0xbbc1u, 0x662cu, 0x4d90u, 0x0badu, /* 2^59 < 841413987972987841 < 2^60 */
   0xc647u, 0x3c91u, 0x46b2u, 0x2e9bu, /* 2^61 < 3358355678469146183 < 2^62 */
   0x8969u, 0x4c70u, 0x6dbeu, 0xdad8u  /* 2^63 < 15769474759331449193 < 2^64 */
  }; 

static const size_t C_SECOND_PRIME_PARTS[1 + 8 * (2 + 3 + 4)] =
  {0xc221u,                            /* 2^15 < 49697 < 2^16 */
   0xe04bu, 0x0002u,                   /* 2^17 < 188491 < 2^18 */
   0xf6a7u, 0x000bu,                   /* 2^19 < 784039 < 2^20 */
   0x1b4fu, 0x0030u,                   /* 2^21 < 3152719 < 2^22 */
   0x4761u, 0x00beu,                   /* 2^23 < 12470113 < 2^24 */
   0x3eadu, 0x0312u,                   /* 2^25 < 51527341 < 2^26 */
   0x08e9u, 0x0ca5u,                   /* 2^27 < 212142313 < 2^28 */
   0x06b9u, 0x2eecu,                   /* 2^29 < 787220153 < 2^30 */
   0xbe7du, 0xc073u,                   /* 2^31 < 3228810877 < 2^32 */
   0x3739u, 0xf7fdu, 0x0002u,          /* 2^33 < 12750501689 < 2^34 */
   0x852bu, 0x07f8u, 0x000cu,          /* 2^35 < 51673335083 < 2^36 */
   0xa61bu, 0x457au, 0x0031u,          /* 2^37 < 211619063323 < 2^38 */
   0xb041u, 0xbf9eu, 0x00bdu,          /* 2^39 < 814963667009 < 2^40 */
   0x4515u, 0x3eafu, 0x0308u,          /* 2^41 < 3333946295573 < 2^42 */
   0x6f4fu, 0xc0d9u, 0x0c3cu,          /* 2^43 < 13455073046351 < 2^44 */
   0x0da1u, 0x6600u, 0x3025u,          /* 2^45 < 52937183202721 < 2^46 */
   0xb229u, 0x8facu, 0xc1e5u,          /* 2^47 < 213191702131241 < 2^48 */
   0x58f1u, 0x94e9u, 0xff18u, 0x0002u, /* 2^49 < 843430996039921 < 2^50 */
   0x73abu, 0xda62u, 0x9da8u, 0x000bu, /* 2^51 < 3269573287769003 < 2^52 */
   0x37f1u, 0xd800u, 0x135bu, 0x0031u, /* 2^53 < 13813559045666801 < 2^54 */
   0xd909u, 0xa518u, 0xebc1u, 0x00c4u, /* 2^55 < 55428312366373129 < 2^56 */
   0x03a7u, 0x5cb0u, 0xba89u, 0x0302u, /* 2^57 < 216940831195530151 < 2^58 */
   0x12adu, 0x7477u, 0xb251u, 0x0c10u, /* 2^59 < 869390790998561453 < 2^60 */
   0xe411u, 0x4bacu, 0x9c82u, 0x2f17u, /* 2^61 < 3393352927676261393 < 2^62 */
   0xd047u, 0x33a5u, 0x5cb7u, 0xbd8fu  /* 2^63 < 13659238136753279047 < 2^64 */
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
static const size_t C_FULL_SIZE = sizeof(size_t);
static const size_t C_SIZE_MAX = (size_t)-1;
static const size_t C_INIT_LOG_COUNT = 8;

/* placeholder object handling */
static void placeholder_init(dll_node_t *node, int elt_size);
static int is_placeholder(const dll_node_t *node);
static void placeholder_free(dll_node_t *node);

/* hashing */
static size_t find_build_prime(const size_t *parts);
static size_t convert_std_key(const ht_mul_t *ht, const void *key);
static size_t hash(size_t prime, size_t std_key);
static size_t adjust_hash_dist(size_t dist);
static size_t probe_dbl_hash(const ht_mul_t *ht, size_t dist, size_t ix);

/* hash table operations */
static dll_node_t **search(const ht_mul_t *ht, const void *key);
static size_t *first_val_ptr(const void *key_block, size_t key_size);
static size_t *second_val_ptr(const void *key_block, size_t key_size);

/* hash table maintenance */
static void ht_grow(ht_mul_t *ht);
static void ht_clean(ht_mul_t *ht);
static void reinsert(ht_mul_t *ht, const dll_node_t *node);

/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size sizeof(ht_mul_t).
   key_size    : size of a key object
   elt_size    : - size of an element, if the element is within a contiguous
                 memory block,
                 - size of a pointer to an element, if the element is within
                 a noncontiguous memory block
   alpha       : a load factor upper bound that is > 0.0 and < 1.0
   rdc_key     : - if key_size is less or equal to sizeof(size_t) bytes, then
                 rdc_key is NULL
                 - if key_size is greater than sizeof(size_t) bytes, then
                 rdc_key is not NULL and reduces a key to a sizeof(size_t)-
                 byte block prior to hashing; the first argument points to an
                 sizeof(size_t)-byte block, where the reduced form of the
                 block pointed to by the second argument is copied
   free_elt    : - if an element is within a contiguous memory block,
                 as reflected by elt_size, and a pointer to the element is 
                 passed as elt in ht_mul_insert, then the element is
                 fully copied into a hash table, and NULL as free_elt is
                 sufficient to delete the element,
                 - if an element is within a noncontiguous memory block,
                 and a pointer to a pointer to the element is passed
                 as elt in ht_mul_insert, then the pointer to the
                 element is copied into the hash table, and an element-
                 specific free_elt, taking a pointer to a pointer to an
                 element as its argument, is necessary to delete the element
*/
void ht_mul_init(ht_mul_t *ht,
		 size_t key_size,
		 size_t elt_size,
		 float alpha,
		 void (*rdc_key)(void *, const void *),
		 void (*free_elt)(void *)){
  size_t i;
  ht->log_count = C_INIT_LOG_COUNT;
  ht->key_size = key_size;
  ht->elt_size = elt_size;
  ht->count = pow_two(ht->log_count);
  ht->max_count = pow_two(C_FULL_BIT - 1);
  ht->max_num_probes = 1; /* at least one probe */
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->alpha = alpha;
  ht->placeholder = malloc_perror(sizeof(dll_node_t));
  placeholder_init(ht->placeholder, elt_size);
  ht->first_prime = find_build_prime(C_FIRST_PRIME_PARTS);
  ht->second_prime = find_build_prime(C_SECOND_PRIME_PARTS);
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    dll_init(&ht->key_elts[i]);
  }
  ht->rdc_key = rdc_key;
  ht->free_elt = free_elt;
}

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL.
*/
void ht_mul_insert(ht_mul_t *ht, const void *key, const void *elt){
  size_t num_probes = 1;
  size_t std_key, first_val, second_val, ix, dist;
  size_t key_block_size = ht->key_size + 2 * C_FULL_SIZE;
  void *key_block = NULL;
  dll_node_t **head = NULL;
  while ((float)(ht->num_elts + ht->num_placeholders) / ht->count >
	 ht->alpha){
    /* clean or grow if E[# keys in a slot] > alpha */
    if (ht->num_elts < ht->num_placeholders){
      ht_clean(ht);
    }else if (ht->count < ht->max_count){
      ht_grow(ht);
    }else{
      break;
    }
  }
  std_key = convert_std_key(ht, key);
  first_val = hash(ht->first_prime, std_key); 
  second_val = hash(ht->second_prime, std_key);
  /* prepare a hash key and two hash values for storage as a block */
  key_block = malloc_perror(key_block_size);
  memcpy(key_block, key, ht->key_size);
  *first_val_ptr(key_block, ht->key_size) = first_val;
  *second_val_ptr(key_block, ht->key_size) = second_val;
  ix = first_val >> (C_FULL_BIT - ht->log_count);
  dist = adjust_hash_dist(second_val >> (C_FULL_BIT - ht->log_count));
  head = &ht->key_elts[ix];
  while (*head != NULL){
    if (!is_placeholder(*head) &&
	dll_search_key(head, key, ht->key_size) != NULL){
      dll_delete(head, *head, ht->free_elt);
      dll_prepend(head, key_block, elt, key_block_size, ht->elt_size);
      free(key_block);
      key_block = NULL;
      return;
    }
    ix = probe_dbl_hash(ht, dist, ix);
    head = &ht->key_elts[ix];
    num_probes++;
    if (num_probes > ht->max_num_probes){
      ht->max_num_probes++;
    }
  }
  dll_prepend(head, key_block, elt, key_block_size, ht->elt_size);
  ht->num_elts++;
  free(key_block);
  key_block = NULL;
}

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL.
*/
void *ht_mul_search(const ht_mul_t *ht, const void *key){
  dll_node_t **head = search(ht, key);
  if (head != NULL){
    return (*head)->elt;
  }else{
    return NULL;
  }
}

/**
   Removes a key and its associated element from a hash table by copying 
   the element into a block of size elt_size pointed to by elt. If the key is
   not in the hash table, leaves the block pointed to by elt unchanged.
   The key and elt parameters are not NULL.
*/
void ht_mul_remove(ht_mul_t *ht, const void *key, void *elt){
  dll_node_t **head = search(ht, key);
  if (head != NULL){
    memcpy(elt, (*head)->elt, ht->elt_size);
    /* if an element is noncontiguous, only the pointer to it is deleted */
    dll_delete(head, *head, NULL);
    *head = ht->placeholder;
    ht->num_elts--;
    ht->num_placeholders++;
  }
}

/**
   If a key is in a hash table, deletes the key and its associated element 
   according to free_elt. The key parameter is not NULL.
*/
void ht_mul_delete(ht_mul_t *ht, const void *key){
  dll_node_t **head = search(ht, key);
  if (head != NULL){
    dll_delete(head, *head, ht->free_elt);
    *head = ht->placeholder;
    ht->num_elts--;
    ht->num_placeholders++;
  }
}

/**
   Frees a hash table and leaves a block of size sizeof(ht_mul_t)
   pointed to by the ht parameter.
*/
void ht_mul_free(ht_mul_t *ht){
  size_t i;
  dll_node_t **head = NULL;
  for (i = 0; i < ht->count; i++){
    head = &ht->key_elts[i];
    if (*head != NULL && !is_placeholder(*head)){
      dll_free(head, ht->free_elt);
    }
  }
  free(ht->key_elts);
  placeholder_free(ht->placeholder);
  free(ht->placeholder);
  ht->key_elts = NULL;
  ht->placeholder = NULL;
  head = NULL;
}

/** Helper functions */

/**
   Initializes a placeholder node. The node parameter points to a
   preallocated block of size sizeof(dll_node_t).
*/
static void placeholder_init(dll_node_t *node, int elt_size){
  node->key_size = 0;
  node->elt_size = elt_size;
  node->key = NULL; /* element in ht cannot have node->key == NULL */
  node->elt = calloc_perror(1, elt_size); /* for consistency purposes */
  node->next = node;
  node->prev = node;
}

/**
   Tests if a node is a placeholder node.
*/
static int is_placeholder(const dll_node_t *node){
  if (node->key == NULL){
    return 1;
  }else{
    return 0;
  }
}

/**
   Frees a placeholder node and leaves a block of size sizeof(dll_node_t) 
   pointed to by the node parameter.
*/
static void placeholder_free(dll_node_t *node){
  free(node->elt);
  node->elt = NULL;
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
  return (c + (count - 1) * C_BYTE_BIT > C_FULL_BIT);
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
   Finds a and builds a prime number p, s.t. 2^{n - 1} < p < 2^n where
   n = CHAR_BIT * sizeof(size_t), from parts in the C_FIRST_PRIME_PARTS or
   C_SECOND_PRIME_PARTS array.
*/
static size_t find_build_prime(const size_t *parts){
  size_t p;
  size_t pi = 0, gi = 0;
  while (pi <= C_LAST_PRIME_IX &&
	 !is_overflow(parts, pi, C_PARTS_PER_PRIME[gi])){
    p = build_prime(parts, pi, C_PARTS_PER_PRIME[gi]);
    pi += C_PARTS_PER_PRIME[gi];
    if (pi == C_PARTS_ACC_COUNTS[gi]) gi++;
  }
  return p;
}

/**
   Converts a key to a key of the standard size of sizeof(size_t) bytes.
*/
static size_t convert_std_key(const ht_mul_t *ht, const void *key){
  size_t std_key = 0; /* initialize all bits */
  if (ht->key_size > sizeof(size_t)){
    ht->rdc_key(&std_key, key);
  }else{ 
    memcpy(&std_key, key, ht->key_size);
  }
  return std_key;
}

/**
   Maps a hash key to a hash value without subsequent bit shifting. The 
   latter is necessary to determine the index of a slot in a hash table or 
   a probe distance.
*/
static size_t hash(size_t prime, size_t std_key){
  return mul_mod_pow_two(prime, std_key);
}

/**
   Adjusts a probe distance to an odd distance, if necessary. 
*/
static size_t adjust_hash_dist(size_t dist){
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
   Returns the next index in a hash table, based on double hashing.
*/
static size_t probe_dbl_hash(const ht_mul_t *ht, size_t dist, size_t ix){
  return sum_mod(dist, ix, ht->count);
}

/**
   If a key is present in a hash table, returns a head pointer to the dll
   with a single node containing the key, otherwise returns NULL.
*/
static dll_node_t **search(const ht_mul_t *ht, const void *key){
  size_t num_probes = 1;
  size_t std_key, first_val, second_val, ix, dist;
  dll_node_t **head = NULL;
  std_key = convert_std_key(ht, key);
  first_val = hash(ht->first_prime, std_key); 
  second_val = hash(ht->second_prime, std_key); 
  ix = first_val >> (C_FULL_BIT - ht->log_count);
  dist = adjust_hash_dist(second_val >> (C_FULL_BIT - ht->log_count));
  head = &ht->key_elts[ix];
  while (*head != NULL){
    if (!is_placeholder(*head) &&
	dll_search_key(head, key, ht->key_size) != NULL){
      return head;
    }else if (num_probes == ht->max_num_probes){
      break;
    }else{
      ix = probe_dbl_hash(ht, dist, ix);
      head = &ht->key_elts[ix];
      num_probes++;
    }
  }
  return NULL;
}

/**
   Compute pointers to the first and second hash values in a key block.
*/
static size_t *first_val_ptr(const void* key_block, size_t key_size){
  return (size_t *)((char *)key_block + key_size);
}

static size_t *second_val_ptr(const void* key_block, size_t key_size){
  return (size_t *)((char *)key_block + key_size + C_FULL_SIZE);
}

/**
   Doubles the count of a hash table. Makes no changes if the maximum count
   has been reached.
*/
static void ht_grow(ht_mul_t *ht){
  size_t i, prev_count = ht->count;
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head = NULL;
  if (ht->count == ht->max_count) return;
  ht->log_count++;
  ht->count *= 2;
  ht->max_num_probes = 1;
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    head = &ht->key_elts[i];
    dll_init(head);
  }
  for (i = 0; i < prev_count; i++){
    head = &prev_key_elts[i];
    if (*head != NULL && !is_placeholder(*head)){
      reinsert(ht, *head);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
  head = NULL;
}

/**
   Eliminates the placeholders left by delete and remove operations.
   If called when num_elts < num_placeholders, then for every delete/remove 
   operation at most one rehashing operation is performed, resulting in a 
   constant overhead of at most one rehashing per delete/remove operation.
*/
static void ht_clean(ht_mul_t *ht){
  size_t i;
  dll_node_t **prev_key_elts = ht->key_elts;
  dll_node_t **head = NULL;
  ht->max_num_probes = 1;
  ht->num_elts = 0;
  ht->num_placeholders = 0;
  ht->key_elts = malloc_perror(ht->count * sizeof(dll_node_t *));
  for (i = 0; i < ht->count; i++){
    head = &ht->key_elts[i];
    dll_init(head);
  }
  for (i = 0; i < ht->count; i++){
    head = &prev_key_elts[i];
    if (*head != NULL && !is_placeholder(*head)){
      reinsert(ht, *head);
    }
  }
  free(prev_key_elts);
  prev_key_elts = NULL;
  head = NULL;
}

/**
   Reinserts a key and an associated element into a new hash table during 
   ht_grow and ht_clean operations by recomputing the hash values with 
   bit shifting and without multiplication.
*/
static void reinsert(ht_mul_t *ht, const dll_node_t *node){
  size_t num_probes = 1;
  size_t first_val, second_val, ix, dist;
  dll_node_t **head = NULL;
  first_val = *first_val_ptr(node->key, ht->key_size);
  second_val = *second_val_ptr(node->key, ht->key_size);
  ix = first_val >> (C_FULL_BIT - ht->log_count);
  dist = adjust_hash_dist(second_val >> (C_FULL_BIT - ht->log_count));
  head = &ht->key_elts[ix];
  while (*head != NULL){
    ix = probe_dbl_hash(ht, dist, ix);
    head = &ht->key_elts[ix];
    num_probes++;
    if (num_probes > ht->max_num_probes){
      ht->max_num_probes++;
    }
  }
  *head = (dll_node_t *)node;
  ht->num_elts++;
}
