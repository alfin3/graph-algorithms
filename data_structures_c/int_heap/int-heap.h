/**
   int-heap.h

   Struct declarations and declarations of accessible functions of a 
   non-generic dynamicaly allocated min heap of integer elements and 
   integer priority values.
*/

#ifndef INT_HEAP_H  
#define INT_HEAP_H

typedef struct{
  int heap_size;
  int num_elts;
  int *elts;
  int *ptys;
} int_heap_t;

/**
   Initializes a min heap.
*/
void int_heap_init(int_heap_t *h, int heap_size);

/**
   Pushes integer element associated with integer priority onto a min heap.
*/
void int_heap_push(int_heap_t *h, int elt, int pty);

/**
   Pops an element associated with the minimal priority.
*/
void int_heap_pop(int_heap_t *h, int *elt, int *pty);

/**
   If element is present on the heap, updates its priority and returns 1.
   Returns 0 otherwise.
*/
int int_heap_update(int_heap_t *h, int elt, int pty);

/**
   Frees dynamically allocated arrays in a struct of heap_t type.
*/
void int_heap_free(int_heap_t *h);

#endif
