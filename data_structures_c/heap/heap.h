/**
   heap.h

   Struct declarations and declarations of accessible functions of generic 
   dynamicaly allocated (min) heap. 

   Through user-defined comparison and deallocation functions, the 
   implementation enables a dynamic set in heap form of any objects 
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
  void **elts;
  void *ptys;
  int (*cmp_elt_fn)(void *, void *);
  int (*cmp_pty_fn)(void *, void *);
  void (*free_elt_fn)(void *);
} heap_t;

/**
   Initializes a heap. 
   cmp_elt_fn: returns 0 if both pointers point to identical elements,
                       non-zero otherwise.
   cmp_pty_fn returns: 0 if both pointers point to the same priority value,
                       >0 if the first pointer points to greater priority value
                       <0 if the first pointer points to lower priority value.
   free_elt_fn: can be NULL if each element does not point to additional 
                memory blocks (e.g. basic type).                    
*/
void heap_init(heap_t *h,
	       int init_heap_size,
	       int elt_size,
	       int pty_size,
	       int (*cmp_elt_fn)(void *, void *),
	       int (*cmp_pty_fn)(void *, void *),
	       void (*free_elt_fn)(void *));

/**
   Pushes an element onto a heap.
*/
void heap_push(heap_t *h, void *elt, void *pty);

/**
    Pops an element and the minimal priority value according to cmp_elts_fn.
*/
void heap_pop(heap_t *h, void *elt, void *pty);

/**
   If element is present on a heap, updates its priority and returns 1,
   otherwise returns 0.
*/
int heap_update(heap_t *h, void *elt, void *pty);

/**
   Frees a dynamically allocated heap. Memory dynamically allocated to 
   each element is freed according to free_elt_fn.
*/
void heap_free(heap_t *h);

#endif
