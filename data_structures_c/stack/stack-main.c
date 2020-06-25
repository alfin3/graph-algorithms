/**
   stack-main.c

   Examples of a generic dynamically allocated stack.

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
   Run tests of a stack of integer elements. A pointer to an integer is 
   passed as elt in stack_push and the integer element is fully copied into 
   the elts array. NULL as free_int_fn is sufficient to free the stack.
*/
static void int_stack_test_helper(stack_t *s,
				  int init_val,
				  int num_elts);

void run_int_stack_test(){
  stack_t s;
  int num_elts = 100000000;
  int stack_init_size = 1;
  int init_val;
  stack_init(&s, stack_init_size, sizeof(int), NULL);
  init_val = 0;
  printf("Run stack test on int elements, initial stack size: %d, "
	 "initial value: %d, number of elements: %d\n",
	 stack_init_size, init_val, num_elts);
  int_stack_test_helper(&s, init_val, num_elts);
  printf("Run stack test on int elements, same stack, "
	 "initial value: %d, number of elements: %d\n",
	 init_val, num_elts);
  int_stack_test_helper(&s, init_val, num_elts);
  init_val = num_elts;
  printf("Run stack test on int elements, same stack, "
	 "initial value: %d, number of elements: %d\n",
	 init_val, num_elts);
  int_stack_test_helper(&s, init_val, num_elts);
  stack_free(&s);
}

static void int_stack_test_helper(stack_t *s,
				  int init_val,
				  int num_elts){
  int popped;
  int cur_val = init_val + num_elts - 1;
  int result = 1;
  clock_t t;
  t = clock();
  for (int i = init_val; i < init_val + num_elts; i++){
    stack_push(s, &i);
  }
  t = clock() - t;
  printf("\tTime of pushing: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (int i = 0; i < num_elts; i++){
    stack_pop(s, &popped);
    result *= (cur_val == popped);
    cur_val--;
  }
  t = clock() - t;
  result *= (s->num_elts == 0);
  result *= (s->stack_size >= num_elts);
  printf("\tTime of popping: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\tOrder correctness --> ");
  print_test_result(result);
}

void run_int_stack_free_test(){
  stack_t s;
  int num_elts = 100000000;
  int stack_init_size = 1;
  clock_t t;
  stack_init(&s, stack_init_size, sizeof(int), NULL);
  printf("Run stack_free test on %d int elements\n", num_elts);
  for (int i = 0; i < num_elts; i++){
    stack_push(&s, &i);
  }
  t = clock();
  stack_free(&s);
  t = clock() - t;
  printf("\tTime of freeing: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a stack of int_ptr_t elements. A pointer to a pointer to an
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

static void int_ptr_t_stack_test_helper(stack_t *s,
					int init_val,
					int num_elts);

void run_int_ptr_t_stack_test(){
  stack_t s;
  int num_elts = 10000000;
  int stack_init_size = 1;
  int init_val;
  stack_init(&s, stack_init_size, sizeof(int_ptr_t *), free_int_ptr_t_fn);
  init_val = 0;
  printf("Run stack test on int_ptr_t elements, initial stack size: %d, "
	 "initial value: %d, number of elements: %d\n",
	 stack_init_size, init_val, num_elts);
  int_ptr_t_stack_test_helper(&s, init_val, num_elts);
  printf("Run stack test on int_ptr_t elements, same stack, "
	 "initial value: %d, number of elements: %d\n",
	 init_val, num_elts);
  int_ptr_t_stack_test_helper(&s, init_val, num_elts);
  init_val = num_elts;
  printf("Run stack test on int_ptr_t elements, same stack, "
	 "initial value: %d, number of elements: %d\n",
	 init_val, num_elts);
  int_ptr_t_stack_test_helper(&s, init_val, num_elts);
  stack_free(&s);
}

static void int_ptr_t_stack_test_helper(stack_t *s,
					int init_val,
					int num_elts){
  int_ptr_t *pushed;
  int_ptr_t *popped;
  int cur_val = init_val + num_elts - 1;
  int result = 1;
  clock_t t;
  t = clock();
  for (int i = init_val; i < init_val + num_elts; i++){
    pushed = malloc(sizeof(int_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(int));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    stack_push(s, &pushed);
    pushed = NULL;
  }
  t = clock() - t;
  printf("\tTime of pushing: %.4f seconds (incl. element allocation)\n",
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
  printf("\tTime of popping: %.4f seconds (incl. element deallocation)\n",
	 (float)t / CLOCKS_PER_SEC);
  printf("\tOrder correctness --> ");
  print_test_result(result);
}

void run_int_ptr_t_stack_free_test(){
  stack_t s;
  int_ptr_t *pushed;
  int num_elts = 10000000;
  int stack_init_size = 1;
  clock_t t;
  stack_init(&s, stack_init_size, sizeof(int_ptr_t *), free_int_ptr_t_fn);
  printf("Run stack_free test on %d int_ptr_t elements\n", num_elts);
  for (int i = 0; i < num_elts; i++){
    pushed = malloc(sizeof(int_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(int));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    stack_push(&s, &pushed);
    pushed = NULL;
  }
  t = clock();
  stack_free(&s);
  t = clock() - t;
  printf("\tTime of freeing: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
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
  run_int_stack_free_test();
  run_int_ptr_t_stack_free_test();
  return 0;
}
