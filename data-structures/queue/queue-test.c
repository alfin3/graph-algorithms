/**
   queue-test.c

   Tests of a generic queue.

   The following command line arguments can be used to customize tests:
   queue-test
      [0, ulong width) : i s.t. # inserts = 2**i
      [0, ulong width) : i s.t. # inserts = 2**i in uchar queue test
      [0, 1] : on/off push pop first free uint test
      [0, 1] : on/off push pop first free uint_ptr (noncontiguous) test
      [0, 1] : on/off uchar queue test

   usage examples:
   ./queue-test
   ./queue-test 23
   ./queue-test 24 31
   ./queue-test 24 31 0 0 1

   queue-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99. The tests require that:
   - size_t and clock_t are convertible to double,
   - size_t can represent values upto 65535 for default values, and upto
     ULONG_MAX (>= 4294967295) otherwise,
   - the widths of the unsigned integral types are less than 2040 and even.

   TODO: add portable size_t printing
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "queue.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-lim.h"

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "queue-test \n"
  "[0, ulong width) : i s.t. # inserts = 2**i\n"
  "[0, ulong width) : i s.t. # inserts = 2**i in uchar queue test\n"
  "[0, 1] : on/off push pop first free uint test\n"
  "[0, 1] : on/off push pop first free uint_ptr (noncontiguous) test\n"
  "[0, 1] : on/off uchar queue test\n";
const int C_ARGC_ULIMIT = 6;
const size_t C_ARGS_DEF[5] = {14, 15, 1, 1, 1};
const size_t C_ULONG_BIT = PRECISION_FROM_ULIMIT((unsigned long)-1);

/* tests */
const unsigned char C_UCHAR_ULIMIT = (unsigned char)-1;
const size_t C_START_VAL = 0; /* <= # inserts */

void print_test_result(int res);

/**
   Run tests of a queue of size_t elements. A pointer to an element is
   passed as elt in queue_push and the size_t element is copied onto the
   queue. NULL as free_elt is sufficient to free the queue.
*/

void uint_push_pop_helper(struct queue *q, size_t start_val, size_t num_ins);

void run_uint_push_pop_test(size_t log_ins){
  size_t num_ins;
  size_t start_val = C_START_VAL;
  struct queue q;
  num_ins = pow_two_perror(log_ins);
  queue_init(&q, sizeof(size_t), NULL);
  printf("Run a queue_{push, pop} test on %lu size_t elements\n",
         TOLU(num_ins));
  printf("\teff initial count: %lu, eff max count: %lu\n",
         TOLU(q.init_count) >> 1, TOLU(q.max_count) >> 1);
  uint_push_pop_helper(&q, start_val, num_ins);
  queue_free(&q);
  queue_init(&q, sizeof(size_t), NULL);
  queue_bound(&q, 1, num_ins);
  printf("\teff initial count: %lu, eff max count: %lu\n",
         TOLU(q.init_count) >> 1, TOLU(q.max_count) >> 1);
  uint_push_pop_helper(&q, start_val, num_ins);
  queue_free(&q);
  queue_init(&q, sizeof(size_t), NULL);
  queue_bound(&q, num_ins, num_ins);
  printf("\teff initial count: %lu, eff max count: %lu\n",
         TOLU(q.init_count) >> 1, TOLU(q.max_count) >> 1);
  uint_push_pop_helper(&q, start_val, num_ins);
  queue_free(&q);
}

void uint_push_pop_helper(struct queue *q, size_t start_val, size_t num_ins){
  int res = 1;
  size_t i;
  size_t *pushed = NULL, *popped = NULL;
  clock_t t_push, t_pop;
  pushed = malloc_perror(num_ins, sizeof(size_t));
  popped = malloc_perror(num_ins, sizeof(size_t));
  for (i = 0; i < num_ins; i++){
    pushed[i] = start_val + i;
  }
  t_push = clock();
  for (i = 0; i < num_ins; i++){
    queue_push(q, &pushed[i]);
  }
  t_push = clock() - t_push;
  t_pop = clock();
  for (i = 0; i < num_ins; i++){
    queue_pop(q, &popped[i]);
  }
  t_pop = clock() - t_pop;
  res *= (q->num_elts == 0);
  res *= (q->count >= num_ins);
  for (i = 0; i < num_ins; i++){
    res *= (popped[i] == start_val + i);
  }
  printf("\t\tpush time:   %.4f seconds\n", (double)t_push / CLOCKS_PER_SEC);
  printf("\t\tpop time:    %.4f seconds\n", (double)t_pop / CLOCKS_PER_SEC);
  printf("\t\tcorrectness: ");
  print_test_result(res);
  free(pushed);
  free(popped);
  pushed = NULL;
  popped = NULL;
}

