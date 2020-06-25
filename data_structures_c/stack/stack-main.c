/**
   stack-main.c

   Examples of a generic dynamicaly allocated stack.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "stack.h"

void print_test_result(int result);

/**
   Run a test of a stack of integer elements. A pointer to an integer is 
   passed as elt in stack_push and the integer element is fully copied into 
   the elts array. NULL as free_int_fn is sufficient to free the stack.
*/
static void int_stack_test_helper(stack_t *s, int num_elts);

void run_int_stack_test(){
  stack_t s;
  int num_elts = 100000000;
  int stack_init_size = 1;
  stack_init(&s, stack_init_size, sizeof(int), NULL);
  printf("Run stack test on %d int elements and %d as the initial size of "
	 "the stack \n", num_elts, stack_init_size);
  int_stack_test_helper(&s, num_elts);
  printf("Run stack test on %d int elements and the same stack \n", num_elts);
  int_stack_test_helper(&s, num_elts);
  stack_free(&s);
}

static void int_stack_test_helper(stack_t *s, int num_elts){
  int popped;
  int cur_val = num_elts - 1;
  int result = 1;
  clock_t t;
  t = clock();
  for (int i = 0; i < num_elts; i++){
    stack_push(s, &i);
  }
  t = clock() - t;
  printf("\tTime of pushing: %.4f seconds \n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (int i = 0; i < num_elts; i++){
    stack_pop(s, &popped);
    result *= (cur_val == popped);
    cur_val--;
  }
  t = clock() - t;
  result *= (s->num_elts == 0);
  result *= (s->stack_size >= num_elts);
  printf("\tTime of popping: %.4f seconds \n", (float)t / CLOCKS_PER_SEC);
  printf("\tOrder correctness --> ");
  print_test_result(result);
}

/**
   Run a test of a stack of int_ptr_t elements. A pointer to a pointer to an
   int_ptr_t element is passed as elt in stack_push and the pointer to the
   int_ptr_t element is copied into the elts array. A int_ptr_t-specific 
   free_int_fn is necessary to free the stack.
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

static void int_ptr_t_stack_test_helper(stack_t *s, int num_elts);

void run_int_ptr_t_stack_test(){
  stack_t s;
  int num_elts = 10000000;
  int stack_init_size = 1;
  stack_init(&s, stack_init_size, sizeof(int_ptr_t *), free_int_ptr_t_fn);
  printf("Run stack test on %d int_ptr_t elements and %d as the initial " 
         "size of the stack \n", num_elts, stack_init_size);
  int_ptr_t_stack_test_helper(&s, num_elts);
  printf("Run stack test on %d int_ptr_t elements and the same stack \n",
	 num_elts);
  int_ptr_t_stack_test_helper(&s, num_elts);
  stack_free(&s);
}

static void int_ptr_t_stack_test_helper(stack_t *s, int num_elts){
  int_ptr_t *pushed;
  int_ptr_t *popped;
  int cur_val = num_elts - 1;
  int result = 1;
  clock_t t;
  t = clock();
  for (int i = 0; i < num_elts; i++){
    pushed = malloc(sizeof(int_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(int));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    stack_push(s, &pushed);
    pushed = NULL;
  }
  t = clock() - t;
  printf("\tTime of pushing and element allocation: %.4f seconds \n",
	 (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (int i = 0; i < num_elts; i++){
    stack_pop(s, &popped);
    result *= (cur_val == *(popped->val));
    cur_val--;
    free_int_ptr_t_fn(&popped);
    popped = NULL;
  }
  t = clock() - t;
  result *= (s->num_elts == 0);
  result *= (s->stack_size >= num_elts);
  printf("\tTime of popping and element deallocation: %.4f seconds \n",
	 (float)t / CLOCKS_PER_SEC);
  printf("\tOrder correctness --> ");
  print_test_result(result);
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_int_stack_test();
  run_int_ptr_t_stack_test();
  return 0;
}
