/**
   heap-main.c

   Examples of generic dynamicaly allocated (min) heap with user-defined 
   comparison and deallocation functions.

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "heap.h"

/** Heap of integer elements and integer priorities */

int cmp_int_elt(void *a, void *b){
  return *(int *)a - *(int *)b;
}

int cmp_int_pty(void *a, void *b){
  return *(int *)a - *(int *)b;
}

/**
   Prints a specified number of integer elements pointed from elts array.
*/
void print_int_elts(void **elts, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *(int *)(elts[i]));
  }
  printf("\n");
}

/**
   Prints a specified number of integer priority values of a ptys array.
*/
void print_int_ptys(void *ptys, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *((int *)ptys + i));
  }
  printf("\n");
}

/**
   Runs an example of heap of integer elements and integer priorities.
*/
void run_int_int_heap(){
  //init
  heap_t h;
  int heap_init_size = 1;
  //push
  int start_pty_val = 10;
  //pop
  int num_pops = 2;
  int min_elt;
  int min_pty;
  //update
  int updated_p;
  int num_new_ptys = 3;
  int new_ptys[] = {10, 0, 10};
  int elts[] = {5, 5, 11};
  heap_init(&h,
	    heap_init_size,
	    sizeof(int),
	    sizeof(int),
	    cmp_int_elt,
	    cmp_int_pty,
	    NULL);
  for (int i = 0; i < start_pty_val; i++){
    int pty = start_pty_val - i;
    heap_push(&h, &i, &pty);
    printf("Element array: ");
    print_int_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_int_ptys(h.ptys, h.num_elts);
  }
  printf("\n");
  for (int i = 0; i < num_pops; i++){
    heap_pop(&h, &min_elt, &min_pty);
    printf("min elt: %d, min pty: %d\n", min_elt, min_pty);
    printf("Element array: ");
    print_int_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_int_ptys(h.ptys, h.num_elts);
  }
  printf("\n");
  for (int i = 0; i < num_new_ptys; i++){
    updated_p = heap_update(&h, &elts[i], &new_ptys[i]);
    printf("updated? %d\n", updated_p);
    printf("Element array: ");
    print_int_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_int_ptys(h.ptys, h.num_elts);
  }
  printf("\n");
  heap_free(&h);
}

/** Heap of int_ptr_t elements and long double priorities */

typedef struct{
  int *val;
} int_ptr_t;

int cmp_int_ptr_t_elt(void *a, void *b){
  int *a_val = ((int_ptr_t *)a)->val;
  int *b_val = ((int_ptr_t *)b)->val;
  return *a_val - *b_val;
}

int cmp_long_double_pty(void *a, void *b){
  long double *a_val = a;
  long double *b_val = b;
  if (*a_val == *b_val){
    return 0;
  }else if (*a_val > *b_val){
    return 1;
  }else{
    return -1;
  }
}

void free_elt_fn(void *a){
  int *a_val = ((int_ptr_t *)a)->val;
  free(a_val);
  free(a);
}

/**
   Prints a specified number of integer values across int_ptr_t elements
   pointed from elts array.
*/
void print_int_ptr_t_elts(void **elts, int n){
  for (int i = 0; i < n; i++){
    int *val = ((int_ptr_t *)elts[i])->val;
    printf("%d ", *val);
  }
  printf("\n");
}

/**
   Prints a specified number of long double priority values of a ptys array.
*/
void print_long_double_ptys(void *ptys, int n){
  for (int i = 0; i < n; i++){
    printf("%Lf ", *((long double *)ptys + i));
  }
  printf("\n");
}

/**
   Runs an example of heap of int_ptr_t elements and long double priorities.
*/
void run_int_ptr_t_long_double_heap(){
  //init
  heap_t h;
  int heap_init_size = 1;
  //push
  int_ptr_t *s;
  int start_pty_val = 10;
  //pop
  int num_pops = 2;
  int_ptr_t min_elt;
  long double min_pty;;
  //update
  int updated_p;
  int num_new_ptys = 3;
  int_ptr_t elts[num_new_ptys];
  int elt_vals[] = {5, 5, 11};
  long double new_ptys[] = {10.0, 0.0, 10.0};
  heap_init(&h,
	    heap_init_size,
	    sizeof(int_ptr_t),
	    sizeof(long double),
	    cmp_int_ptr_t_elt,
	    cmp_long_double_pty,
	    free_elt_fn);
  for (int i = 0; i < start_pty_val; i++){
    long double pty = start_pty_val - i;
    s = (int_ptr_t *)malloc(sizeof(int_ptr_t));
    assert(s != NULL);
    s->val = (int *)malloc(sizeof(int));
    assert(s->val != NULL);
    *(s->val) = i;
    heap_push(&h, s, &pty);
    printf("Element array: ");
    print_int_ptr_t_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_long_double_ptys(h.ptys, h.num_elts);
    free(s); //free only the memory of the top level object of element
    s = NULL;
  }
  printf("\n");
  for (int i = 0; i < num_pops; i++){
    heap_pop(&h, &min_elt, &min_pty);
    printf("min elt: %d, min pty: %Lf\n", *(min_elt.val), min_pty);
    printf("Element array: ");
    print_int_ptr_t_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_long_double_ptys(h.ptys, h.num_elts);
  }
  printf("\n");
  for (int i = 0; i < num_new_ptys; i++){
    s = (int_ptr_t *)malloc(sizeof(int_ptr_t));
    assert(s != NULL);
    s->val = (int *)malloc(sizeof(int));
    assert(s->val != NULL);
    *(s->val) = elt_vals[i];
    updated_p = heap_update(&h, s, &new_ptys[i]);
    printf("updated? %d\n", updated_p);
    printf("Element array: ");
    print_int_ptr_t_elts(h.elts, h.num_elts);
    printf("Priority array: ");
    print_long_double_ptys(h.ptys, h.num_elts);
    free_elt_fn(s); //free the entire element
    s = NULL;
  }
  printf("\n");
  heap_free(&h);
}

int main(){
  run_int_int_heap();
  run_int_ptr_t_long_double_heap();
  return 0;
}
