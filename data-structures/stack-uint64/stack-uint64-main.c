/**
   stack-uint64-main.c

   Examples of a generic dynamically allocated stack with upto 
   (2^64 - 1) / elt_size elements.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the stack form.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include "stack-uint64.h"
#include "utilities-rand-mod.h"

void print_test_result(int result);

/**
   Run tests of a stack of uint64_t elements. A pointer to an element is 
   passed as elt in stack_uint64_push and the uint64_t element is fully 
   copied into the elts array. NULL as free_elt_fn is sufficient to free 
   the stack.
*/
static void uint64_stack_test_helper(stack_uint64_t *s,
				     uint64_t init_val,
				     uint64_t num_elts);

void run_uint64_stack_test(){
  stack_uint64_t s;
  uint64_t num_elts = 100000000;
  uint64_t init_stack_size = 1;
  uint64_t init_val = 1; //>= 1
  stack_uint64_init(&s, init_stack_size, sizeof(uint64_t), NULL);
  printf("Run a stack_uint64_{push, pop} test on uint64_t elements \n");
  printf("\tinitial stack size: %lu, initial value: %lu, "
	 "number of elements: %lu\n", init_stack_size, init_val, num_elts);
  uint64_stack_test_helper(&s, init_val, num_elts);
  printf("\tsame stack, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_stack_test_helper(&s, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame stack, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_stack_test_helper(&s, init_val, num_elts);
  stack_uint64_free(&s);
}

static void uint64_stack_test_helper(stack_uint64_t *s,
				     uint64_t init_val,
				     uint64_t num_elts){
  uint64_t popped;
  uint64_t cur_val = init_val + num_elts - 1;
  int result = 1;
  clock_t t;
  t = clock();
  for (uint64_t i = init_val; i < init_val + num_elts; i++){
    stack_uint64_push(s, &i);
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    stack_uint64_pop(s, &popped);
    result *= (cur_val == popped);
    cur_val--;
  }
  t = clock() - t;
  result *= (s->num_elts == 0);
  result *= (s->stack_size >= num_elts);
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness: ");
  print_test_result(result);
}

void run_uint64_stack_free_test(){
  stack_uint64_t s;
  uint64_t num_elts = 100000000;
  uint64_t init_stack_size = 1;
  clock_t t;
  stack_uint64_init(&s, init_stack_size, sizeof(uint64_t), NULL);
  printf("Run a stack_uint64_free test on %lu uint64_t elements\n",
	 num_elts);
  for (uint64_t i = 0; i < num_elts; i++){
    stack_uint64_push(&s, &i);
  }
  t = clock();
  stack_uint64_free(&s);
  t = clock() - t;
  printf("\t\tfree time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a stack of uint64_ptr_t elements. A pointer to a pointer to 
   an uint64_ptr_t element is passed as elt in stack_uint64_push and the 
   pointer to the uint64_ptr_t element is copied into the elts array. A 
   uint64_ptr_t-specific free_elt_fn is necessary to free the stack.
*/

typedef struct{
  uint64_t *val;
} uint64_ptr_t;

void free_uint64_ptr_fn(void *a){
  uint64_ptr_t **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

static void uint64_ptr_stack_test_helper(stack_uint64_t *s,
					 uint64_t init_val,
					 uint64_t num_elts);

void run_uint64_ptr_stack_test(){
  stack_uint64_t s;
  uint64_t num_elts = 10000000;
  uint64_t init_stack_size = 1;
  uint64_t init_val = 1; //>= 1
  stack_uint64_init(&s,
		    init_stack_size,
		    sizeof(uint64_ptr_t *),
		    free_uint64_ptr_fn);
  printf("Run a stack_uint64_{push, pop} test on multilayered uint64_ptr_t "
         "elements; time includes allocation and deallocation\n");
  printf("\tinitial stack size: %lu, initial value: %lu, "
	 "number of elements: %lu\n", init_stack_size, init_val, num_elts);
  uint64_ptr_stack_test_helper(&s, init_val, num_elts);
  printf("\tsame stack, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_ptr_stack_test_helper(&s, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame stack, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_ptr_stack_test_helper(&s, init_val, num_elts);
  stack_uint64_free(&s);
}

static void uint64_ptr_stack_test_helper(stack_uint64_t *s,
					 uint64_t init_val,
					 uint64_t num_elts){
  uint64_ptr_t *pushed;
  uint64_ptr_t *popped;
  uint64_t cur_val = init_val + num_elts - 1;
  int result = 1;
  clock_t t;
  t = clock();
  for (uint64_t i = init_val; i < init_val + num_elts; i++){
    pushed = malloc(sizeof(uint64_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(uint64_t));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    stack_uint64_push(s, &pushed);
    pushed = NULL;
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    stack_uint64_pop(s, &popped);
    result *= (cur_val == *(popped->val));
    cur_val--;
    free_uint64_ptr_fn(&popped);
    popped = NULL;
  }
  t = clock() - t;
  result *= (s->num_elts == 0);
  result *= (s->stack_size >= num_elts);
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness: ");
  print_test_result(result);
}

void run_uint64_ptr_stack_free_test(){
  stack_uint64_t s;
  uint64_ptr_t *pushed;
  uint64_t num_elts = 10000000;
  uint64_t init_stack_size = 1;
  clock_t t;
  stack_uint64_init(&s,
		    init_stack_size,
		    sizeof(uint64_ptr_t *),
		    free_uint64_ptr_fn);
  printf("Run a stack_uint64_free test on %lu multilayered "
         "uint64_ptr_t elements \n", num_elts);
  for (uint64_t i = 0; i < num_elts; i++){
    pushed = malloc(sizeof(uint64_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(uint64_t));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    stack_uint64_push(&s, &pushed);
    pushed = NULL;
  }
  t = clock();
  stack_uint64_free(&s);
  t = clock() - t;
  printf("\t\tfree time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run a test of a stack of 5 billion char elements.
*/

void run_large_stack_test(){
  stack_uint64_t s;
  char c = 0;
  uint64_t num_elts = 5000000000;
  uint64_t init_stack_size = 1;
  clock_t t;
  stack_uint64_init(&s, init_stack_size, sizeof(char), NULL);
  printf("Run a stack_uint64_{push, pop} test " 
         "on %lu char elements; requires sufficient memory \n", num_elts);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    stack_uint64_push(&s, &c);
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    stack_uint64_pop(&s, &c);
  }
  t = clock() - t;
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  stack_uint64_free(&s);
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_uint64_stack_test();
  run_uint64_stack_free_test();
  run_uint64_ptr_stack_test();
  run_uint64_ptr_stack_free_test();
  run_large_stack_test();
  return 0;
}
