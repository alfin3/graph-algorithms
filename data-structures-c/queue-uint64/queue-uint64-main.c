/**
   queue-uint64-main.c

   Examples of a generic dynamically allocated queue with upto 
   (2^64 - 1) / elt_size elements.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the fifo queue form.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include "queue-uint64.h"
#include "utilities-ds.h"

void print_test_result(int result);

/**
   Run tests of a queue of uint64_t elements. A pointer to an element is 
   passed as elt in queue_uint64_push and the uint64_t element is fully 
   copied into the elts array. NULL as free_elt_fn is sufficient to free 
   the queue.
*/
static void uint64_queue_test_helper(queue_uint64_t *q,
				     uint64_t init_val,
				     uint64_t num_elts);

void run_uint64_queue_test(){
  queue_uint64_t q;
  uint64_t num_elts = 100000000;
  uint64_t init_queue_size = 1;
  uint64_t init_val = 1;
  queue_uint64_init(&q, init_queue_size, sizeof(uint64_t), NULL);
  printf("Run a queue_uint64_{push, pop} test on uint64_t elements \n");
  printf("\tinitial queue size: %lu, initial value: %lu, "
	 "number of elements: %lu\n", init_queue_size, init_val, num_elts);
  uint64_queue_test_helper(&q, init_val, num_elts);
  printf("\tsame queue, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_queue_test_helper(&q, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame queue, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_queue_test_helper(&q, init_val, num_elts);
  queue_uint64_free(&q);
}

static void uint64_queue_test_helper(queue_uint64_t *q,
				     uint64_t init_val,
				     uint64_t num_elts){
  uint64_t popped;
  uint64_t cur_val = init_val;
  int result = 1;
  clock_t t;
  t = clock();
  for (uint64_t i = init_val; i < init_val + num_elts; i++){
    queue_uint64_push(q, &i);
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    queue_uint64_pop(q, &popped);
    result *= (cur_val == popped);
    cur_val++;
  }
  t = clock() - t;
  result *= (q->num_elts == 0);
  result *= (q->queue_size >= num_elts);
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness: ");
  print_test_result(result);
}

void run_uint64_queue_free_test(){
  queue_uint64_t q;
  uint64_t num_elts = 100000000;
  uint64_t init_queue_size = 1;
  clock_t t;
  queue_uint64_init(&q, init_queue_size, sizeof(uint64_t), NULL);
  printf("Run a queue_uint64_free test on %lu uint64_t elements\n",
	 num_elts);
  for (uint64_t i = 0; i < num_elts; i++){
    queue_uint64_push(&q, &i);
  }
  t = clock();
  queue_uint64_free(&q);
  t = clock() - t;
  printf("\t\tfree time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a queue of uint64_ptr_t elements. A pointer to a pointer to 
   an uint64_ptr_t element is passed as elt in queue_uint64_push and the 
   pointer to the uint64_ptr_t element is copied into the elts array. A 
   uint64_ptr_t-specific free_elt_fn is necessary to free the queue.
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

static void uint64_ptr_queue_test_helper(queue_uint64_t *q,
					 uint64_t init_val,
					 uint64_t num_elts);

void run_uint64_ptr_queue_test(){
  queue_uint64_t q;
  uint64_t num_elts = 10000000;
  uint64_t init_queue_size = 1;
  uint64_t init_val = 1;
  queue_uint64_init(&q,
		    init_queue_size,
		    sizeof(uint64_ptr_t *),
		    free_uint64_ptr_fn);
  printf("Run a queue_uint64_{push, pop} test on multilayered uint64_ptr_t "
         "elements; time includes allocation and deallocation\n");
  printf("\tinitial queue size: %lu, initial value: %lu, "
	 "number of elements: %lu\n", init_queue_size, init_val, num_elts);
  uint64_ptr_queue_test_helper(&q, init_val, num_elts);
  printf("\tsame queue, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_ptr_queue_test_helper(&q, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame queue, initial value: %lu, number of elements: %lu\n",
	 init_val, num_elts);
  uint64_ptr_queue_test_helper(&q, init_val, num_elts);
  queue_uint64_free(&q);
}

static void uint64_ptr_queue_test_helper(queue_uint64_t *q,
					 uint64_t init_val,
					 uint64_t num_elts){
  uint64_ptr_t *pushed;
  uint64_ptr_t *popped;
  uint64_t cur_val = init_val;
  int result = 1;
  clock_t t;
  t = clock();
  for (uint64_t i = init_val; i < init_val + num_elts; i++){
    pushed = malloc(sizeof(uint64_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(uint64_t));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    queue_uint64_push(q, &pushed);
    pushed = NULL;
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    queue_uint64_pop(q, &popped);
    result *= (cur_val == *(popped->val));
    cur_val++;
    free_uint64_ptr_fn(&popped);
    popped = NULL;
  }
  t = clock() - t;
  result *= (q->num_elts == 0);
  result *= (q->queue_size >= num_elts);
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness: ");
  print_test_result(result);
}

void run_uint64_ptr_queue_free_test(){
  queue_uint64_t q;
  uint64_ptr_t *pushed;
  uint64_t num_elts = 10000000;
  uint64_t init_queue_size = 1;
  clock_t t;
  queue_uint64_init(&q,
		    init_queue_size,
		    sizeof(uint64_ptr_t *),
		    free_uint64_ptr_fn);
  printf("Run a queue_uint64_free test on %lu multilayered "
         "uint64_ptr_t elements \n", num_elts);
  for (uint64_t i = 0; i < num_elts; i++){
    pushed = malloc(sizeof(uint64_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(uint64_t));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    queue_uint64_push(&q, &pushed);
    pushed = NULL;
  }
  t = clock();
  queue_uint64_free(&q);
  t = clock() - t;
  printf("\t\tfree time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run a test of a queue of 5 billion char elements.
*/

void run_large_queue_test(){
  queue_uint64_t q;
  char c = 0;
  uint64_t num_elts = 5000000000;
  uint64_t init_queue_size = 1;
  clock_t t;
  queue_uint64_init(&q, init_queue_size, sizeof(char), NULL);
  printf("Run a queue_uint64_{push, pop} test " 
         "on %lu char elements; requires sufficient memory \n", num_elts);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    queue_uint64_push(&q, &c);
  }
  t = clock() - t;
  printf("\t\tpush time:         %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (uint64_t i = 0; i < num_elts; i++){
    queue_uint64_pop(&q, &c);
  }
  t = clock() - t;
  printf("\t\tpop time:          %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  queue_uint64_free(&q);
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_uint64_queue_test();
  run_uint64_queue_free_test();
  run_uint64_ptr_queue_test();
  run_uint64_ptr_queue_free_test();
  run_large_queue_test();
  return 0;
}
