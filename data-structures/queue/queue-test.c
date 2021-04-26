/**
   queue-test.c

   Tests of a generic dynamically allocated queue.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

void print_test_result(int res);

/**
   Run tests of a queue of size_t elements. A pointer to an element is 
   passed as elt in queue_push and the size_t element is fully 
   copied onto the queue. NULL as free_elt is sufficient to free 
   the queue.
*/

void uint_push_pop_helper(queue_t *q,
			  size_t init_val,
			  size_t num_elts);

void run_uint_push_pop_test(){
  size_t num_elts = 50000000;
  size_t init_count = 1;
  size_t init_val = 1;
  queue_t q;
  queue_init(&q, init_count, sizeof(size_t), NULL);
  printf("Run a queue_{push, pop} test on size_t elements \n");
  printf("\tinitial queue count: %lu, "
	 "initial value: %lu, "
	 "number of elements: %lu\n",
	 TOLU(init_count), TOLU(init_val), TOLU(num_elts));
  uint_push_pop_helper(&q, init_val, num_elts);
  printf("\tsame queue, initial value: %lu, number of elements: %lu\n",
	 TOLU(init_val), TOLU(num_elts));
  uint_push_pop_helper(&q, init_val, num_elts);
  init_val = num_elts + 1;
  printf("\tsame queue, initial value: %lu, number of elements: %lu\n",
	 TOLU(init_val), TOLU(num_elts));
  uint_push_pop_helper(&q, init_val, num_elts);
  queue_free(&q);
}

