/**
   heap-main.c

   Examples of a generic dynamicaly allocated (min) heap with user-defined 
   comparison and deallocation functions.

*/

#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

int cmp_int(void *a, void *b){
  return *((int *)a) - *((int *)b);
}

/**
   Prints a specified number of priority values of a ptys array.
*/
void print_int_ptys(void *ptys, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *((int *)ptys + i));
  }
  printf("\n");
}

/**
   Prints a specified number of integer elements pointed to from elts array.
*/
void print_int_elts(void **elts, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *(int *)(elts[i]));
  }
  printf("\n");
}

int main(){
  heap_t h;
  int min_elt;
  int min_pty;
  int start_pty_val = 10;
  int heap_init_size = 1;
  int updated_p;
  int new_ptys[3] = {10, 0, 10};
  int elts[3] = {5, 5, 11};
  heap_init(&h,
	    heap_init_size,
	    sizeof(int),
	    sizeof(int),
	    cmp_int,
	    cmp_int,
	    NULL);
  for (int i = 0; i < start_pty_val; i++){
    int pty = start_pty_val - i;
    heap_push(&h, &i, &pty);
    printf("Element array: ");
    print_int_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_int_ptys(h.ptys, h.num_elts);
  }
  for (int i = 0; i < 2; i++){
    heap_pop(&h, &min_elt, &min_pty);
    printf("min elt: %d, min pty: %d\n", min_elt, min_pty);
    printf("Element array: ");
    print_int_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_int_ptys(h.ptys, h.num_elts);
  }
  for (int i = 0; i < 3; i++){
    updated_p = heap_update(&h, &elts[i], &new_ptys[i]);
    printf("updated? %d\n", updated_p);
    printf("Element array: ");
    print_int_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_int_ptys(h.ptys, h.num_elts);
  }
  heap_free(&h);
  return 0;
}
