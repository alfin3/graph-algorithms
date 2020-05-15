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
    In this example, a pointer to element is passed to heap_push that 
    fully copies the element to element array of heap.
*/

int cmp_int_elt(void *a, void *b){
  return *(int *)a - *(int *)b;
}

int cmp_int_pty(void *a, void *b){
  return *(int *)a - *(int *)b;
}

void free_int_fn(void *a){} //elements fully copied to element array

/**
   Prints a specified number of integer elements in element array.
*/
void print_int_elts(void *elts, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *((int *)elts + i));
  }
  printf("\n");
}

/**
   Prints a specified number of integer priorities in priority array.
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
   Pushes n integer elements and priorities onto heap.
*/
void push_int_elts(heap_t *h, int n){
  print_int_elts_int_ptys(h);
  for (int i = 0; i < n; i++){
    int pty = n - i;
    heap_push(h, &i, &pty);
    print_int_elts_int_ptys(h);
  }
  printf("\n");
}

/**
   Pops n elements and priorities of heap.
*/
void pop_int_elts(heap_t *h, int num_pops){
  int min_elt;
  int min_pty;
  for (int i = 0; i < num_pops; i++){
    heap_pop(h, &min_elt, &min_pty);
    printf("E: %d, P: %d\n", min_elt, min_pty);
    print_int_elts_int_ptys(h);
  }
  printf("\n"); 
}

/**
   Pops all elements and priorities of heap.
*/
void pop_all_int_elts(heap_t *h){
  int min_elt;
  int min_pty;
  while (h->num_elts > 0){
    heap_pop(h, &min_elt, &min_pty);
    printf("E: %d, P: %d\n", min_elt, min_pty);
    print_int_elts_int_ptys(h);
  }
  printf("\n"); 
}

/**
   Updates priorities of elements that are present on heap.
*/
void update_int_elts(heap_t *h, int num_upd, int elts_upd[], int new_ptys[]){
  int updated_p;
  printf("The following element priority pairs are used for updates: \n\n");
  for (int i = 0; i < num_upd; i++){
    printf("E: %d P: %d\n", elts_upd[i], new_ptys[i]);
  }
  printf("\n");
  for (int i = 0; i < num_upd; i++){
    updated_p = heap_update(h, &elts_upd[i], &new_ptys[i]);
    printf("Updated? %d\n", updated_p);
    print_int_elts_int_ptys(h);
  }
  printf("\n");
}

/**
   Runs an example of heap of integer elements and integer priorities.
*/
void run_int_int_heap_test(){
  printf("Running int int heap test... \n\n");
  heap_t h;
  int heap_init_size = 1;
  heap_init(&h,
	    heap_init_size,
	    sizeof(int),
	    sizeof(int),
	    cmp_int_elt,
	    cmp_int_pty,
	    free_int_fn);
  int num_push = 10;
  printf("Pushing %d elements... \n\n", num_push);
  push_int_elts(&h, num_push);
  int num_pops = 2;
  printf("Popping %d elements... \n\n", num_pops);
  pop_int_elts(&h, num_pops);
  printf("Updating... \n\n");
  int num_upd = 3;
  int elts_upd[] = {5, 5, 11};
  int new_ptys[] = {10, 0, 10};
  update_int_elts(&h, num_upd, elts_upd, new_ptys);
  printf("Popping all residual elements... \n\n");
  pop_all_int_elts(&h);
  printf("Pushing %d elements again... \n\n", num_push);
  push_int_elts(&h, num_push);
  printf("Freeing heap... \n\n");
  heap_free(&h);
}

/** 
    Dynamically allocated heap of int_ptr_t elements and long double 
    priorities. In this example, a pointer to an element pointer is passed 
    to heap_push that copies the element pointer to element array of heap.
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
   in element array.
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
   Prints a specified number of long double priorities in priority array.
*/
void print_l_dbl_ptys(void *ptys, int n){
  for (int i = 0; i < n; i++){
    printf("%.2Lf ", *((long double *)ptys + i));
  }
  printf("\n");
}

/**
   Prints integer values across int_ptr_t elements and long double priorities.
*/
void print_int_ptr_t_elts_l_dbl_ptys(heap_t *h){
  printf("Element array: ");
  print_int_ptr_t_elts(h->elts, h->num_elts);
  printf("Priority array: ");
  print_l_dbl_ptys(h->ptys, h->num_elts);
}

