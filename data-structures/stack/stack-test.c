/**
   stack-test.c

   Tests of a generic stack.

   The following command line arguments can be used to customize tests:
   stack-test
      [0, ulong width) : i s.t. # inserts = 2**i
      [0, ulong width) : i s.t. # inserts = 2**i in uchar stack test
      [0, 1] : on/off push pop first free uint test
      [0, 1] : on/off push pop first free uint_ptr (noncontiguous) test
      [0, 1] : on/off uchar stack test

   usage examples:
   ./stack-test
   ./stack-test 23
   ./stack-test 24 31
   ./stack-test 24 31 0 0 1

   stack-test can be run with any subset of command line arguments in the
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
#include "stack.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-lim.h"

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "stack-test \n"
  "[0, ulong width) : i s.t. # inserts = 2**i\n"
  "[0, ulong width) : i s.t. # inserts = 2**i in uchar stack test\n"
  "[0, 1] : on/off push pop first free uint test\n"
  "[0, 1] : on/off push pop first free uint_ptr (noncontiguous) test\n"
  "[0, 1] : on/off uchar stack test\n";
const int C_ARGC_ULIMIT = 6;
const size_t C_ARGS_DEF[5] = {14, 15, 1, 1, 1};
const size_t C_ULONG_BIT = PRECISION_FROM_ULIMIT((unsigned long)-1);

/* tests */
const unsigned char C_UCHAR_ULIMIT = (unsigned char)-1;
const size_t C_START_VAL = 0; /* <= # inserts */

void print_test_result(int res);

/**
   Run tests of a stack of size_t elements. A pointer to an element is
   passed as elt in stack_push and the size_t element is copied onto the
   stack. NULL as free_elt is sufficient to free the stack.
*/

void uint_push_pop_helper(struct stack *s, size_t start_val, size_t num_ins);

void run_uint_push_pop_test(size_t log_ins){
  size_t num_ins;
  size_t start_val = C_START_VAL;
  struct stack s;
  num_ins = pow_two_perror(log_ins);
  stack_init(&s, sizeof(size_t), NULL);
  printf("Run a stack_{push, pop} test on %lu size_t elements\n",
         TOLU(num_ins));
  printf("\tinitial count: %lu, max count: %lu\n",
         TOLU(s.init_count), TOLU(s.max_count));
  uint_push_pop_helper(&s, start_val, num_ins);
  stack_free(&s);
  stack_init(&s, sizeof(size_t), NULL);
  stack_bound(&s, 1, num_ins);
  printf("\tinitial count: %lu, max count: %lu\n",
         TOLU(s.init_count), TOLU(s.max_count));
  uint_push_pop_helper(&s, start_val, num_ins);
  stack_free(&s);
  stack_init(&s, sizeof(size_t), NULL);
  stack_bound(&s, num_ins, num_ins);
  printf("\tinitial count: %lu, max count: %lu\n",
         TOLU(s.init_count), TOLU(s.max_count));
  uint_push_pop_helper(&s, start_val, num_ins);
  stack_free(&s);
}

void uint_push_pop_helper(struct stack *s, size_t start_val, size_t num_ins){
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
    stack_push(s, &pushed[i]);
  }
  t_push = clock() - t_push;
  t_pop = clock();
  for (i = 0; i < num_ins; i++){
    stack_pop(s, &popped[i]);
  }
  t_pop = clock() - t_pop;
  res *= (s->num_elts == 0);
  res *= (s->count >= num_ins);
  for (i = 0; i < num_ins; i++){
    res *= (popped[i] == num_ins - 1 - i + start_val);
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
  struct stack s;
  num_ins = pow_two_perror(log_ins);
  stack_init(&s, sizeof(size_t), NULL);
  printf("Run a stack_first test on %lu size_t elements\n", TOLU(num_ins));
  for (i = 0; i < num_ins; i++){
    if (s.num_elts == 0){
      res *= (stack_first(&s) == NULL);
    }
    pushed = start_val + i;
    stack_push(&s, &pushed);
    res *= (*(size_t *)stack_first(&s) == pushed);
  }
  for (i = 0; i < num_ins; i++){
    res *= (*(size_t *)stack_first(&s) == num_ins - 1 - i + start_val);
    stack_pop(&s, &popped);
    if (s.num_elts == 0){
      res *= (stack_first(&s) == NULL);
    }
  }
  res *= (s.num_elts == 0);
  res *= (s.count >= num_ins);
  printf("\t\tcorrectness: ");
  print_test_result(res);
  stack_free(&s);
}

