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
void print_elts_ptys(int_heap_t *h){
  printf("Element array: ");
  print_arr(h->elts, h->num_elts);
  printf("Priority array: ");
  print_arr(h->ptys, h->num_elts);
}

/**
   Runs a heap test.
*/
void run_int_heap_test(){
  printf("Running int heap test... \n\n");

  //initialize a heap
  int_heap_t h;
  int heap_init_size = 1;
  int_heap_init(&h, heap_init_size);

  //push elements
  int num_push = 10;
  printf("Pushing %d elements... \n\n", num_push);
  print_elts_ptys(&h);
  for (int i = 0; i < num_push; i++){
    int pty =  num_push - i;
    int_heap_push(&h, &i, &pty);
    print_elts_ptys(&h);
  }
  printf("\n");

  //pop num_pops elements and priority values
  int min_elt;
  int min_pty;
  int num_pops = 2;
  printf("Popping %d elements... \n\n", num_pops);
  for (int i = 0; i < num_pops; i++){
    int_heap_pop(&h, &min_elt, &min_pty);
    printf("E: %d, P: %d\n", min_elt, min_pty);
    print_elts_ptys(&h);
  }
  printf("\n");

  //update heap
  int updated_p;
  int num_new_ptys = 3;
  int elts_upd[] = {5, 5, 11};
  int new_ptys[] = {10, 0, 10};
  printf("Updating with the following E, P pairs... \n\n");
  for (int i = 0; i < num_new_ptys; i++){
    printf("E: %d P: %d\n", elts_upd[i], new_ptys[i]);
  }
  printf("\n");
  for (int i = 0; i < num_new_ptys; i++){
    updated_p = int_heap_update(&h, &elts_upd[i], &new_ptys[i]);
    printf("Updated? %d\n", updated_p);
    print_elts_ptys(&h);
  }
  printf("\n");

  // continue popping
  int num_res_pops = num_push - num_pops;
  printf("Continue popping %d elements... \n\n", num_res_pops);
  for (int i = 0; i < num_res_pops; i++){
    int_heap_pop(&h, &min_elt, &min_pty);
    printf("E: %d, P: %d\n", min_elt, min_pty);
    print_elts_ptys(&h);
  }
  printf("\n");
  int_heap_free(&h);
}
  

int main(){
  run_int_heap_test(); 
  return 0;
}