/**
   Pushes n int_ptr_t elements and long double priorities onto heap.
*/
void push_int_ptr_t_elts(heap_t *h, int n){
  int_ptr_t *s;
  print_int_ptr_t_elts_l_dbl_ptys(h);
  for (int i = 0; i < n; i++){
    long double pty = n - i;
    s = malloc(sizeof(int_ptr_t));
    assert(s != NULL);
    s->val = malloc(sizeof(int));
    assert(s->val != NULL);
    *(s->val) = i;
    heap_push(h, &s, &pty);
    print_int_ptr_t_elts_l_dbl_ptys(h);
    s = NULL;
  }
  printf("\n");
}

/**
   Pops n int_ptr_t elements and long double priorities of heap.
*/
void pop_int_ptr_t_elts(heap_t *h, int num_pops){
  int_ptr_t *min_elt;
  long double min_pty;
  for (int i = 0; i < num_pops; i++){
    heap_pop(h, &min_elt, &min_pty);
    printf("E: %d, P: %.2Lf\n", *(min_elt->val), min_pty);
    print_int_ptr_t_elts_l_dbl_ptys(h);
    free_int_ptr_t_fn(&min_elt);
    min_elt = NULL;
  }
  printf("\n"); 
}

/**
   Pops all elements and priorities of heap.
*/
void pop_all_int_ptr_t_elts(heap_t *h){
  int_ptr_t *min_elt;
  long double min_pty;
  while (h->num_elts > 0){
    heap_pop(h, &min_elt, &min_pty);
    printf("E: %d, P: %.2Lf\n", *(min_elt->val), min_pty);
    print_int_ptr_t_elts_l_dbl_ptys(h);
    free_int_ptr_t_fn(&min_elt);
    min_elt = NULL;
  }
  printf("\n"); 
}

/**
   Updates priorities of elements that are present on heap.
*/
void update_int_ptr_t_elts(heap_t *h,
			   int num_upd,
			   int elt_vals[],
			   long double new_ptys[]){
  int updated_p;
  int_ptr_t *s;
  printf("The following element priority pairs are used for updates: \n\n");
  for (int i = 0; i < num_upd; i++){
    printf("E: %d P: %.2Lf\n", elt_vals[i], new_ptys[i]);
  }
  printf("\n");
  for (int i = 0; i < num_upd; i++){
    s = malloc(sizeof(int_ptr_t));
    assert(s != NULL);
    s->val = malloc(sizeof(int));
    assert(s->val != NULL);
    *(s->val) = elt_vals[i];
    updated_p = heap_update(h, &s, &new_ptys[i]);
    printf("Updated? %d\n", updated_p);
    print_int_ptr_t_elts_l_dbl_ptys(h);
    free_int_ptr_t_fn(&s);
    s = NULL;
  }
  printf("\n");
}

/**
   Runs an example of heap of int_ptr_t elements and long double priorities.
*/
void run_int_ptr_t_l_dbl_heap_test(){
  printf("Running int_ptr_t long double heap test... \n\n");
  heap_t h;
  int heap_init_size = 1;
  heap_init(&h,
	    heap_init_size,
	    sizeof(int_ptr_t *),
	    sizeof(long double),
	    cmp_int_ptr_t_elt,
	    cmp_long_double_pty,
	    free_int_ptr_t_fn);
  int num_push = 10;
  printf("Pushing %d elements... \n\n", num_push);
  push_int_ptr_t_elts(&h, num_push);  
  int num_pops = 2;
  printf("Popping %d elements... \n\n", num_pops);
  pop_int_ptr_t_elts(&h, num_pops);
  printf("Updating... \n\n");
  int num_upd = 3;
  int elt_vals[] = {5, 5, 11};
  long double new_ptys[] = {10.0, 0.0, 10.0};
  printf("Updating with the following E, P pairs... \n\n");
  update_int_ptr_t_elts(&h, num_upd, elt_vals, new_ptys);
  printf("Continue popping the residual elements... \n\n");
  pop_all_int_ptr_t_elts(&h);
  printf("Pushing %d elements again... \n\n", num_push);
  push_int_ptr_t_elts(&h, num_push); 
  printf("Freeing heap... \n\n");
  heap_free(&h);
}

int main(){
  run_int_int_heap_test();
  run_int_ptr_t_l_dbl_heap_test();
  return 0;
}
