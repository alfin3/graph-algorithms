/**
   stack-main.c

   Tests of a generic dynamically allocated stack.

   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "stack.h"
#include "utilities-mem.h"

void print_test_result(int res);

/**
   Run tests of a stack of uint64_t elements. A pointer to an element is
   passed as elt in stack_push and the uint64_t element is fully
   copied onto the stack. NULL as free_elt is sufficient to free
   the stack.
*/

void uint64_push_pop_helper(stack_t *s,
			    uint64_t init_val,
			    uint64_t num_elts);

void run_uint64_push_pop_test(){
  uint64_t num_elts = 50000000;
  uint64_t init_count = 1;
  uint64_t init_val = 1;
  stack_t s;
  stack_init(&s, init_count, sizeof(uint64_t), NULL);
  printf("Run a stack_{push, pop} test on uint64_t elements \n");
  printf("\tinitial stack count: %lu, "
	 "initial value: %lu, "
	 "number of elements: %lu\n",
	 init_count, init_val, num_elts);
  uint64_push_pop_helper(&s, init_val, num_elts);
  printf("\tsame stack, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_push_pop_helper(&s, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame stack, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_push_pop_helper(&s, init_val, num_elts);
  stack_free(&s);
}

void uint64_push_pop_helper(stack_t *s,
			    uint64_t init_val,
			    uint64_t num_elts){
  int res = 1;
  uint64_t *pushed = NULL, *popped = NULL;
  uint64_t i;
  clock_t t_push, t_pop;
  pushed = malloc_perror(num_elts * sizeof(uint64_t));
  popped = malloc_perror(num_elts * sizeof(uint64_t));
  for (i = 0; i < num_elts; i++){
    pushed[i] = init_val + i;
  }
  t_push = clock();
  for (i = 0; i < num_elts; i++){
    stack_push(s, &pushed[i]);
  }
  t_push = clock() - t_push;
  t_pop = clock();
  for (i = 0; i < num_elts; i++){
    stack_pop(s, &popped[i]);
  }
  t_pop = clock() - t_pop;
  res *= (s->num_elts == 0);
  res *= (s->count >= num_elts);
  for (i = 0; i < num_elts; i++){
    res *= (popped[i] == num_elts - 1 + init_val - i);
  }
  printf("\t\tpush time:   %.4f seconds\n", (float)t_push / CLOCKS_PER_SEC);
  printf("\t\tpop time:    %.4f seconds\n", (float)t_pop / CLOCKS_PER_SEC);
  printf("\t\tcorrectness: ");
  print_test_result(res);
  free(pushed);
  free(popped);
  pushed = NULL;
  popped = NULL;
}

void run_uint64_first_test(){
  int res = 1;
  uint64_t num_elts = 50000000;
  uint64_t init_count = 1;
  uint64_t init_val = 1;
  uint64_t pushed, popped;
  uint64_t i;
  stack_t s;
  stack_init(&s, init_count, sizeof(uint64_t), NULL);
  printf("Run a stack_first test on uint64_t elements \n");
  printf("\tinit stack count: %lu\n"
	 "\t#elements:        %lu\n",
	 init_count,  num_elts);
  for (i = 0; i < num_elts; i++){
    if (s.num_elts == 0){
      res *= (stack_first(&s) == NULL);
    }else{
      res *= (*(uint64_t *)stack_first(&s) == init_val + i - 1);
    }
    pushed = init_val + i;
    stack_push(&s, &pushed);
    res *= (*(uint64_t *)stack_first(&s) == init_val + i);
  }
  for (i = 0; i < num_elts; i++){
    res *= (*(uint64_t *)stack_first(&s) == num_elts - 1 + init_val - i);
    stack_pop(&s, &popped);
    if (s.num_elts == 0){
      res *= (stack_first(&s) == NULL);
    }else{
      res *= (*(uint64_t *)stack_first(&s) == num_elts - 2 + init_val - i);
    }
  }
  res *= (s.num_elts == 0);
  res *= (s.count >= num_elts);
  printf("\tcorrectness:      ");
  print_test_result(res);
  stack_free(&s);
}

void run_uint64_free_test(){
  uint64_t num_elts = 50000000;
  uint64_t init_count = 1;
  uint64_t i;
  stack_t s;
  clock_t t;
  stack_init(&s, init_count, sizeof(uint64_t), NULL);
  printf("Run a stack_free test on uint64_t elements\n");
  printf("\t# elements:       %lu\n", num_elts);
  for (i = 0; i < num_elts; i++){
    stack_push(&s, &i);
  }
  t = clock();
  stack_free(&s);
  t = clock() - t;
  printf("\tfree time:        %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a stack of uint64_ptr_t elements. A pointer to a pointer to
   an uint64_ptr_t element is passed as elt in stack_push and the pointer to
   the uint64_ptr_t element is copied onto the stack. A uint64_ptr_t-
   specific free_elt, taking a pointer to a pointer to an element as its
   parameter, is necessary to free the stack.
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

void uint64_ptr_push_pop_helper(stack_t *s,
				uint64_t init_val,
				uint64_t num_elts);

void run_uint64_ptr_push_pop_test(){
  uint64_t num_elts = 50000000;
  uint64_t init_count = 1;
  uint64_t init_val = 1;
  stack_t s;
  stack_init(&s, init_count, sizeof(uint64_ptr_t *), free_uint64_ptr_fn);
  printf("Run a stack_{push, pop} test on noncontiguous uint64_ptr_t "
         "elements\n");
  printf("\tinitial stack count: %lu, "
	 "initial value: %lu, "
	 "number of elements: %lu\n",
	 init_count, init_val, num_elts);
  uint64_ptr_push_pop_helper(&s, init_val, num_elts);
  printf("\tsame stack, initial value: %lu, "
	 "number of elements: %lu\n",
	 init_val, num_elts);
  uint64_ptr_push_pop_helper(&s, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame stack, initial value: %lu, "
	 "number of elements: %lu\n",
	 init_val, num_elts);
  uint64_ptr_push_pop_helper(&s, init_val, num_elts);
  stack_free(&s);
}

void uint64_ptr_push_pop_helper(stack_t *s,
				uint64_t init_val,
				uint64_t num_elts){
  int res = 1;
  uint64_t i;
  uint64_ptr_t **pushed = NULL, **popped = NULL;
  clock_t t_push, t_pop;
  pushed = calloc_perror(num_elts, sizeof(uint64_ptr_t *));
  popped = calloc_perror(num_elts, sizeof(uint64_ptr_t *));
  for (i = 0; i < num_elts; i++){
    pushed[i] = malloc_perror(sizeof(uint64_ptr_t));
    pushed[i]->val = malloc_perror(sizeof(uint64_t));
    *(pushed[i]->val) = init_val + i;
  }
  t_push = clock();
  for (i = 0; i < num_elts; i++){
    stack_push(s, &pushed[i]);
  }
  t_push = clock() - t_push;
  memset(pushed, 0, num_elts * sizeof(uint64_ptr_t *));
  t_pop = clock();
  for (i = 0; i < num_elts; i++){
    stack_pop(s, &popped[i]);
  }
  t_pop = clock() - t_pop;
  res *= (s->num_elts == 0);
  res *= (s->count >= num_elts);
  for (i = 0; i < num_elts; i++){
    res *= (*(popped[i]->val) == num_elts - 1 + init_val - i);
    free_uint64_ptr_fn(&popped[i]);
  }
  printf("\t\tpush time:   %.4f seconds\n", (float)t_push / CLOCKS_PER_SEC);
  printf("\t\tpop time:    %.4f seconds\n", (float)t_pop / CLOCKS_PER_SEC);
  printf("\t\tcorrectness: ");
  print_test_result(res);
  free(pushed);
  free(popped);
  pushed = NULL;
  popped = NULL;
}

void run_uint64_ptr_first_test(){
  int res = 1;
  uint64_t num_elts = 50000000;
  uint64_t init_count = 1;
  uint64_t init_val = 1;
  uint64_t i;
  uint64_ptr_t *pushed = NULL, *popped = NULL;
  stack_t s;
  stack_init(&s, init_count, sizeof(uint64_ptr_t *), free_uint64_ptr_fn);
  printf("Run a stack_first test on noncontiguous uint64_ptr_t elements \n");
  printf("\tinit stack count: %lu\n"
	 "\t#elements:        %lu\n",
	 init_count, num_elts);
  for (i = 0; i < num_elts; i++){
    if (s.num_elts == 0){
      res *= (stack_first(&s) == NULL);
    }else{
      res *= (*(*(uint64_ptr_t **)stack_first(&s))->val == init_val + i - 1);
    }
    pushed = malloc_perror(sizeof(uint64_ptr_t));
    pushed->val = malloc_perror(sizeof(uint64_t));
    *(pushed->val) = init_val + i;
    stack_push(&s, &pushed);
    res *= (*(*(uint64_ptr_t **)stack_first(&s))->val == init_val + i);
    pushed = NULL;
  }
  for (i = 0; i < num_elts; i++){
    res *= (*(*(uint64_ptr_t **)stack_first(&s))->val ==
	    num_elts - 1 + init_val - i);
    stack_pop(&s, &popped);
    if (s.num_elts == 0){
      res *= (stack_first(&s) == NULL);
    }else{
      res *= (*(*(uint64_ptr_t **)stack_first(&s))->val ==
	      num_elts - 2 + init_val - i);
    }
    free_uint64_ptr_fn(&popped);
    popped = NULL; 
  }
  res *= (s.num_elts == 0);
  res *= (s.count >= num_elts);
  printf("\tcorrectness:      ");
  print_test_result(res);
  stack_free(&s);
}

void run_uint64_ptr_free_test(){
  uint64_t num_elts = 50000000;
  uint64_t init_count = 1;
  uint64_t i;
  uint64_ptr_t *pushed = NULL;
  stack_t s;
  clock_t t;
  stack_init(&s, init_count, sizeof(uint64_ptr_t *), free_uint64_ptr_fn);
  printf("Run a stack_free test on noncontiguous uint64_ptr_t elements \n");
  printf("\t# elements:       %lu\n", num_elts);
  for (i = 0; i < num_elts; i++){
    pushed = malloc_perror(sizeof(uint64_ptr_t));
    pushed->val = malloc_perror(sizeof(uint64_t));
    *(pushed->val) = i;
    stack_push(&s, &pushed);
    pushed = NULL;
  }
  t = clock();
  stack_free(&s);
  t = clock() - t;
  printf("\tfree time:        %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Runs a test of a stack of 5 billion char elements.
*/
void run_large_stack_test(){
  unsigned char c = 0xff;
  uint64_t num_elts = 5000000000;
  uint64_t init_count = 1;
  uint64_t i;
  stack_t s;
  clock_t t_push, t_pop;
  stack_init(&s, init_count, sizeof(unsigned char), NULL);
  printf("Run a stack_{push, pop} test on %lu char elements; "
	 "requires sufficient memory \n", num_elts);
  t_push = clock();
  for (i = 0; i < num_elts; i++){
    stack_push(&s, &c);
  }
  t_push = clock() - t_push;
  t_pop = clock();
  for (i = 0; i < num_elts; i++){
    stack_pop(&s, &c);
  }
  t_pop = clock() - t_pop;
  printf("\t\tpush time:   %.4f seconds\n", (float)t_push / CLOCKS_PER_SEC);
  printf("\t\tpop time:    %.4f seconds\n", (float)t_pop / CLOCKS_PER_SEC);
  stack_free(&s);
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_uint64_push_pop_test();
  run_uint64_first_test();
  run_uint64_free_test();
  run_uint64_ptr_push_pop_test();
  run_uint64_ptr_first_test();
  run_uint64_ptr_free_test();
  run_large_stack_test();
  return 0;
}