void run_uint_first_test(size_t log_ins){
  int res = 1;
  size_t i;
  size_t num_ins;
  size_t start_val = C_START_VAL;
  size_t pushed, popped;
  struct queue q;
  num_ins = pow_two_perror(log_ins);
  queue_init(&q, sizeof(size_t), NULL);
  printf("Run a queue_first test on %lu size_t elements\n", TOLU(num_ins));
  for (i = 0; i < num_ins; i++){
    if (q.num_elts == 0){
      res *= (queue_first(&q) == NULL);
    }
    pushed = start_val + i;
    queue_push(&q, &pushed);
    res *= (*(size_t *)queue_first(&q) == start_val);
  }
  for (i = 0; i < num_ins; i++){
    res *= (*(size_t *)queue_first(&q) == start_val + i);
    queue_pop(&q, &popped);
    if (q.num_elts == 0){
      res *= (queue_first(&q) == NULL);
    }
  }
  res *= (q.num_elts == 0);
  res *= (q.count >= num_ins);
  printf("\t\tcorrectness: ");
  print_test_result(res);
  queue_free(&q);
}

void run_uint_free_test(size_t log_ins){
  size_t i;
  size_t num_ins;
  struct queue q;
  clock_t t;
  num_ins = pow_two_perror(log_ins);
  queue_init(&q, sizeof(size_t), NULL);
  printf("Run a queue_free test on %lu size_t elements\n", TOLU(num_ins));
  for (i = 0; i < num_ins; i++){
    queue_push(&q, &i);
  }
  t = clock();
  queue_free(&q);
  t = clock() - t;
  printf("\t\tfree time:   %.4f seconds\n", (double)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a queue of uint_ptr elements. A pointer to a pointer to
   an uint_ptr element is passed as elt in queue_push and the pointer to
   the uint_ptr element is copied onto the queue. A uint_ptr-
   specific free_elt, taking a pointer to a pointer to an element as its
   parameter, is necessary to free the queue.
*/

struct uint_ptr{
  size_t *val;
};

void free_uint_ptr(void *a){
  struct uint_ptr **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
}

void uint_ptr_push_pop_helper(struct queue *q,
                              size_t start_val,
                              size_t num_ins);

void run_uint_ptr_push_pop_test(size_t log_ins){
  size_t num_ins;
  size_t start_val = C_START_VAL;
  struct queue q;
  num_ins = pow_two_perror(log_ins);
  queue_init(&q, sizeof(struct uint_ptr *), free_uint_ptr);
  printf("Run a queue_{push, pop} test on %lu noncontiguous uint_ptr "
         "elements\n", TOLU(num_ins));
  printf("\teff initial count: %lu, eff max count: %lu\n",
         TOLU(q.init_count) >> 1, TOLU(q.max_count) >> 1);
  uint_ptr_push_pop_helper(&q, start_val, num_ins);
  queue_free(&q);
  queue_init(&q, sizeof(struct uint_ptr *), free_uint_ptr);
  queue_bound(&q, 1, num_ins);
  printf("\teff initial count: %lu, eff max count: %lu\n",
         TOLU(q.init_count) >> 1, TOLU(q.max_count) >> 1);
  uint_ptr_push_pop_helper(&q, start_val, num_ins);
  queue_free(&q);
  queue_init(&q, sizeof(struct uint_ptr *), free_uint_ptr);
  queue_bound(&q, num_ins, num_ins);
  printf("\teff initial count: %lu, eff max count: %lu\n",
         TOLU(q.init_count) >> 1, TOLU(q.max_count) >> 1);
  uint_ptr_push_pop_helper(&q, start_val, num_ins);
  queue_free(&q);
}

void uint_ptr_push_pop_helper(struct queue *q,
                              size_t start_val,
                              size_t num_ins){
  int res = 1;
  size_t i;
  struct uint_ptr **pushed = NULL, **popped = NULL;
  clock_t t_push, t_pop;
  pushed = calloc_perror(num_ins, sizeof(struct uint_ptr *));
  popped = calloc_perror(num_ins, sizeof(struct uint_ptr *));
  for (i = 0; i < num_ins; i++){
    pushed[i] = malloc_perror(1, sizeof(struct uint_ptr));
    pushed[i]->val = malloc_perror(1, sizeof(size_t));
    *(pushed[i]->val) = start_val + i;
  }
  t_push = clock();
  for (i = 0; i < num_ins; i++){
    queue_push(q, &pushed[i]);
  }
  t_push = clock() - t_push;
  for (i = 0; i < num_ins; i++){
    pushed[i] = NULL;
  }
  t_pop = clock();
  for (i = 0; i < num_ins; i++){
    queue_pop(q, &popped[i]);
  }
  t_pop = clock() - t_pop;
  res *= (q->num_elts == 0);
  res *= (q->count >= num_ins);
  for (i = 0; i < num_ins; i++){
    res *= (*(popped[i]->val) == start_val + i);
    free_uint_ptr(&popped[i]);
  }
  printf("\t\tpush time:   %.4f seconds\n", (double)t_push / CLOCKS_PER_SEC);
  printf("\t\tpop time:    %.4f seconds\n", (double)t_pop / CLOCKS_PER_SEC);
  printf("\t\tcorrectness: ");
  print_test_result(res);
  free(pushed);
  free(popped);
  pushed = NULL;
  popped = NULL;
}

void run_uint_ptr_first_test(size_t log_ins){
  int res = 1;
  size_t i;
  size_t num_ins;
  size_t start_val = C_START_VAL;
  struct uint_ptr *pushed = NULL, *popped = NULL;
  struct queue q;
  num_ins = pow_two_perror(log_ins);
  queue_init(&q, sizeof(struct uint_ptr *), free_uint_ptr);
  printf("Run a queue_first test on %lu noncontiguous uint_ptr elements\n",
         TOLU(num_ins));
  for (i = 0; i < num_ins; i++){
    if (q.num_elts == 0){
      res *= (queue_first(&q) == NULL);
    }
    pushed = malloc_perror(1, sizeof(struct uint_ptr));
    pushed->val = malloc_perror(1, sizeof(size_t));
    *(pushed->val) = start_val + i;
    queue_push(&q, &pushed);
    res *= (*(*(struct uint_ptr **)queue_first(&q))->val == start_val);
    pushed = NULL;
  }
  for (i = 0; i < num_ins; i++){
    res *= (*(*(struct uint_ptr **)queue_first(&q))->val == start_val + i);
    queue_pop(&q, &popped);
    if (q.num_elts == 0){
      res *= (queue_first(&q) == NULL);
    }
    free_uint_ptr(&popped);
    popped = NULL;
  }
  res *= (q.num_elts == 0);
  res *= (q.count >= num_ins);
  printf("\t\tcorrectness: ");
  print_test_result(res);
  queue_free(&q);
}

void run_uint_ptr_free_test(size_t log_ins){
  size_t i;
  size_t num_ins;
  struct uint_ptr *pushed = NULL;
  struct queue q;
  clock_t t;
  num_ins = pow_two_perror(log_ins);
  queue_init(&q, sizeof(struct uint_ptr *), free_uint_ptr);
  printf("Run a queue_free test on %lu noncontiguous uint_ptr elements\n",
         TOLU(num_ins));
  for (i = 0; i < num_ins; i++){
    pushed = malloc_perror(1, sizeof(struct uint_ptr));
    pushed->val = malloc_perror(1, sizeof(size_t));
    *(pushed->val) = i;
    queue_push(&q, &pushed);
    pushed = NULL;
  }
  t = clock();
  queue_free(&q);
  t = clock() - t;
  printf("\t\tfree time:   %.4f seconds\n", (double)t / CLOCKS_PER_SEC);
}

/**
   Runs a test of a queue of unsigned char elements.
*/
void run_uchar_queue_test(size_t log_ins){
  unsigned char c;
  size_t i;
  size_t num_ins;
  struct queue q;
  clock_t t_push, t_pop;
  num_ins = pow_two_perror(log_ins);
  queue_init(&q, sizeof(unsigned char), NULL);
  printf("Run a queue_{push, pop} test on %lu char elements\n",
         TOLU(num_ins));
  t_push = clock();
  for (i = 0; i < num_ins; i++){
    queue_push(&q, &C_UCHAR_ULIMIT);
  }
  t_push = clock() - t_push;
  t_pop = clock();
  for (i = 0; i < num_ins; i++){
    queue_pop(&q, &c);
  }
  t_pop = clock() - t_pop;
  printf("\t\tpush time:   %.4f seconds\n", (double)t_push / CLOCKS_PER_SEC);
  printf("\t\tpop time:    %.4f seconds\n", (double)t_pop / CLOCKS_PER_SEC);
  queue_free(&q);
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(int argc, char *argv[]){
  int i;
  size_t *args = NULL;
  if (argc > C_ARGC_ULIMIT){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_ULIMIT - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_ULIMIT - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] > C_ULONG_BIT - 1 ||
      args[1] > C_ULONG_BIT - 1 ||
      args[2] > 1 ||
      args[3] > 1 ||
      args[4] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[2]){
    run_uint_push_pop_test(args[0]);
    run_uint_first_test(args[0]);
    run_uint_free_test(args[0]);
  }
  if (args[3]){
    run_uint_ptr_push_pop_test(args[0]);
    run_uint_ptr_first_test(args[0]);
    run_uint_ptr_free_test(args[0]);
  }
  if (args[4]){
    run_uchar_queue_test(args[1]);
  }
  free(args);
  args = NULL;
  return 0;
}
