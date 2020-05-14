/**
   stack-main.c

   Examples of generic dynamicaly allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set in stack form of any objects. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"

/** 
    Dynamically allocated stack of integer elements. In this test example, 
    a pointer to element is passed to stack_push that fully copies the 
    element to element array of stack.
*/

void free_int_fn(void *a){} //fully in elts array

/**
   Prints a specified number of integer elements in element array of stack.
*/
void print_int_elts(void *elts, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *((int *)elts + i));
  }
  printf("\n");
}

/**
   Prints all integer elements in element array of stack.
*/
void print_all_int_elts(stack_t *s){
  printf("Element array: ");
  print_int_elts(s->elts, s->num_elts);
}

/**
   Runs a test example of stack of integer elements.
*/

void run_int_stack_test(){
  printf("Running int stack test... \n\n");
  
  //initialize a stack
  stack_t s;
  int stack_init_size = 1;
  stack_init(&s,
	     stack_init_size,
	     sizeof(int),
	     free_int_fn);
  
  //push elements 
  int num_push = 10;
  printf("Pushing %d elements... \n\n", num_push);
  print_all_int_elts(&s);
  for (int i = 0; i < num_push; i++){
    stack_push(&s, &i);
    print_all_int_elts(&s);
  }
  printf("\n");
  
  //pop num_push elements
  int popped_elt;
  printf("Popping %d elements... \n\n", num_push);
  for (int i = 0; i < num_push; i++){
    stack_pop(&s, &popped_elt);
    printf("E: %d \n", popped_elt);
    print_all_int_elts(&s);
  }
  printf("\n");
  stack_free(&s);
}

/** 
    Dynamically allocated stack of int_ptr_t elements. In this test example, 
    a pointer to an element pointer is passed to stack_push that copies 
    the element pointer to element array of stack.
*/

typedef struct{
  int *val;
} int_ptr_t;

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
   in element array of stack.
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
   Prints integer values across int_ptr_t elements in element array of stack.
*/
void print_all_int_ptr_t_elts(stack_t *s){
  printf("Element array: ");
  print_int_ptr_t_elts(s->elts, s->num_elts);
}

/**
   Runs a test example of stack of int_ptr_t elements.
*/
void run_int_ptr_t_stack_test(){
  printf("Running int_ptr_t stack test... \n\n");
  
  //initialize a stack
  stack_t s;
  int stack_init_size = 1;
  stack_init(&s,
	     stack_init_size,
	     sizeof(int_ptr_t *),
	     free_int_ptr_t_fn);
  
  //dynamically allocate and push elements
  int_ptr_t *a;
  int num_push = 10;
  printf("Pushing %d elements... \n\n", num_push);
  print_all_int_ptr_t_elts(&s);
  for (int i = 0; i < num_push; i++){
    a = (int_ptr_t *)malloc(sizeof(int_ptr_t));
    assert(a != NULL);
    a->val = (int *)malloc(sizeof(int));
    assert(a->val != NULL);
    *(a->val) = i;
    stack_push(&s, &a);
    print_all_int_ptr_t_elts(&s);
    a = NULL;
  }
  printf("\n");
  
  //pop num_push elements
  int_ptr_t *popped_elt;
  printf("Popping %d elements... \n\n", num_push);
  for (int i = 0; i < num_push; i++){
    stack_pop(&s, &popped_elt);
    printf("E: %d \n", *(popped_elt->val));
    print_all_int_ptr_t_elts(&s);
    free_int_ptr_t_fn(&popped_elt);
    popped_elt = NULL;
  }
  printf("\n");
  stack_free(&s);
}

int main(){
  run_int_stack_test();
  run_int_ptr_t_stack_test();
  return 0;
}
