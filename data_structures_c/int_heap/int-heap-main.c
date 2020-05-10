/**
   int-heap-main.c

   Examples of a non-generic dynamicaly allocated min heap of integer elements 
   and integer priority values.

*/

#include <stdio.h>
#include <stdlib.h>
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

int main(){
  int_heap_t h;
  int min_elt;
  int min_pty;
  int pty_start = 10;
  int updated_p;
  int new_pty[3] = {10, 0, 10};
  int elts[3] = {5, 5, 11};
  int_heap_init(1, &h);
  for (int i = 0; i < pty_start; i++){
    int_heap_push(i, pty_start - i, &h);
    printf("Element array: ");
    print_arr(h.elt_arr, h.num_elts);
    printf("Priority array: ");
    print_arr(h.pty_arr, h.num_elts);
  }
  for (int i = 0; i < 2; i++){
    int_heap_pop(&min_elt, &min_pty, &h);
    printf("min element: %d, min priority: %d\n", min_elt, min_pty);
    printf("Element array: ");
    print_arr(h.elt_arr, h.num_elts);
    printf("Priority array: ");
    print_arr(h.pty_arr, h.num_elts);
  }
  for (int i = 0; i < 2; i++){
    updated_p = int_heap_update(elts[i], new_pty[i], &h);
    printf("updated? %d\n", updated_p);
    printf("Element array: ");
    print_arr(h.elt_arr, h.num_elts);
    printf("Priority array: ");
    print_arr(h.pty_arr, h.num_elts);
  }
  int_heap_free(&h);
  return 0;
}
