/**
   int-min-heap.c

   Implementation of a dynamicaly allocated non-generic version of min heap.
   Each entry consists of an integer element and integer priority value.

*/

#include <stdio.h>
#include <stdlib.h>

typedef struct{
  int heap_size;
  int num_elts;
  int *elt_arr;
  int *pty_arr;
} heap_t;

/** Main functions */

void heapify_up(heap_t *h, int i);
void heapify_down(heap_t *h, int i);

/**
   Initializes a dynamically allocated min heap.
*/
void heap_init(int heap_size, heap_t *h){
  h->heap_size = heap_size;
  h->num_elts = 0;
  h->elt_arr = (int *)malloc(heap_size * sizeof(int));
  h->pty_arr = (int *)malloc(heap_size * sizeof(int));
}

/**
   Pushes integer element associated with integer priority onto a min heap.
*/
void heap_push(int elt, int pty, heap_t *h){
  //amortized constant overhead in worst case of realloc calls
  if (h->heap_size == h->num_elts){
    h->elt_arr = (int *)realloc(h->elt_arr, 2 * h->heap_size * sizeof(int));
    h->pty_arr = (int *)realloc(h->pty_arr, 2 * h->heap_size * sizeof(int));
    h->heap_size = h->heap_size * 2; 
  }
  h->elt_arr[h->num_elts] = elt;
  h->pty_arr[h->num_elts] = pty;
  h->num_elts++;
  heapify_up(h, h->num_elts - 1);
}

/**
   Pops an element associated with the minimal priority.
*/
void heap_pop(int *elt, int *pty, heap_t *h){
  *elt = h->elt_arr[0];
  *pty = h->pty_arr[0];
  h->elt_arr[0] = h->elt_arr[h->num_elts - 1];
  h->pty_arr[0] = h->pty_arr[h->num_elts - 1];
  h->num_elts--;
  heapify_down(h, 0);
}

/**
   If element is present on the heap, updates its priority and returns 1.
   Returns 0 otherwise.
*/
int heap_update(int elt, int pty, heap_t *h){
  // at this time, implementation w/o hash table => O(n) instead of O(logn)
  for (int i = 0; i < h->num_elts; i++){
    if (h->elt_arr[i] == elt){
      h->elt_arr[i] = elt;
      h->pty_arr[i] = pty;
      int ju = (i - 1) / 2;
      if (ju >= 0 && h->pty_arr[ju] > h->pty_arr[i]){
	heapify_up(h, i);
      } else {heapify_down(h, i);}
      return 1;
    }
  }
  return 0;
}

/**
   Frees dynamically allocated arrays in a struct of heap_t type.
*/
void heap_free(heap_t *h){
  free(h->elt_arr);
  free(h->pty_arr);
}

/** Helper functions */

/**
   Swaps elements at indeces i and j in both element and priority arrays.
*/
void swap(int *elt_arr, int *pty_arr, int i, int j){
  int temp_pty = pty_arr[i];
  int temp_elt = elt_arr[i];
  pty_arr[i] = pty_arr[j];
  elt_arr[i] = elt_arr[j];
  pty_arr[j] = temp_pty;
  elt_arr[j] = temp_elt;
}

/**
   Heapifies heap structure from ith element in array representation upwards.
*/
void heapify_up(heap_t *h, int i){
  int ju = (i - 1) / 2; // if i is even, equivalent to (i - 2) / 2
  while(ju >= 0 && h->pty_arr[ju] > h->pty_arr[i]){
      swap(h->elt_arr, h->pty_arr, i, ju);
      i = ju;
      ju = (i - 1) / 2;
  }
}

/**
   Heapifies heap structure from ith element in array representation downwards.
*/
void heapify_down(heap_t *h, int i){
  int jl = 2 * i + 1;
  int jr = 2 * i + 2;
  while (jr < h->num_elts && (h->pty_arr[i] > h->pty_arr[jl] ||
			      h->pty_arr[i] > h->pty_arr[jr])){
      if (h->pty_arr[jl] < h->pty_arr[jr]){
	swap(h->elt_arr, h->pty_arr, i, jl);
	i = jl;
      } else {
	swap(h->elt_arr, h->pty_arr, i, jr);
	i = jr;
      }
      jl = 2 * i + 1;
      jr = 2 * i + 2;
  }
  if (jl == h->num_elts - 1 && h->pty_arr[i] > h->pty_arr[jl]){
    swap(h->elt_arr, h->pty_arr, i, jl);
  }
}

/**
   Prints a specified number of elements of an integer array.
*/
void print_arr(int *arr, int num_elts){
  for (int i = 0; i < num_elts; i++){
    printf("%d ", arr[i]);
  }
  printf("\n");
}

/** Use examples */

int main(){
  heap_t h;
  int min_elt;
  int min_pty;
  int pty_start = 10;
  int updated_p;
  int new_pty[3] = {10, 0, 10};
  int elts[3] = {5, 5, 11};
  heap_init(1, &h);
  for (int i = 0; i < pty_start; i++){
    heap_push(i, pty_start - i, &h);
    printf("Element array: ");
    print_arr(h.elt_arr, h.num_elts);
    printf("Priority array: ");
    print_arr(h.pty_arr, h.num_elts);
  }
  for (int i = 0; i < 2; i++){
    heap_pop(&min_elt, &min_pty, &h);
    printf("min element: %d, min priority: %d\n", min_elt, min_pty);
    printf("Element array: ");
    print_arr(h.elt_arr, h.num_elts);
    printf("Priority array: ");
    print_arr(h.pty_arr, h.num_elts);
  }
  for (int i = 0; i < 2; i++){
    updated_p = heap_update(elts[i], new_pty[i], &h);
    printf("updated? %d\n", updated_p);
    printf("Element array: ");
    print_arr(h.elt_arr, h.num_elts);
    printf("Priority array: ");
    print_arr(h.pty_arr, h.num_elts);
  }
  heap_free(&h);
  return 0;
}