void uint_push_pop_helper(queue_t *q,
			  size_t init_val,
			  size_t num_elts){
  int res = 1;
  size_t i;
  size_t *pushed = NULL, *popped = NULL;
  clock_t t_push, t_pop;
  pushed = malloc_perror(num_elts, sizeof(size_t));
  popped = malloc_perror(num_elts, sizeof(size_t));
  for (i = 0; i < num_elts; i++){
    pushed[i] = init_val + i;
  }
  t_push = clock();
  for (i = 0; i < num_elts; i++){
    queue_push(q, &pushed[i]);
  }
  t_push = clock() - t_push;
  t_pop = clock();
  for (i = 0; i < num_elts; i++){
    queue_pop(q, &popped[i]);
  }
  t_pop = clock() - t_pop;
  res *= (q->num_elts == 0);
  res *= (q->count >= num_elts);
  for (i = 0; i < num_elts; i++){
    res *= (popped[i] == init_val + i);
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

void run_uint_first_test(){
  int res = 1;
  size_t num_elts = 50000000;
  size_t init_count = 1;
  size_t init_val = 1;
  size_t pushed, popped;
  size_t i;
  queue_t q;
  queue_init(&q, init_count, sizeof(size_t), NULL);
  printf("Run a queue_first test on size_t elements \n");
  printf("\tinit queue count: %lu\n"
	 "\t#elements:        %lu\n",
	 TOLU(init_count), TOLU(num_elts));
  for (i = 0; i < num_elts; i++){
    if (q.num_elts == 0){
      res *= (queue_first(&q) == NULL);
    }else{
      res *= (*(size_t *)queue_first(&q) == init_val);
    }
    pushed = init_val + i;
    queue_push(&q, &pushed);
    res *= (*(size_t *)queue_first(&q) == init_val);
  }
  for (i = 0; i < num_elts; i++){
    res *= (*(size_t *)queue_first(&q) == init_val + i);
    queue_pop(&q, &popped);
    if (q.num_elts == 0){
      res *= (queue_first(&q) == NULL);
    }else{
      res *= (*(size_t *)queue_first(&q) == init_val + i + 1);
    }
  }
  res *= (q.num_elts == 0);
  res *= (q.count >= num_elts);
  printf("\tcorrectness:      ");
  print_test_result(res);
  queue_free(&q);
}

void run_uint_free_test(){
  size_t num_elts = 50000000;
  size_t init_count = 1;
  size_t i;
  queue_t q;
  clock_t t;
  queue_init(&q, init_count, sizeof(size_t), NULL);
  printf("Run a queue_free test on size_t elements\n");
  printf("\t# elements:       %lu\n", TOLU(num_elts));
  for (i = 0; i < num_elts; i++){
    queue_push(&q, &i);
  }
  t = clock();
  queue_free(&q);
  t = clock() - t;
  printf("\tfree time:        %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a queue of uint_ptr_t elements. A pointer to a pointer to 
   an uint_ptr_t element is passed as elt in queue_push and the pointer to
   the uint_ptr_t element is copied onto the queue. A uint_ptr_t-
   specific free_elt, taking a pointer to a pointer to an element as its
   parameter, is necessary to free the queue.
*/

typedef struct{
  size_t *val;
} uint_ptr_t;

void free_uint_ptr(void *a){
  uint_ptr_t **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

void uint_ptr_push_pop_helper(queue_t *q,
				size_t init_val,
				size_t num_elts);

void run_uint_ptr_push_pop_test(){
  size_t num_elts = 50000000;
  size_t init_count = 1;
  size_t init_val = 1;
  queue_t q;
  queue_init(&q, init_count, sizeof(uint_ptr_t *), free_uint_ptr);
  printf("Run a queue_{push, pop} test on noncontiguous uint_ptr_t "
         "elements\n");
  printf("\tinitial queue count: %lu, "
	 "initial value: %lu, "
	 "number of elements: %lu\n",
	 TOLU(init_count), TOLU(init_val), TOLU(num_elts));
  uint_ptr_push_pop_helper(&q, init_val, num_elts);
  printf("\tsame queue, initial value: %lu, "
	 "number of elements: %lu\n",
	 TOLU(init_val), TOLU(num_elts));
  uint_ptr_push_pop_helper(&q, TOLU(init_val), TOLU(num_elts));
  init_val = num_elts + 1;
  printf("\tsame queue, initial value: %lu, "
	 "number of elements: %lu\n",
	 TOLU(init_val), TOLU(num_elts));
  uint_ptr_push_pop_helper(&q, init_val, num_elts);
  queue_free(&q);
}

void uint_ptr_push_pop_helper(queue_t *q,
				size_t init_val,
				size_t num_elts){
  int res = 1;
  size_t i;
  uint_ptr_t **pushed = NULL, **popped = NULL;
  clock_t t_push, t_pop;
  pushed = calloc_perror(num_elts, sizeof(uint_ptr_t *));
  popped = calloc_perror(num_elts, sizeof(uint_ptr_t *));
  for (i = 0; i < num_elts; i++){
    pushed[i] = malloc_perror(1, sizeof(uint_ptr_t));
    pushed[i]->val = malloc_perror(1, sizeof(size_t));
    *(pushed[i]->val) = init_val + i;
  }
  t_push = clock();
  for (i = 0; i < num_elts; i++){
    queue_push(q, &pushed[i]);
  }
  t_push = clock() - t_push;
  memset(pushed, 0, num_elts * sizeof(uint_ptr_t *));
  t_pop = clock();
  for (i = 0; i < num_elts; i++){
    queue_pop(q, &popped[i]);
  }
  t_pop = clock() - t_pop;
  res *= (q->num_elts == 0);
  res *= (q->count >= num_elts);
  for (i = 0; i < num_elts; i++){
    res *= (*(popped[i]->val) == init_val + i);
    free_uint_ptr(&popped[i]);
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

void run_uint_ptr_first_test(){
  int res = 1;
  size_t num_elts = 50000000;
  size_t init_count = 1;
  size_t init_val = 1;
  size_t i;
  uint_ptr_t *pushed = NULL, *popped = NULL;
  queue_t q;
  queue_init(&q, init_count, sizeof(uint_ptr_t *), free_uint_ptr);
  printf("Run a queue_first test on noncontiguous uint_ptr_t elements \n");
  printf("\tinit queue count: %lu\n"
	 "\t#elements:        %lu\n",
	 TOLU(init_count), TOLU(num_elts));
  for (i = 0; i < num_elts; i++){
    if (q.num_elts == 0){
      res *= (queue_first(&q) == NULL);
    }else{
      res *= (*(*(uint_ptr_t **)queue_first(&q))->val == init_val);
    }
    pushed = malloc_perror(1, sizeof(uint_ptr_t));
    pushed->val = malloc_perror(1, sizeof(size_t));
    *(pushed->val) = init_val + i;
    queue_push(&q, &pushed);
    res *= (*(*(uint_ptr_t **)queue_first(&q))->val == init_val);
    pushed = NULL;
  }
  for (i = 0; i < num_elts; i++){
    res *= (*(*(uint_ptr_t **)queue_first(&q))->val == init_val + i);
    queue_pop(&q, &popped);
    if (q.num_elts == 0){
      res *= (queue_first(&q) == NULL);
    }else{
      res *= (*(*(uint_ptr_t **)queue_first(&q))->val == init_val + i + 1);
    }
    free_uint_ptr(&popped);
    popped = NULL; 
  }
  res *= (q.num_elts == 0);
  res *= (q.count >= num_elts);
  printf("\tcorrectness:      ");
  print_test_result(res);
  queue_free(&q);
}

void run_uint_ptr_free_test(){
  size_t num_elts = 50000000;
  size_t init_count = 1;
  size_t i;
  uint_ptr_t *pushed = NULL;
  queue_t q;
  clock_t t;
  queue_init(&q, init_count, sizeof(uint_ptr_t *), free_uint_ptr);
  printf("Run a queue_free test on noncontiguous uint_ptr_t elements \n");
  printf("\t# elements:       %lu\n", TOLU(num_elts));
  for (i = 0; i < num_elts; i++){
    pushed = malloc_perror(1, sizeof(uint_ptr_t));
    pushed->val = malloc_perror(1, sizeof(size_t));
    *(pushed->val) = i;
    queue_push(&q, &pushed);
    pushed = NULL;
  }
  t = clock();
  queue_free(&q);
  t = clock() - t;
  printf("\tfree time:        %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Runs a test of a queue of 5 billion char elements.
*/
void run_large_queue_test(){
  unsigned char c = 0xffu;
  size_t num_elts = 5000000000u; /*wraps around for now*/
  size_t init_count = 1;
  size_t i;
  queue_t q;
  clock_t t_push, t_pop;
  queue_init(&q, init_count, sizeof(unsigned char), NULL);
  printf("Run a queue_{push, pop} test on %lu char elements; "
	 "requires sufficient memory \n", TOLU(num_elts));
  t_push = clock();
  for (i = 0; i < num_elts; i++){
    queue_push(&q, &c);
  }
  t_push = clock() - t_push;
  t_pop = clock();
  for (i = 0; i < num_elts; i++){
    queue_pop(&q, &c);
  }
  t_pop = clock() - t_pop;
  printf("\t\tpush time:   %.4f seconds\n", (float)t_push / CLOCKS_PER_SEC);
  printf("\t\tpop time:    %.4f seconds\n", (float)t_pop / CLOCKS_PER_SEC);
  queue_free(&q);
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_uint_push_pop_test();
  run_uint_first_test();
  run_uint_free_test();
  run_uint_ptr_push_pop_test();
  run_uint_ptr_first_test();
  run_uint_ptr_free_test();
  run_large_queue_test();
  return 0;
}
