/**
   heap-uint32-main.c

   Examples of a generic dynamically allocated (min) heap with upto 2^32 - 2
   elements.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "heap-uint32.h"

/** 
    A dynamically allocated heap of integer elements and integer priorities.
    In this example, a pointer to an element is passed as elt in heap_push 
    that fully copies the element into the elts array of the heap.
*/

int cmp_int_elt(void *a, void *b){
  return *(int *)a - *(int *)b;
}

int cmp_int_pty(void *a, void *b){
  return *(int *)a - *(int *)b;
}

/**
   Prints a specified number of integer elements in an element array.
*/
void print_int_elts(void *elts, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *((int *)elts + i));
  }
  printf("\n");
}

/**
   Prints a specified number of integer priorities in a priority array.
*/
void print_int_ptys(void *ptys, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *((int *)ptys + i));
  }
  printf("\n");
}

/**
   Prints integer elements and integer priorities.
*/
void print_int_elts_ptys(heap_t *h){
  printf("Element array: ");
  print_int_elts(h->elts, h->num_elts);
  printf("Priority array: ");
  print_int_ptys(h->ptys, h->num_elts);
}

/**
   Pushes n integer elements and priorities.
*/
void push_int_elts(heap_t *h, int n){
  print_int_elts_ptys(h);
  for (int i = 0; i < n; i++){
    int pty = n - i;
    heap_push(h, &pty, &i);
    print_int_elts_ptys(h);
  }
  printf("\n");
}

/**
   Pops n elements and priorities.
*/
void pop_int_elts(heap_t *h, int n){
  int min_elt;
  int min_pty;
  for (int i = 0; i < n; i++){
    heap_pop(h, &min_pty, &min_elt);
    printf("E: %d, P: %d\n", min_elt, min_pty);
    print_int_elts_ptys(h);
  }
  printf("\n"); 
}

/**
   Pops all elements and priorities.
*/
void pop_all_int_elts(heap_t *h){
  int min_elt;
  int min_pty;
  while (h->num_elts > 0){
    heap_pop(h, &min_pty, &min_elt);
    printf("E: %d, P: %d\n", min_elt, min_pty);
    print_int_elts_ptys(h);
  }
  printf("\n"); 
}

/**
   Updates priorities of elements that are present in a heap.
*/
void update_int_elts(heap_t *h, int n, int elts_upd[], int new_ptys[]){
  printf("The following element priority pairs are used for updates: \n\n");
  for (int i = 0; i < n; i++){
    printf("E: %d P: %d\n",  elts_upd[i], new_ptys[i]);
  }
  printf("\n");
  for (int i = 0; i < n; i++){
    heap_update(h, &(new_ptys[i]), &(elts_upd[i]));
    print_int_elts_ptys(h);
  }
  printf("\n");
}

/**
   Runs an example of a heap with integer elements and integer priorities.
*/
void run_int_heap_test(){
  printf("Running int int heap test... \n\n");
  heap_t h;
  uint32_t heap_init_size = 1;
  heap_init(&h,
	    heap_init_size,
	    sizeof(int),
	    sizeof(int),
	    cmp_int_elt,
	    cmp_int_pty,
	    NULL);
  int num_push = 10;
  printf("Pushing %d elements... \n\n", num_push);
  push_int_elts(&h, num_push);
  int num_pop = 2;
  printf("Popping %d elements... \n\n", num_pop);
  pop_int_elts(&h, num_pop);
  printf("Updating... \n\n");
  int num_upd = 2;
  int elts_upd[] = {5, 5};
  int new_ptys[] = {10, 0};
  update_int_elts(&h, num_upd, elts_upd, new_ptys);
  printf("Popping all residual elements... \n\n");
  pop_all_int_elts(&h);
  printf("Pushing %d elements again... \n\n", num_push);
  push_int_elts(&h, num_push);
  printf("Freeing heap... \n\n");
  heap_free(&h);
}

int main(){
  run_int_heap_test();
  return 0;
}
