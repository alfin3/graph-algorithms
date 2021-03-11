/**
   heap.c

   Implementation of a generic dynamicaly allocated (min) heap with a hash
   table parameter. 

   The implementation provides a dynamic set in the min heap form for any
   element objects in memory associated with priority values of basic type
   (e.g. char, int, long, double).

   The hash table parameter specifies a hash table used for in-heap
   search and modifications, and enables the optimization of space and
   time resources associated with heap operations by choice of a hash
   table and its load factor upper bound.

   The implementation assumes that for every element in a heap, the 
   block of size elt_size pointed to by an argument passed as the elt
   parameter in heap_push is unique. Because an element object can be
   represented by its unique pointer, this invariant only prevents
   associating a given element object in memory with more than one priority
   value in a heap.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heap.h"
#include "utilities-mem.h"

static void swap(heap_t *h, size_t i, size_t j);
static void half_swap(heap_t *h, size_t t, size_t s);
static void heap_grow(heap_t *h);
static void heapify_up(heap_t *h, size_t i);
static void heapify_down(heap_t *h, size_t i);
static void *pty_ptr(const heap_t *h, size_t i);
static void *elt_ptr(const heap_t *h, size_t i);
static void fprintf_stderr_exit(const char *s, int line);

/**
   Initializes a heap.
   h           : pointer to a preallocated block of size sizeof(heap_t)
   init_count  : > 0
   pty_size    : size of a contiguous priority object
   elt_size    : - size of an element object, if the element object is
                 within a contiguous memory block and element copies
                 are not pushed as distinct elements
                 - size of a pointer to an element object, if the element
                 object is within a contiguous memory block and element
                 copies are pushed as distinct elements
                 - size of a pointer to an element object, if the element
                 object is within a noncontiguous memory block
   ht          : a non-NULL pointer to a set of parameters specifying a
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
   free_elt    : - if an element is within a contiguous memory block,
                 as reflected by elt_size, and a pointer to the element is 
                 passed as elt in ht_push, then the element is fully copied
                 into a heap, and NULL as free_elt is sufficient to delete
                 the element,
                 - if an element is an object within a contiguous or
                 noncontiguous memory block, and a pointer to a pointer to
                 the element is passed as elt in heap_push, then the pointer
                 to the element is copied into the heap, and an element-
                 specific free_elt, taking a pointer to a pointer to an
                 element as its parameter, is necessary to delete the element
*/
void heap_init(heap_t *h,
	       size_t init_count,
	       size_t pty_size,
	       size_t elt_size,
	       const heap_ht_t *ht,
	       int (*cmp_pty)(const void *, const void *),
	       void (*free_elt)(void *)){
  h->count = init_count;
  h->count_max = HEAP_COUNT_MAX;
  if (h->count > h->count_max){
    fprintf_stderr_exit("init_count > count maximum", __LINE__);
  }
  h->num_elts = 0;
  h->pty_size = pty_size;
  h->elt_size = elt_size;
  h->pty_elts = malloc_perror(init_count * (pty_size + elt_size));
  h->ht = ht;
  h->cmp_pty = cmp_pty;
  h->free_elt = free_elt;
  h->ht->init(ht->ht, elt_size, sizeof(size_t), NULL, ht->context);
}

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
                 non-contiguous element object; the block must have a unique
                 bit pattern for each pushed element
*/
void heap_push(heap_t *h, const void *pty, const void *elt){
  size_t ix = h->num_elts;
  if (h->count == ix) heap_grow(h);
  memcpy(pty_ptr(h, ix), pty, h->pty_size);
  memcpy(elt_ptr(h, ix), elt, h->elt_size);
  h->ht->insert(h->ht->ht, elt, &ix);
  h->num_elts++;
  heapify_up(h, ix);
}

