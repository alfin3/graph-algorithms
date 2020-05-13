/**
   int-heap-main.c

   Examples of a non-generic dynamicaly allocated min heap of integer 
   elements and integer priority values.

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "int-heap.h"

/**
   Prints a specified number of elements of an integer array.
*/
void print_arr(int *arr, int num_elts){
  for (int i = 0; i < num_elts; i++){
    printf("%d ", arr[i]);
  }
  printf("\n");
}

/**
   Prints element and priority arrays in a heap.
*/
void print_els_ptys(int_heap_t *h){
  printf("Element array: ");
  print_arr(h->elts, h->num_elts);
  printf("Priority array: ");
  print_arr(h->ptys, h->num_elts);
}

int main(){
  int_heap_t h;
  int min_elt;
  int min_pty;
  int start_pty = 10;
  int updated_p;
  int new_pty[3] = {10, 0, 10};
  int elts[3] = {5, 5, 11};
  int_heap_init(&h, 1);
  for (int i = 0; i < start_pty; i++){
    int pty = start_pty - i;
    int_heap_push(&h, &i, &pty);
    print_els_ptys(&h);
  }
  printf("\n");
  for (int i = 0; i < 2; i++){
    int_heap_pop(&h, &min_elt, &min_pty);
    printf("min element: %d, min priority: %d\n", min_elt, min_pty);
    print_els_ptys(&h);
  }
  printf("\n");
  for (int i = 0; i < 2; i++){
    updated_p = int_heap_update(&h, &elts[i], &new_pty[i]);
    printf("updated? %d\n", updated_p);
    print_els_ptys(&h);
  }
  printf("\n");
  int_heap_free(&h);
  return 0;
}
