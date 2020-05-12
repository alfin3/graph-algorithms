/**
   heap.h

   Struct declarations and declarations of accessible functions of generic 
   dynamicaly allocated (min) heap. 

   Through user-defined comparison and deallocation functions, the 
   implementation provides a dynamic set in heap form of any objects 
   associated with priority values of basic type of choice (e.g. char, int, 
   long, double). 

   Push and pop operations do not change the location of elements in memory,
   whereas priority values are copied.
*/

#ifndef HEAP_H  
#define HEAP_H

typedef struct{
  int heap_size;
  int num_elts;
  int elt_size;
  int pty_size;
  void **elts;
  void *ptys;
  int (*cmp_elt_fn)(void *, void *);
  int (*cmp_pty_fn)(void *, void *);
  void (*free_elt_fn)(void *);
} heap_t;

/**
   Initializes a heap. 
   init_heap_size: integer > 0.
   cmp_elt_fn: returns 0 if both pointers point to elements that match,
                       non-zero otherwise.
   cmp_pty_fn: returns  0 if both pointers point to equal priority values,
                       > 0 if first pointer points to greater priority value,
                       < 0 if first pointer points to lower priority value.
   free_elt_fn: is non-NULL regardless if elements are on memory heap 
                or memory stack.
*/
void heap_init(heap_t *h,
	       int init_heap_size,
	       int elt_size,
	       int pty_size,
	       int (*cmp_elt_fn)(void *, void *),
	       int (*cmp_pty_fn)(void *, void *),
	       void (*free_elt_fn)(void *));

/**
   Pushes an element onto a heap. Copies a priority value and pointer to 
   element, preserving element's location in memory. 
   elt: pointer to element pointer
   pty: pointer to object of basic type
*/
void heap_push(heap_t *h, void *elt, void *pty);

/**
    Pops an element and the minimal priority value according to cmp_pty_fn.
*/
void heap_pop(heap_t *h, void *elt, void *pty);

/**
   If element is present on heap according to cmp_elt_fn, updates its 
   priority and returns 1, otherwise returns 0.
*/
int heap_update(heap_t *h, void *elt, void *pty);

/**
   Frees element and priority arrays. Memory allocated to each element is 
   freed according to free_elt_fn.
*/
void heap_free(heap_t *h);

#endif
