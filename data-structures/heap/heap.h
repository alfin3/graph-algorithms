/**
   heap.h

   Struct declarations and declarations of accessible functions of a generic 
   dynamicaly allocated (min) heap with a hash table parameter. 

   The implementation provides a dynamic set in the min heap form for any
   elements in memory associated with priority values of basic type
   (e.g. char, int, long, double).

   The hash table parameter specifies a hash table used for in-heap
   search and modifications, and enables the optimization of space and
   time resources associated with heap operations by choice of a hash
   table, its load factor upper bound, and known or expected minimum
   number of simultaneously present elements.

   The implementation assumes that for every element in a heap, the 
   block of size elt_size pointed to by an argument passed as the elt
   parameter in heap_push is unique. Because an element can be
   represented by its unique pointer, this invariant only prevents
   associating a given element in memory with more than one priority
   value in a heap.
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
	       size_t (*)(const void *, size_t),
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
  size_t (*rdc_elt)(const void *, size_t);
  void (*free_elt)(void *);
} heap_t;

/**
   Initializes a heap.
   h           : pointer to a preallocated block of size sizeof(heap_t)
   init_count  : > 0
   pty_size    : size of a contiguous priority object
   elt_size    : - size of an element, if the element is within a contiguous
                 memory block and a copy of the element is inserted,
                 - size of a pointer to an element, if the element is within
                 a noncontiguous memory block or a pointer to a contiguous
                 element is inserted
   hht         : a non-NULL pointer to a set of parameters specifying a
                 hash table for in-heap search and modifications; a hash
                 key has the size and bit pattern of the block of size
                 elt_size pointed to by elt in heap_push
   cmp_pty     : comparison function which returns a negative integer value
                 if the priority value pointed to by the first argument is
                 less than the priority value pointed to by the second, a
                 positive integer value if the priority value pointed to by
                 the first argument is greater than the priority value 
                 pointed to by the second, and zero integer value if the two
                 priority values are equal
   free_elt    : - if an element is within a contiguous memory block and
                 a copy of the element was inserted, then NULL as free_elt
                 is sufficient to delete the element,
                 - if an element is within a noncontiguous memory block or
                 a pointer to a contiguous element was inserted, then an
                 element-specific free_elt, taking a pointer to a pointer to an
                 element as its argument and leaving a block of size elt_size
                 pointed to by the argument, is necessary to delete the element
*/
void heap_init(heap_t *h,
	       size_t pty_size,
	       size_t elt_size,
	       size_t min_num,
	       size_t alpha_n;
	       size_t log_alpha_d;
	       const heap_ht_t *hht,
	       int (*cmp_pty)(const void *, const void *),
	       int (*cmp_elt)(const void *, const void *),
	       size_t (*rdc_elt)(const void *, size_t),
	       void (*free_elt)(void *));

/**
*/
void heap_align(heap_t *h,
		size_t pty_alignment,
		size_t elt_alignment,
		size_t sz_alignment);

/**
   Pushes an element not in a heap and an associated priority value. 
   Prior to pushing, the membership of an element can be tested, if 
   necessary, with heap_search in O(1) time in expectation under the
   uniformity assumptions suitable for the used hash table.
   h           : pointer to an initialized heap
   pty         : pointer to a block of size pty_size that is an object of
                 basic type (e.g. char, int, long, double)
   elt         : pointer to a block of size elt_size that is either a
                 contiguous element object or a pointer to a contiguous or
                 non-contiguous element; the block must have a unique
                 bit pattern for each pushed element
*/
void heap_push(heap_t *h, const void *pty, const void *elt);

/** 
   Returns a pointer to the priority of an element in a heap or NULL if the
   element is not in the heap in O(1) time in expectation under the
   uniformity assumptions suitable for the used hash table. The returned
   pointer is guaranteed to point to the current priority value until another
   heap operation is performed. Please see the parameter specification in
   heap_push.
*/
void *heap_search(const heap_t *h, const void *elt);

/**
   Updates the priority value of an element that is in a heap. Prior
   to updating, the membership of an element can be tested, if necessary, 
   with heap_search in O(1) time in expectation under the uniformity
   assumptions suitable for the used hash table. Please see the parameter
   specification in heap_push.
*/
void heap_update(heap_t *h, const void *pty, const void *elt);

/**
   Pops an element associated with a minimal priority value in a heap 
   according to cmp_pty. If the heap is empty, the memory blocks pointed 
   to by elt and pty remain unchanged. Please see the parameter
   specification in heap_push.
*/
void heap_pop(heap_t *h, void *pty, void *elt);

/**
   Frees a heap and leaves a block of size sizeof(heap_t) pointed to by
   an argument passed as the h parameter.
*/
void heap_free(heap_t *h);

#endif
