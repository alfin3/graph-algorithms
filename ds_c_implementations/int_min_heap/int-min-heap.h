/**
   int-min-heap.h

   Struct declarations and declarations of accessible functions of a dynamicaly
   allocated non-generic min heap of integer elements and integer priority values.
*/

#ifndef INT_MIN_HEAP_H  
#define INT_MIN_HEAP_H

typedef struct{
  int heap_size;
  int num_elts;
  int *elt_arr;
  int *pty_arr;
} heap_t;

/**
   Initializes a dynamically allocated min heap.
*/
void heap_init(int heap_size, heap_t *h);

/**
   Pushes integer element associated with integer priority onto a min heap.
*/
void heap_push(int elt, int pty, heap_t *h);

/**
   Pops an element associated with the minimal priority.
*/
void heap_pop(int *elt, int *pty, heap_t *h);

/**
   If element is present on the heap, updates its priority and returns 1.
   Returns 0 otherwise.
*/
int heap_update(int elt, int pty, heap_t *h);

/**
   Frees dynamically allocated arrays in a struct of heap_t type.
*/
void heap_free(heap_t *h);

#endif
