/**
   heap-main.c

   Examples of generic dynamically allocated (min) heap with user-defined 
   comparison and deallocation functions.

   Through user-defined comparison and deallocation functions, the 
   implementation provides a dynamic set in heap form of any objects 
   associated with priority values of basic type of choice (e.g. char, int, 
   long, double). 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "heap.h"

/** 
    Dynamically allocated heap of integer elements and integer priorities.
    In this example, pointer to element is passed to heap_push that 
    fully copies the element to elts array of heap.
*/

int cmp_int_elt(void *a, void *b){
  return *(int *)a - *(int *)b;
}

int cmp_int_pty(void *a, void *b){
  return *(int *)a - *(int *)b;
}

void free_int_fn(void *a){} //fully in elts array, freed with free(h->elts)

/**
   Prints a specified number of integer elements in elts array.
*/
void print_int_elts(void *elts, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *((int *)elts + i));
  }
  printf("\n");
}

/**
   Prints a specified number of integer priorities in ptys array.
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
void print_int_elts_int_ptys(heap_t *h){
  printf("Element array: ");
  print_int_elts(h->elts, h->num_elts);
  printf("Priority array: ");
  print_int_ptys(h->ptys, h->num_elts);
}

/**
   Runs an example of heap of integer elements and integer priorities.
*/

void run_int_int_heap(){
  //initialize a heap
  heap_t h;
  int heap_init_size = 1;
  heap_init(&h,
	    heap_init_size,
	    sizeof(int),
	    sizeof(int),
	    cmp_int_elt,
	    cmp_int_pty,
	    free_int_fn);
  //push elements 
  int start_pty_val = 10;
  for (int i = 0; i < start_pty_val; i++){
    int pty = start_pty_val - i;
    heap_push(&h, &i, &pty);
    print_int_elts_int_ptys(&h);
  }
  printf("\n");
  //pop num_pops elements and priority values
  int min_elt;
  int min_pty;
  int num_pops = 2;
  for (int i = 0; i < num_pops; i++){
    heap_pop(&h, &min_elt, &min_pty);
    printf("min elt: %d, min pty: %d\n", min_elt, min_pty);
    print_int_elts_int_ptys(&h);
  }
  printf("\n");
  //update heap
  int updated_p;
  int num_new_ptys = 3;
  int elts_upd[] = {5, 5, 11};
  int new_ptys[] = {10, 0, 10};
  for (int i = 0; i < num_new_ptys; i++){
    updated_p = heap_update(&h, &elts_upd[i], &new_ptys[i]);
    printf("updated? %d\n", updated_p);
    print_int_elts_int_ptys(&h);
  }
  printf("\n");
  heap_free(&h);
}

/** 
    Dynamically allocated heap of int_ptr_t elements and long double 
    priorities. In this example, pointer to element pointer is passed 
    to heap_push that copies the element pointer to elts array of heap.
*/

typedef struct{
  int *val;
} int_ptr_t;

int cmp_int_ptr_t_elt(void *a, void *b){
  int *a_val = (*(int_ptr_t **)a)->val;
  int *b_val = (*(int_ptr_t **)b)->val;
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

void free_int_ptr_t_fn(void *a){
  int_ptr_t **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

/**
   Prints a specified number of integer values across int_ptr_t elements
   in elts array.
*/
void print_int_ptr_t_elts(void *elts, int n){
  int_ptr_t **s = elts;
  for (int i = 0; i < n; i++){
    
    int *val = (*(s + i))->val;
    printf("%d ", *val);
  }
  printf("\n");
}

/**
   Prints a specified number of long double priority values in ptys array.
*/
void print_long_double_ptys(void *ptys, int n){
  for (int i = 0; i < n; i++){
    printf("%Lf ", *((long double *)ptys + i));
  }
  printf("\n");
}

/**
   Prints integer values across int_ptr_t elements and long double priorities.
*/
void print_int_ptr_t_elts_long_double_ptys(heap_t *h){
  printf("Element array: ");
  print_int_ptr_t_elts(h->elts, h->num_elts);
  printf("Priority array: ");
  print_long_double_ptys(h->ptys, h->num_elts);
}

/**
   Runs an example of heap of int_ptr_t elements and long double priorities.
*/
void run_int_ptr_t_long_double_heap(){
  //initialize a heap
  heap_t h;
  int heap_init_size = 1;
  heap_init(&h,
	    heap_init_size,
	    sizeof(int_ptr_t *),
	    sizeof(long double),
	    cmp_int_ptr_t_elt,
	    cmp_long_double_pty,
	    free_int_ptr_t_fn);
  //dynamically allocate and push elements
  int_ptr_t *s;
  int start_pty_val = 10;
  for (int i = 0; i < start_pty_val; i++){
    long double pty = start_pty_val - i;
    s = (int_ptr_t *)malloc(sizeof(int_ptr_t));
    assert(s != NULL);
    s->val = (int *)malloc(sizeof(int));
    assert(s->val != NULL);
    *(s->val) = i;
    heap_push(&h, &s, &pty);
    print_int_ptr_t_elts_long_double_ptys(&h);
    s = NULL;
  }
  printf("\n");
  //pop num_pops elements and priority values
  int_ptr_t *min_elt;
  long double min_pty;
  int num_pops = 2;
  for (int i = 0; i < num_pops; i++){
    heap_pop(&h, &min_elt, &min_pty);
    printf("min elt: %d, min pty: %Lf\n", *(min_elt->val), min_pty);
    print_int_ptr_t_elts_long_double_ptys(&h);
    free_int_ptr_t_fn(&min_elt);
    min_elt = NULL;
  }
  printf("\n");
  //update heap
  int updated_p;
  int num_new_ptys = 3;
  int elt_vals[] = {5, 5, 11};
  long double new_ptys[] = {10.0, 0.0, 10.0};
  for (int i = 0; i < num_new_ptys; i++){
    s = (int_ptr_t *)malloc(sizeof(int_ptr_t));
    assert(s != NULL);
    s->val = (int *)malloc(sizeof(int));
    assert(s->val != NULL);
    *(s->val) = elt_vals[i];
    updated_p = heap_update(&h, &s, &new_ptys[i]);
    printf("updated? %d\n", updated_p);
    print_int_ptr_t_elts_long_double_ptys(&h);
    free_int_ptr_t_fn(&s);
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
