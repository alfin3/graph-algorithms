/**
   queue-main.c

   Tests of a generic dynamically allocated queue.

   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "queue.h"
#include "utilities-mem.h"

void print_test_result(int result);

/**
   Run tests of a queue of uint64_t elements. A pointer to an element is 
   passed as elt in queue_push and the uint64_t element is fully 
   copied onto the queue. NULL as free_elt is sufficient to free 
   the queue.
*/

void uint64_test_helper(queue_t *s,
			uint64_t init_val,
			uint64_t num_elts);

void run_uint64_test(){
  queue_t s;
  uint64_t num_elts = 100000000;
  uint64_t init_count = 1;
  uint64_t init_val = 1; //>= 1
  queue_init(&s, init_count, sizeof(uint64_t), NULL);
  printf("Run a queue_{push, pop} test on uint64_t elements \n");
  printf("\tinitial queue count: %lu, "
	 "initial value: %lu, "
	 "number of elements: %lu\n",
	 init_count, init_val, num_elts);
  uint64_test_helper(&s, init_val, num_elts);
  printf("\tsame queue, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_test_helper(&s, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame queue, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_test_helper(&s, init_val, num_elts);
  queue_free(&s);
}

void uint64_test_helper(queue_t *s,
			uint64_t init_val,
			uint64_t num_elts){
  uint64_t popped;
  uint64_t cur_val = init_val;
  int result = 1;
  clock_t t;
  t = clock();
  for (uint64_t i = init_val; i < init_val + num_elts; i++){
    queue_push(s, &i);
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    queue_pop(s, &popped);
    result *= (cur_val == popped);
    cur_val++;
  }
  t = clock() - t;
  result *= (s->num_elts == 0);
  result *= (s->count >= num_elts);
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness: ");
  print_test_result(result);
}

void run_uint64_free_test(){
  queue_t s;
  uint64_t num_elts = 100000000;
  uint64_t init_count = 1;
  clock_t t;
  queue_init(&s, init_count, sizeof(uint64_t), NULL);
  printf("Run a queue_free test on %lu uint64_t elements\n", num_elts);
  for (uint64_t i = 0; i < num_elts; i++){
    queue_push(&s, &i);
  }
  t = clock();
  queue_free(&s);
  t = clock() - t;
  printf("\t\tfree time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a queue of uint64_ptr_t elements. A pointer to a pointer to 
   an uint64_ptr_t element is passed as elt in queue_push and the pointer to
   the uint64_ptr_t element is copied onto the queue. A uint64_ptr_t-
   specific free_elt, taking a pointer to a pointer to an element as its only
   parameter, is necessary to free the queue.
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

void uint64_ptr_test_helper(queue_t *s,
			    uint64_t init_val,
			    uint64_t num_elts);

void run_uint64_ptr_test(){
  queue_t s;
  uint64_t num_elts = 10000000;
  uint64_t init_count = 1;
  uint64_t init_val = 1; //>= 1
  queue_init(&s, init_count, sizeof(uint64_ptr_t *), free_uint64_ptr_fn);
  printf("Run a queue_{push, pop} test on noncontiguous uint64_ptr_t "
         "elements; time includes allocation and deallocation\n");
  printf("\tinitial queue count: %lu, "
	 "initial value: %lu, "
	 "number of elements: %lu\n",
	 init_count, init_val, num_elts);
  uint64_ptr_test_helper(&s, init_val, num_elts);
  printf("\tsame queue, initial value: %lu, "
	 "number of elements: %lu\n",
	 init_val, num_elts);
  uint64_ptr_test_helper(&s, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame queue, initial value: %lu, "
	 "number of elements: %lu\n",
	 init_val, num_elts);
  uint64_ptr_test_helper(&s, init_val, num_elts);
  queue_free(&s);
}

void uint64_ptr_test_helper(queue_t *s,
			    uint64_t init_val,
			    uint64_t num_elts){
  uint64_ptr_t *pushed;
  uint64_ptr_t *popped;
  uint64_t cur_val = init_val;
  int result = 1;
  clock_t t;
  t = clock();
  for (uint64_t i = init_val; i < init_val + num_elts; i++){
    pushed = malloc_perror(sizeof(uint64_ptr_t));
    pushed->val = malloc_perror(sizeof(uint64_t));
    *(pushed->val) = i;
    queue_push(s, &pushed);
    pushed = NULL;
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    queue_pop(s, &popped);
    result *= (cur_val == *(popped->val));
    cur_val++;
    free_uint64_ptr_fn(&popped);
    popped = NULL;
  }
  t = clock() - t;
  result *= (s->num_elts == 0);
  result *= (s->count >= num_elts);
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness: ");
  print_test_result(result);
}

void run_uint64_ptr_free_test(){
  queue_t s;
  uint64_ptr_t *pushed;
  uint64_t num_elts = 10000000;
  uint64_t init_count = 1;
  clock_t t;
  queue_init(&s, init_count, sizeof(uint64_ptr_t *), free_uint64_ptr_fn);
  printf("Run a queue_free test on %lu multilayered uint64_ptr_t "
	 "elements \n", num_elts);
  for (uint64_t i = 0; i < num_elts; i++){
    pushed = malloc_perror(sizeof(uint64_ptr_t));
    pushed->val = malloc_perror(sizeof(uint64_t));
    *(pushed->val) = i;
    queue_push(&s, &pushed);
    pushed = NULL;
  }
  t = clock();
  queue_free(&s);
  t = clock() - t;
  printf("\t\tfree time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Runs a test of a queue of 5 billion char elements.
*/
void run_large_queue_test(){
  queue_t s;
  char c = 0;
  uint64_t num_elts = 5000000000;
  uint64_t init_count = 1;
  clock_t t;
  queue_init(&s, init_count, sizeof(char), NULL);
  printf("Run a queue_{push, pop} test on %lu char elements; "
	 "requires sufficient memory \n", num_elts);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    queue_push(&s, &c);
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    queue_pop(&s, &c);
  }
  t = clock() - t;
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  queue_free(&s);
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_uint64_test();
  run_uint64_free_test();
  run_uint64_ptr_test();
  run_uint64_ptr_free_test();
  run_large_queue_test();
  return 0;
}
