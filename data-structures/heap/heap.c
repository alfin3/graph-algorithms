/**
   heap.c

   A (min) heap with generic contiguous priorities, generic contiguous or
   non-contiguous elements, and a hash table parameter. 

   The implementation provides a dynamic set in the min heap form for any
   elements in memory associated with contiguous priority values (e.g. basic
   types). The only requirement is that no two elements in the heap compare
   equal according to a user-defined cmp_elt function.

   The hash table parameter specifies a hash table used for in-heap
   search and modifications, and enables the optimization of space and
   time resources associated with heap operations by choice of a hash
   table, its load factor upper bound, and a known or expected minimum
   number of simultaneously present elements.

   A distinction is made between an element and an "elt_size block". During
   an insertion (push op), a block of size pty_size ("pty_size block")
   containing an entire priority value and a block of size elt_size
   ("elt_size block") are copied into the heap. In contrast to a priority
   that is always contiguous and is copied entirely, an element may be
   within a contiguous or non-contiguous memory block. Given an element,
   the user decides what is copied into the elt_size block of the heap.
   If the element is within a contiguous memory block, then it can be
   entirely copied as an elt_size block, or a pointer to it can be copied
   as an elt_size block. If the element is within a non-contiguous memory
   block, then a pointer to it is copied as an elt_size block.

   When a pointer to an element is copied into a heap as an elt_size block,
   the user can also decide if only the pointer or the entire element is
   deleted during the free operation. By setting free_elt to NULL, only
   the pointer is deleted. Otherwise, the deletion is performed according to
   a non-NULL free_elt. For example, when an in-memory set of images are
   used as elements (e.g. with a subset of bits in each image used for
   hashing according to rdc_elt) and pointers are copied into the heap,
   then setting free_elt to NULL will not affect the original set of images
   throughout the lifetime of the heap.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted* or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heap.h"
#include "utilities-mem.h"

static void half_swap(struct heap *h, size_t t, size_t s);
static void heapify_up(struct heap *h, size_t i);
static void heapify_down(struct heap *h, size_t i);
static void *pty_ptr(const struct heap *h, size_t i);
static void *elt_ptr(const struct heap *h, size_t i);

/**
   Initializes a heap. The in-heap pty_size and elt_size blocks, as well as
   the size_t indices in a hash table are aligned by default according to their
   sizes because size of a type T >= alignment requirement of T (due to the
   structure of arrays).
   h           : pointer to a preallocated block of size sizeof(struct heap)
   pty_size    : non-zero size of a pty_size block; must account for internal
                 and trailing padding according or equivalent to sizeof
   elt_size    : non-zero size of an elt_size block; must account for internal
                 and trailing padding according or equivalent to sizeof
   min_num     : > 0 minimum number of elements that are known to be or
                 expected to be present simultaneously in a heap; may result
                 in a speedup by avoiding the unnecessary growth steps of the
                 hash table
   hht         : a non-NULL pointer to a set of parameters specifying a
                 hash table for in-heap search and modifications
   cmp_pty     : comparison function which returns a negative integer value
                 if the priority value pointed to by the first argument is
                 less than the priority value pointed to by the second, a
                 positive integer value if the priority value pointed to by
                 the first argument is greater than the priority value 
                 pointed to by the second, and zero integer value if the two
                 priority values are equal
   cmp_elt     : - if NULL then a default memcmp-based comparison of elt_size
                 blocks of elements is performed in hash table operations
                 - otherwise comparison function is applied which returns a
                 zero integer value iff the two elements accessed through the
                 first and the second arguments are equal; each argument is
                 a pointer to the elt_size block of an element; cmp_elt must
                 use the same subset of bits in an element as rdc_elt
   rdc_elt     : - if NULL then a default conversion of a bit pattern
                 in the elt_size block of an element is performed prior to
                 hashing, which may introduce regularities
                 - otherwise rdc_elt is applied to an element to reduce the
                 element to a size_t integer value prior to hashing; the
                 argument points to the elt_size block of an element; rdc_elt
                 must use the same subset of bits in an element as cmp_elt
   free_elt    : - NULL if only elt_size blocks should be deleted throughout
                 the lifetime of the heap (e.g. because elements were
                 entirely copied as elt_size blocks, or because pointers
                 were copied as elt_size blocks and only pointers should
                 be deleted)
                 - otherwise takes a pointer to the elt_size block of an
                 element as an argument, frees the memory of the element
                 except the elt_size block pointed to by the argument
*/
void heap_init(struct heap *h,
	       size_t pty_size,
	       size_t elt_size,
	       size_t min_num,
	       const struct heap_ht *hht,
	       int (*cmp_pty)(const void *, const void *),
	       int (*cmp_elt)(const void *, const void *),
	       size_t (*rdc_elt)(const void *),
	       void (*free_elt)(void *)){
  size_t elt_rem, pty_rem;
  h->pty_size = pty_size;
  h->elt_size = elt_size;
  /* align priority relative to a malloc's pointer and compute pair_size */
  if (h->pty_size <= h->elt_size){
    h->elt_offset = h->elt_size;
  }else{
    elt_rem = h->pty_size % h->elt_size;
    h->elt_offset = add_sz_perror(h->pty_size,
				  (elt_rem > 0) * (h->elt_size - elt_rem));
  }
  pty_rem = add_sz_perror(h->elt_offset, h->elt_size) % h->pty_size;
  h->pair_size = add_sz_perror(h->elt_offset + h->elt_size,
			       (pty_rem > 0) * (h->pty_size - pty_rem));
  h->count = min_num;
  h->num_elts = 0;
  h->buf = malloc_perror(1, h->pair_size); /* heapify */
  h->pty_elts = malloc_perror(h->count, h->pair_size);
  h->hht = hht;
  h->cmp_pty = cmp_pty;
  h->cmp_elt = cmp_elt;
  h->rdc_elt = rdc_elt;
  h->free_elt = free_elt;
  /* hash table maps an element to a size_t index  */
  if (h->hht->init != NULL && h->hht->align != NULL){
    h->hht->init(hht->ht,
		 h->elt_size,
		 sizeof(size_t),
		 h->count,
		 hht->alpha_n,
		 hht->log_alpha_d,
		 h->cmp_elt,
		 h->rdc_elt,
		 NULL, /* only elt_size block is deleted in hash table */
		 NULL);
    /* elements dereferenced with size_t *; may be slighly overaligned */
    h->hht->align(h->hht->ht, sizeof(size_t));
  }
}