void run_uint_free_test(size_t log_ins){
  size_t i;
  size_t num_ins;
  struct stack s;
  clock_t t;
  num_ins = pow_two_perror(log_ins);
  stack_init(&s, sizeof(size_t), NULL);
  printf("Run a stack_free test on %lu size_t elements\n", TOLU(num_ins));
  for (i = 0; i < num_ins; i++){
    stack_push(&s, &i);
  }
  t = clock();
  stack_free(&s);
  t = clock() - t;
  printf("\t\tfree time:   %.4f seconds\n", (double)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a stack of uint_ptr elements. A pointer to a pointer to
   an uint_ptr element is passed as elt in stack_push and the pointer to
   the uint_ptr element is copied onto the stack. A uint_ptr-
   specific free_elt, taking a pointer to a pointer to an element as its
   parameter, is necessary to free the stack.
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

void uint_ptr_push_pop_helper(struct stack *s,
                              size_t start_val,
                              size_t num_ins);

void run_uint_ptr_push_pop_test(size_t log_ins){
  size_t num_ins;
  size_t start_val = C_START_VAL;
  struct stack s;
  num_ins = pow_two_perror(log_ins);
  stack_init(&s, sizeof(struct uint_ptr *), free_uint_ptr);
  printf("Run a stack_{push, pop} test on %lu noncontiguous uint_ptr "
         "elements\n", TOLU(num_ins));
  printf("\tinitial count: %lu, max count: %lu\n",
         TOLU(s.init_count), TOLU(s.max_count));
  uint_ptr_push_pop_helper(&s, start_val, num_ins);
  stack_free(&s);
  stack_init(&s, sizeof(struct uint_ptr *), free_uint_ptr);
  stack_bound(&s, 1, num_ins);
  printf("\tinitial count: %lu, max count: %lu\n",
         TOLU(s.init_count), TOLU(s.max_count));
  uint_ptr_push_pop_helper(&s, start_val, num_ins);
  stack_free(&s);
  stack_init(&s, sizeof(struct uint_ptr *), free_uint_ptr);
  stack_bound(&s, num_ins, num_ins);
  printf("\tinitial count: %lu, max count: %lu\n",
         TOLU(s.init_count), TOLU(s.max_count));
  uint_ptr_push_pop_helper(&s, start_val, num_ins);
  stack_free(&s);
}

void uint_ptr_push_pop_helper(struct stack *s,
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
    stack_push(s, &pushed[i]);
  }
  t_push = clock() - t_push;
  for (i = 0; i < num_ins; i++){
    pushed[i] = NULL;
  }
  t_pop = clock();
  for (i = 0; i < num_ins; i++){
    stack_pop(s, &popped[i]);
  }
  t_pop = clock() - t_pop;
  res *= (s->num_elts == 0);
  res *= (s->count >= num_ins);
  for (i = 0; i < num_ins; i++){
    res *= (*(popped[i]->val) == num_ins - 1 - i + start_val);
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
  struct stack s;
  num_ins = pow_two_perror(log_ins);
  stack_init(&s, sizeof(struct uint_ptr *), free_uint_ptr);
  printf("Run a stack_first test on %lu noncontiguous uint_ptr elements\n",
         TOLU(num_ins));
  for (i = 0; i < num_ins; i++){
    if (s.num_elts == 0){
      res *= (stack_first(&s) == NULL);
    }
    pushed = malloc_perror(1, sizeof(struct uint_ptr));
    pushed->val = malloc_perror(1, sizeof(size_t));
    *(pushed->val) = start_val + i;
    stack_push(&s, &pushed);
    res *= (*(*(struct uint_ptr **)stack_first(&s))->val == start_val + i);
    pushed = NULL;
  }
  for (i = 0; i < num_ins; i++){
    res *= (*(*(struct uint_ptr **)stack_first(&s))->val ==
            num_ins - 1 - i + start_val);
    stack_pop(&s, &popped);
    if (s.num_elts == 0){
      res *= (stack_first(&s) == NULL);
    }
    free_uint_ptr(&popped);
    popped = NULL;
  }
  res *= (s.num_elts == 0);
  res *= (s.count >= num_ins);
  printf("\t\tcorrectness: ");
  print_test_result(res);
  stack_free(&s);
}


void run_uint_ptr_free_test(size_t log_ins){
  size_t i;
  size_t num_ins;
  struct uint_ptr *pushed = NULL;
  struct stack s;
  clock_t t;
  num_ins = pow_two_perror(log_ins);
  stack_init(&s, sizeof(struct uint_ptr *), free_uint_ptr);
  printf("Run a stack_free test on %lu noncontiguous uint_ptr elements\n",
         TOLU(num_ins));
  for (i = 0; i < num_ins; i++){
    pushed = malloc_perror(1, sizeof(struct uint_ptr));
    pushed->val = malloc_perror(1, sizeof(size_t));
    *(pushed->val) = i;
    stack_push(&s, &pushed);
    pushed = NULL;
  }
  t = clock();
  stack_free(&s);
  t = clock() - t;
  printf("\t\tfree time:   %.4f seconds\n", (double)t / CLOCKS_PER_SEC);
}

/**
   Runs a test of a stack of unsigned char elements.
*/
void run_uchar_stack_test(size_t log_ins){
  unsigned char c;
  size_t i;
  size_t num_ins;
  struct stack s;
  clock_t t_push, t_pop;
  num_ins = pow_two_perror(log_ins);
  stack_init(&s, sizeof(unsigned char), NULL);
  printf("Run a stack_{push, pop} test on %lu char elements\n",
         TOLU(num_ins));
  t_push = clock();
  for (i = 0; i < num_ins; i++){
    stack_push(&s, &C_UCHAR_ULIMIT);
  }
  t_push = clock() - t_push;
  t_pop = clock();
  for (i = 0; i < num_ins; i++){
    stack_pop(&s, &c);
  }
  t_pop = clock() - t_pop;
  printf("\t\tpush time:   %.4f seconds\n", (double)t_push / CLOCKS_PER_SEC);
  printf("\t\tpop time:    %.4f seconds\n", (double)t_pop / CLOCKS_PER_SEC);
  stack_free(&s);
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
    run_uchar_stack_test(args[1]);
  }
  free(args);
  args = NULL;
  return 0;
}