/** 
   Returns a pointer to the priority of an element in a heap or NULL if the
   element is not in the heap in O(1) time in expectation under the
   uniformity assumptions suitable for the used hash table. The returned
   pointer is guaranteed to point to the current priority value until another
   heap operation is performed. Please see the parameter specification in
   heap_push.
*/
void *heap_search(const heap_t *h, const void *elt){
  size_t *ix_ptr = h->ht->search(h->ht->ht, elt);
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
   assumptions suitable for the used hash table. Please see the parameter
   specification in heap_push.
*/
void heap_update(heap_t *h, const void *pty, const void *elt){
  size_t ix = *(size_t *)h->ht->search(h->ht->ht, elt);
  memcpy(pty_ptr(h, ix), pty, h->pty_size);
  heapify_up(h, ix);
  heapify_down(h, ix);
}

/**
   Pops an element associated with a minimal priority value in a heap 
   according to cmp_pty. If the heap is empty, the memory blocks pointed 
   to by elt and pty remain unchanged. Please see the parameter
   specification in heap_push.
*/
void heap_pop(heap_t *h, void *pty, void *elt){
  size_t ix_buf, ix = 0;
  if (h->num_elts == 0) return;
  memcpy(pty, pty_ptr(h, ix), h->pty_size);
  memcpy(elt, elt_ptr(h, ix), h->elt_size);
  swap(h, ix, h->num_elts - 1);
  h->ht->remove(h->ht->ht, elt, &ix_buf);
  h->num_elts--;
  if (h->num_elts > 0) heapify_down(h, ix);
}

/**
   Frees a heap and leaves a block of size sizeof(heap_t) pointed to by
   an argument passed as the h parameter.
*/
void heap_free(heap_t *h){
  size_t i;
  if (h->free_elt != NULL){
    for (i = 0; i < h->num_elts; i++){
      h->free_elt(elt_ptr(h, i));
    } 
  }
  free(h->pty_elts);
  h->ht->free(h->ht->ht);
  h->pty_elts = NULL;
}

/** Helper functions */

/**
   Swaps priorities and elements at indices i and j and maps the elements
   to their new indices in the hash table.
*/
static void swap(heap_t *h, size_t i, size_t j){
  void *buf = NULL;
  if (i == j) return;
  buf = malloc_perror(h->pty_size + h->elt_size);
  memcpy(buf, pty_ptr(h, i), h->pty_size + h->elt_size);
  memcpy(pty_ptr(h, i), pty_ptr(h, j), h->pty_size + h->elt_size);
  memcpy(pty_ptr(h, j), buf, h->pty_size + h->elt_size);
  h->ht->insert(h->ht->ht, elt_ptr(h, i), &i);
  h->ht->insert(h->ht->ht, elt_ptr(h, j), &j);
  free(buf);
  buf = NULL;
}

/**
   Copies the priority and element at index s to index t, and maps the
   copied element at index t to t in the hash table.
*/
static void half_swap(heap_t *h, size_t t, size_t s){
  if (s == t) return;
  memcpy(pty_ptr(h, t), pty_ptr(h, s), h->pty_size + h->elt_size);
  h->ht->insert(h->ht->ht, elt_ptr(h, t), &t);
}

/**
   Doubles the size of a heap upto the maximal heap count. Amortized
   constant overhead per push operation, without considering realloc's
   search of the memory heap.
*/
static void heap_grow(heap_t *h){
  if (h->count == h->count_max){
    fprintf_stderr_exit("tried to exceed the count maximum", __LINE__);
  }
  if (h->count_max - h->count < h->count){
    h->count = h->count_max;
  }else{
    h->count *= 2;
  }
  h->pty_elts = realloc_perror(h->pty_elts,
			       h->count * (h->pty_size + h->elt_size));
}

/**
   Heapifies the heap structure from the ith element upwards.
*/
static void heapify_up(heap_t *h, size_t i){
  size_t ju;
  void *buf = malloc_perror(h->pty_size + h->elt_size);
  memcpy(buf, pty_ptr(h, i), h->pty_size + h->elt_size);
  while(i > 0){
    ju = (i - 1) / 2;
    if (h->cmp_pty(pty_ptr(h, ju), buf) > 0){
      half_swap(h, i, ju);
      i = ju;
    }else{
      break;
    }
  }
  memcpy(pty_ptr(h, i), buf, h->pty_size + h->elt_size);
  h->ht->insert(h->ht->ht, elt_ptr(h, i), &i);
  free(buf);
  buf = NULL;
}

/**
   Heapifies the heap structure with at least one element from the ith
   element downwards.
*/
static void heapify_down(heap_t *h, size_t i){
  size_t jl, jr;
  void *buf = malloc_perror(h->pty_size + h->elt_size);
  memcpy(buf, pty_ptr(h, i), h->pty_size + h->elt_size);
  /* 0 <= i <= num_elts - 1 <= SIZE_MAX - 2 */
  while (i + 2 <= h->num_elts - 1 - i){
    /* both next left and next right indices have elements */
    jl = 2 * i + 1;
    jr = 2 * i + 2;
    if (h->cmp_pty(buf, pty_ptr(h, jl)) > 0 &&
	h->cmp_pty(pty_ptr(h, jl), pty_ptr(h, jr)) <= 0){
      half_swap(h, i, jl);
      i = jl;
    }else if (h->cmp_pty(buf, pty_ptr(h, jr)) > 0){
      /* jr has min pty relative to jl and the ith pty is greater */
      half_swap(h, i, jr);
      i = jr;
    }else{
      break;
    }
  }
  if (i + 1 == h->num_elts - 1 - i){
    jl = 2 * i + 1;
    if (h->cmp_pty(buf, pty_ptr(h, jl)) > 0){
      half_swap(h, i, jl);
      i = jl;
    }
  }
  memcpy(pty_ptr(h, i), buf, h->pty_size + h->elt_size);
  h->ht->insert(h->ht->ht, elt_ptr(h, i), &i);
  free(buf);
  buf = NULL;
}

/**
   Computes a pointer to an element in the element-priority array of a heap.
*/
static void *pty_ptr(const heap_t *h, size_t i){
  return (void *)((char *)h->pty_elts + i * (h->pty_size + h->elt_size));
}

/**
   Computes a pointer to a priority in the element-priority array of a heap.
*/
static void *elt_ptr(const heap_t *h, size_t i){
  return (void *)((char *)h->pty_elts +
		  i * (h->pty_size + h->elt_size) + h->pty_size);
}

/**
   Prints an error message and exits.
*/
static void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
}
