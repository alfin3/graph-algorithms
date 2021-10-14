/**
   heap.h

   Struct declarations and declarations of accessible functions of a (min)
   heap with generic contiguous priorities, generic contiguous or
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

#ifndef HEAP_H  
#define HEAP_H

#include <stddef.h>

typedef struct{
  void *ht; /* points to a preallocated hash table struct */
  
  /* pointers to hash table op helpers, pre-defined in each hash table */
  void (*init)(void *,
	       size_t,
	       size_t,
	       size_t,
	       size_t,
	       size_t,
	       int (*)(const void *, const void *),
	       size_t (*)(const void *),
	       void (*)(void *),
	       void (*)(void *));
  void (*align)(void *, size_t);
  void (*insert)(void *, const void *, const void *);
  void *(*search)(const void *, const void *);
  void (*remove)(void *, const void *, void *);
  void (*free)(void *);
} heap_ht_t;

typedef struct{
  size_t pty_size;
  size_t elt_size;
  size_t pair_size; /* size of a pty elt pair aligned in memory */
  size_t elt_offset; /* number of bytes from beginning of pair to elt */
  size_t count;
  size_t num_elts;
  size_t alpha_n;
  size_t log_alpha_d;
  void *buf; /* only used by heap operations internally */
  void *pty_elts;
  const heap_ht_t *hht;
  int (*cmp_pty)(const void *, const void *);
  int (*cmp_elt)(const void *, const void *);
  size_t (*rdc_elt)(const void *);
  void (*free_elt)(void *);
} heap_t;

/**
   Initializes a heap. The in-heap pty_size and elt_size blocks, as well as
   the size_t indices in a hash table are aligned by default according to their
   sizes because size of a type T >= alignment requirement of T (due to the
   structure of arrays).
   h           : pointer to a preallocated block of size sizeof(heap_t)
   pty_size    : non-zero size of a pty_size block
   elt_size    : non-zero size of an elt_size block
   min_num     : minimum number of elements that are known to be or expected to
                 be present simultaneously in a heap; may result in a
                 speedup by avoiding the unnecessary growth steps of the
                 hash table; 0 if a positive value is not specified and all
                 growth steps are to be completed
   alpha_n     : > 0 numerator of a load factor upper bound
   log_alpha_d : < size_t width; log base 2 of the denominator of the load
                 factor upper bound; the denominator is a power of two;
                 additional requirements may apply due to the type of a hash
                 table
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
void heap_init(heap_t *h,
	       size_t pty_size,
	       size_t elt_size,
	       size_t min_num,
	       size_t alpha_n,
	       size_t log_alpha_d,
	       const heap_ht_t *hht,
	       int (*cmp_pty)(const void *, const void *),
	       int (*cmp_elt)(const void *, const void *),
	       size_t (*rdc_elt)(const void *),
	       void (*free_elt)(void *));

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
   ht            : pointer to an initialized heap_t struct
   pty_alignment : alignment requirement or size of the priority type copied
                   as pty_size block
   elt_alignment : alignment requirement or size of the type of the elt_size
                   block of an element
   sz_alignment  : alignment requirement or size of size_t
*/
void heap_align(heap_t *h,
		size_t pty_alignment,
		size_t elt_alignment,
		size_t sz_alignment);

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
void heap_push(heap_t *h, const void *pty, const void *elt);

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
void *heap_search(const heap_t *h, const void *elt);

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
void heap_update(heap_t *h, const void *pty, const void *elt);

/**
   Pops an element associated with a minimal priority value in a heap 
   according to cmp_pty. If the heap is empty, the memory blocks pointed 
   to by elt and pty parameters remain unchanged.
   h           : pointer to an initialized heap
   pty         : non-NULL pointer to a block of size pty_size 
   elt         : non-NULL pointer to a block of size elt_size
*/
void heap_pop(heap_t *h, void *pty, void *elt);

/**
   Frees the memory of all priorities and elements that are in a heap. The
   elements are freed according to free_elt. Frees the memory of the heap
   and hash table leaving only the blocks that were preallocated before
   the call to heap_init, including the block of size sizeof(heap_t)
   pointed to by the h parameter.
*/
void heap_free(heap_t *h);

#endif