/**
   Aligns the in-heap pty_size blocks, elt_size block, and size_t indices,
   according to the provided alignment requirement values. If any of the
   provided values are lower than the corresponding pty_size, elt_size,
   and sizeof(size_t) values then the space requirements of the heap may
   be lowered as compared to a call to heap_init alone. If only some
   alignment requirement values are known, then the type size values can
   be used for the unknown alignment requirement values. The heap keeps the
   effective types of the copied pty_size and elt_size blocks, if they had
   effective types at the time of insertion (push). The operation is
   optionally called after heap_init is completed and before any other
   operation is called.
   ht            : pointer to an initialized heap struct
   pty_alignment : alignment requirement or size of the priority type copied
                   as pty_size block; if size, must account for internal and
                   trailing padding according to sizeof
   elt_alignment : alignment requirement or size of the type of the elt_size
                   block of an element; if size, must account for internal
                   and trailing padding according to sizeof
   sz_alignment  : - zero if a hash table was initialized prior to calling
                   heap_init
                   - otherwise, non-zero alignment requirement or size of
                   size_t
*/
void heap_align(struct heap *h,
		size_t pty_alignment,
		size_t elt_alignment,
		size_t sz_alignment){
  size_t elt_rem, pty_rem;
  if (h->pty_size <= elt_alignment){
    h->elt_offset = elt_alignment;
  }else{
    elt_rem = h->pty_size % elt_alignment;
    h->elt_offset = add_sz_perror(h->pty_size,
				  (elt_rem > 0) * (elt_alignment - elt_rem));
  }
  pty_rem = add_sz_perror(h->elt_offset, h->elt_size) % pty_alignment;
  h->pair_size = add_sz_perror(h->elt_offset + h->elt_size,
			       (pty_rem > 0) * (pty_alignment - pty_rem));
  h->buf = realloc_perror(h->buf, 2, h->pair_size);
  memset(h->buf, 0, 2 * h->pair_size);
  h->pty_elts = realloc_perror(h->pty_elts, h->count, h->pair_size);
  if (sz_alignment > 0) h->hht->align(h->hht->ht, sz_alignment);
}

