/**
   heap.h

   Struct declarations and declarations of accessible functions of generic 
   dynamicaly allocated (min) heap. 

   Through user-defined comparison and deallocation functions, the 
   implementation provides a dynamic set in heap form of any objects 
   associated with priority values of basic type of choice (e.g. char, int, 
   long, double).
*/

#ifndef HEAP_H  
#define HEAP_H

typedef struct{
  int heap_size;
  int num_elts;
  int elt_size;
  int pty_size;
  void *elts;
  void *ptys;
  int (*cmp_elt_fn)(void *, void *);
  int (*cmp_pty_fn)(void *, void *);
  void (*free_elt_fn)(void *);
} heap_t;

/**
   Initializes a heap. 
   init_heap_size: integer > 0.
   cmp_elt_fn: returns 0 if elements match, non-zero otherwise.
   cmp_pty_fn: returns  0 if priorities are equal,
                       > 0 if first priority is greater,
                       < 0 if first priority is lower.
   free_elt_fn: non-NULL.
*/
void heap_init(heap_t *h,
	       int init_heap_size,
	       int elt_size,
	       int pty_size,
	       int (*cmp_elt_fn)(void *, void *),
	       int (*cmp_pty_fn)(void *, void *),
	       void (*free_elt_fn)(void *));

/**
   Pushes an element and priority value onto a heap.
   elt: pointer to element pointer, if element is a multilayer object,
        pointer to element, if element is fully stored in elts array,
        elt_size reflects these cases.
   pty: pointer to object of basic type (e.g. char, int, long, double),
        pty_size reflects a type.
*/
void heap_push(heap_t *h, void *elt, void *pty);

/**
   Pops an element and a minimal priority according to cmp_pty_fn.
*/
void heap_pop(heap_t *h, void *elt, void *pty);

/**
   If element is present on heap according to cmp_elt_fn, updates its 
   priority and returns 1, otherwise returns 0.
*/
int heap_update(heap_t *h, void *elt, void *pty);

/**
   Frees elements, according to free_elt_fn, and element and priority arrays.
*/
void heap_free(heap_t *h);

#endif
