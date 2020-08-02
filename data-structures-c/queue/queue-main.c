/**
   queue-main.c

   Examples of a generic dynamically allocated fifo queue.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the fifo queue form.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "queue.h"

void print_test_result(int result);

/**
   Run tests of a queue of integer elements. A pointer to an integer is 
   passed as elt in queue_push and the integer element is fully copied into 
   the elts array. NULL as free_int_fn is sufficient to free the queue.
*/
static void int_queue_test_helper(queue_t *q,
				  int init_val,
				  int num_elts);

void run_int_queue_test(){
  queue_t q;
  int num_elts = 100000000;
  int init_queue_size = 1;
  int init_val;
  queue_init(&q, init_queue_size, sizeof(int), NULL);
  init_val = 0;
  printf("Run queue tests on int elements \n");
  printf("\tinitial queue size: %d, initial value: %d, "
	 "number of elements: %d\n", init_queue_size, init_val, num_elts);
  int_queue_test_helper(&q, init_val, num_elts);
  printf("\tsame queue, initial value: %d, number of elements: %d\n",
	 init_val, num_elts);
  int_queue_test_helper(&q, init_val, num_elts);
  init_val = num_elts;
  printf("\tsame queue, initial value: %d, number of elements: %d\n",
	 init_val, num_elts);
  int_queue_test_helper(&q, init_val, num_elts);
  queue_free(&q);
}

static void int_queue_test_helper(queue_t *q,
				  int init_val,
				  int num_elts){
  int popped;
  int cur_val = init_val;
  int result = 1;
  clock_t t;
  t = clock();
  for (int i = init_val; i < init_val + num_elts; i++){
    queue_push(q, &i);
  }
  t = clock() - t;
  printf("\t\tpush time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (int i = 0; i < num_elts; i++){
    queue_pop(q, &popped);
    result *= (cur_val == popped);
    cur_val++;
  }
  t = clock() - t;
  result *= (q->num_elts == 0);
  result *= (q->queue_size >= num_elts);
  printf("\t\tpop time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness --> ");
  print_test_result(result);
}

void run_int_queue_free_test(){
  queue_t q;
  int num_elts = 100000000;
  int init_queue_size = 1;
  clock_t t;
  queue_init(&q, init_queue_size, sizeof(int), NULL);
  printf("Run a queue_free test on %d int elements\n", num_elts);
  for (int i = 0; i < num_elts; i++){
    queue_push(&q, &i);
  }
  t = clock();
  queue_free(&q);
  t = clock() - t;
  printf("\t\tfree time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a queue of int_ptr_t elements. A pointer to a pointer to an
   int_ptr_t element is passed as elt in queue_push and the pointer to the
   int_ptr_t element is copied into the elts array. A int_ptr_t-specific 
   free_int_fn is necessary to free the queue.
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

static void int_ptr_t_queue_test_helper(queue_t *q,
					int init_val,
					int num_elts);

void run_int_ptr_t_queue_test(){
  queue_t q;
  int num_elts = 10000000;
  int init_queue_size = 1;
  int init_val;
  queue_init(&q, init_queue_size, sizeof(int_ptr_t *), free_int_ptr_t_fn);
  init_val = 0;
  printf("Run queue tests on int_ptr_t elements (multilayered objects)\n");
  printf("\tinitial queue size: %d, initial value: %d, "
	 "number of elements: %d\n", init_queue_size, init_val, num_elts);
  int_ptr_t_queue_test_helper(&q, init_val, num_elts);
  printf("\tsame queue, initial value: %d, number of elements: %d\n",
	 init_val, num_elts);
  int_ptr_t_queue_test_helper(&q, init_val, num_elts);
  init_val = num_elts;
  printf("\tsame queue, initial value: %d, number of elements: %d\n",
	 init_val, num_elts);
  int_ptr_t_queue_test_helper(&q, init_val, num_elts);
  queue_free(&q);
}

static void int_ptr_t_queue_test_helper(queue_t *q,
					int init_val,
					int num_elts){
  int_ptr_t *pushed;
  int_ptr_t *popped;
  int cur_val = init_val;
  int result = 1;
  clock_t t;
  t = clock();
  for (int i = init_val; i < init_val + num_elts; i++){
    pushed = malloc(sizeof(int_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(int));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    queue_push(q, &pushed);
    pushed = NULL;
  }
  t = clock() - t;
  printf("\t\tpush time: %.4f seconds (incl. element allocation)\n",
	 (float)t / CLOCKS_PER_SEC);
  t = clock();
  for (int i = 0; i < num_elts; i++){
    queue_pop(q, &popped);
    result *= (cur_val == *(popped->val));
    cur_val++;
    free_int_ptr_t_fn(&popped);
    popped = NULL;
  }
  t = clock() - t;
  result *= (q->num_elts == 0);
  result *= (q->queue_size >= num_elts);
  printf("\t\tpop time: %.4f seconds (incl. element deallocation)\n",
	 (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness --> ");
  print_test_result(result);
}

void run_int_ptr_t_queue_free_test(){
  queue_t q;
  int_ptr_t *pushed;
  int num_elts = 10000000;
  int init_queue_size = 1;
  clock_t t;
  queue_init(&q, init_queue_size, sizeof(int_ptr_t *), free_int_ptr_t_fn);
  printf("Run a queue_free test on %d int_ptr_t elements "
	 "(multilayered objects)\n", num_elts);
  for (int i = 0; i < num_elts; i++){
    pushed = malloc(sizeof(int_ptr_t));
    assert(pushed != NULL);
    pushed->val = malloc(sizeof(int));
    assert(pushed->val != NULL);
    *(pushed->val) = i;
    queue_push(&q, &pushed);
    pushed = NULL;
  }
  t = clock();
  queue_free(&q);
  t = clock() - t;
  printf("\t\tfree time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_int_queue_test();
  run_int_ptr_t_queue_test();
  run_int_queue_free_test();
  run_int_ptr_t_queue_free_test();
  return 0;
}