/**
   Inserts (pushes) a priority value and an associated element not yet in a
   heap according to cmp_elt, by copying the corresponding pty_size and
   elt_size blocks into the heap. Prior to insertion, the membership of an
   element can be tested, if necessary, with heap_search in O(1) time in
   expectation under the uniformity assumptions suitable for the used hash
   table.
   h           : pointer to an initialized heap
   pty         : non-NULL pointer to the pty_size block containing a
                 priority value
   elt         : non-NULL pointer to the elt_size block of an element
*/
void heap_push(struct heap *h, const void *pty, const void *elt){
  size_t ix = h->num_elts;
  if (h->count == ix){
    /* grow heap; amortized constant overhead per push, 
       without considering realloc's search */
    h->count = mul_sz_perror(2, h->count);
    h->pty_elts = realloc_perror(h->pty_elts, h->count, h->pair_size);
  }
  memcpy(pty_ptr(h, ix), pty, h->pty_size);
  memcpy(elt_ptr(h, ix), elt, h->elt_size);
  h->hht->insert(h->hht->ht, elt, &ix);
  h->num_elts++;
  heapify_up(h, ix);
}

/** 
   Returns a pointer to the priority value associated with an element in a
   heap according to cmp_elt. Returns NULL if the element is not in the heap
   according to cmp_elt. Runs in O(1) time in expectation under the uniformity
   assumptions suitable for the used hash table. The returned pointer can be
   dereferenced with a pointer compatible with the priority type in order to
   comply with the strict aliasing rules. It can be the same or a cvr-
   qualified/signed/unsigned version of the type. The returned pointer is
   guaranteed to point to the current priority value until another heap
   operation is performed.
   h           : pointer to an initialized heap
   elt         : non-NULL pointer to the elt_size block of an element
*/
void *heap_search(const struct heap *h, const void *elt){
  const size_t *ix_ptr = h->hht->search(h->hht->ht, elt);
  if (ix_ptr != NULL){
    return pty_ptr(h, *ix_ptr);
  }else{
    return NULL;
  }
}

/**
   Updates the priority value of an element that is in a heap. Prior
   to updating, the membership of an element can be tested, if necessary, 
   with heap_search in O(1) time in expectation under the uniformity
   assumptions suitable for the used hash table.
   h           : pointer to an initialized heap
   pty         : non-NULL pointer to the pty_size block containing a
                 priority value
   elt         : non-NULL pointer to the elt_size block of an element
*/
void heap_update(struct heap *h, const void *pty, const void *elt){
  size_t ix = *(const size_t *)h->hht->search(h->hht->ht, elt);
  memcpy(pty_ptr(h, ix), pty, h->pty_size);
  heapify_up(h, ix);
  heapify_down(h, ix);
}

/**
   Pops an element associated with a minimal priority value in a heap 
   according to cmp_pty. If the heap is empty, the memory blocks pointed 
   to by elt and pty parameters remain unchanged.
   h           : pointer to an initialized heap
   pty         : non-NULL pointer to a block of size pty_size 
   elt         : non-NULL pointer to a block of size elt_size
*/
void heap_pop(struct heap *h, void *pty, void *elt){
  size_t ix_buf, ix = 0;
  if (h->num_elts == 0) return;
  memcpy(pty, pty_ptr(h, ix), h->pty_size);
  memcpy(elt, elt_ptr(h, ix), h->elt_size);
  half_swap(h, ix, h->num_elts - 1);
  h->hht->remove(h->hht->ht, elt, &ix_buf);
  h->num_elts--;
  if (h->num_elts > 0) heapify_down(h, ix);
}

/**
   Frees the memory of all priorities and elements that are in a heap. The
   elements are freed according to free_elt. Frees the memory of the heap
   and hash table leaving only the blocks that were preallocated before
   the call to heap_init, including the block of size sizeof(struct heap)
   pointed to by the h parameter.
*/
void heap_free(struct heap *h){
  size_t i;
  if (h->free_elt != NULL){
    for (i = 0; i < h->num_elts; i++){
      h->free_elt(elt_ptr(h, i));
    } 
  }
  free(h->buf);
  free(h->pty_elts);
  h->hht->free(h->hht->ht); /* leaves a block of size of ht struct */
  h->buf = NULL;
  h->pty_elts = NULL;
}

/** Helper functions */

/**
   Copies the priority and element at index s to index t, and maps the
   copied element at index t to t in the hash table.
*/
static void half_swap(struct heap *h, size_t t, size_t s){
  if (s == t) return;
  memcpy(pty_ptr(h, t), pty_ptr(h, s), h->pair_size);
  h->hht->insert(h->hht->ht, elt_ptr(h, t), &t); /* update */
}

/**
   Heapifies the heap structure from the ith element upwards.
*/
static void heapify_up(struct heap *h, size_t i){
  size_t ju;
  size_t ix = i;
  memcpy(h->buf, pty_ptr(h, ix), h->pair_size);
  while (ix > 0){
    ju = (ix - 1) >> 1; /* divide by 2 */;
    if (h->cmp_pty(pty_ptr(h, ju), h->buf) > 0){
      half_swap(h, ix, ju);
      ix = ju;
    }else{
      break;
    }
  }
  memcpy(pty_ptr(h, ix), h->buf, h->pair_size);
  h->hht->insert(h->hht->ht, elt_ptr(h, ix), &ix);
}

/**
   Heapifies the heap structure with at least one element from the ith
   element downwards.
*/
static void heapify_down(struct heap *h, size_t i){
  size_t jl, jr;
  size_t ix = i;
  memcpy(h->buf, pty_ptr(h, ix), h->pair_size);
  /* 0 <= ix <= num_elts - 1 <= SIZE_MAX - 2 */
  while (ix + 2 <= h->num_elts - 1 - ix){
    /* both next left and next right indices have elements */
    jl = 2 * ix + 1;
    jr = 2 * ix + 2;
    if (h->cmp_pty(h->buf, pty_ptr(h, jl)) > 0 &&
	h->cmp_pty(pty_ptr(h, jl), pty_ptr(h, jr)) <= 0){
      half_swap(h, ix, jl);
      ix = jl;
    }else if (h->cmp_pty(h->buf, pty_ptr(h, jr)) > 0){
      /* jr has min pty relative to jl and the ith pty is greater */
      half_swap(h, ix, jr);
      ix = jr;
    }else{
      break;
    }
  }
  if (ix + 1 == h->num_elts - 1 - ix){
    jl = 2 * ix + 1;
    if (h->cmp_pty(h->buf, pty_ptr(h, jl)) > 0){
      half_swap(h, ix, jl);
      ix = jl;
    }
  }
  memcpy(pty_ptr(h, ix), h->buf, h->pair_size);
  h->hht->insert(h->hht->ht, elt_ptr(h, ix), &ix);
}

/**
   Computes a pointer to an element in the pty_elts array of a heap.
*/
static void *pty_ptr(const struct heap *h, size_t i){
  return (void *)((char *)h->pty_elts + i * h->pair_size);
}

/**
   Computes a pointer to a priority in the pty_elts array of a heap.
*/
static void *elt_ptr(const struct heap *h, size_t i){
  return (void *)((char *)h->pty_elts + i * h->pair_size + h->elt_offset);
}
