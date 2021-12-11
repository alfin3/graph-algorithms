/**
   dll-test.c

   Tests of a doubly linked list with cache-efficient allocation of
   nodes with two type-generic data blocks. The list is in a circular
   representation.

   The following command line arguments can be used to customize tests:
   dll-test
      [0, bit width of int - 2) : i s.t. # inserts = 2**i
      [0, 1] : on/off prepend append free int test
      [0, 1] : on/off prepend append free int_ptr (noncontiguous) test
      [0, 1] : on/off corner cases test

   usage examples:
   ./dll-test
   ./dll-test 23
   ./dll-test 24 1 0 0

   dll-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99 with the only requirement that width of size_t is even
   and less than 2040, and the precision of int is less than 2040. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-lim.h"

/* input handling */
const char *C_USAGE =
  "dll-test\n"
  "[0, precision of int - 2) : i s.t. # inserts = 2**i\n"
  "[0, 1] : on/off prepend append free int test\n"
  "[0, 1] : on/off prepend append free int_ptr (noncontiguous) test\n"
  "[0, 1] : on/off corner cases test\n";
const int C_ARGC_ULIMIT = 5;
const size_t C_ARGS_DEF[4] = {13, 1, 1, 1};
const size_t C_INT_BIT = PRECISION_FROM_ULIMIT(INT_MAX);

/* tests */
const int C_START_VAL = 0;

void prepend_append_free(const struct dll *ll_prep,
			 const struct dll *ll_app,
			 struct dll_node **head_prep,
			 struct dll_node **head_app,
			 int start_val,
			 int num_ins,
			 size_t key_size,
			 size_t elt_size,
			 void (*new_key)(void *, int),
			 void (*new_elt)(void *, int),
			 int (*val_key)(const void *),
			 int (*val_elt)(const void *),
			 void (*free_key)(void *),
			 void (*free_elt)(void *));
void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);

/**
   Run tests of a doubly linked list of integer keys and integer elements. 
   A pointer to an integer is passed as key parameter value in prepend and
   append ops, and the integer is copied into a node as a key_size block.
   A pointer to an integer is passed as elt parameter value in prepend_new
   and append_new ops, and the integer is copied into a node as an elt_size
   block. NULL as free_key and free_elt is sufficient to delete a node.
*/

void new_int(void *a, int val){
  *(int *)a = val;
}

int val_int(const void *a){
  return *(int *)a;
}

int cmp_int(const void *a, const void *b){
  if (*(int *)a == *(int *)b){
    return 0;
  }else{
    return 1;
  }
}

void run_prepend_append_free_int_test(int log_ins){
  int num_ins;
  int start_val = C_START_VAL;
  size_t key_size = sizeof(int);
  size_t elt_size = sizeof(int);
  struct dll ll_prep, ll_app;
  struct dll_node *head_prep, *head_app; /* uninitialized pointers for test */
  num_ins = pow_two_perror(log_ins);
  dll_init(&ll_prep, &head_prep, key_size);
  dll_init(&ll_app, &head_app, key_size);
  dll_align_elt(&ll_prep, sizeof(int));
  dll_align_elt(&ll_app, sizeof(int));
  printf("Run prepend, append, free test on int keys and int elements\n");
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&ll_prep,
		      &ll_app,
		      &head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      key_size,
		      elt_size,
		      new_int,
		      new_int,
		      val_int,
		      val_int,
		      NULL,
		      NULL);
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d (repeat test)\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&ll_prep,
		      &ll_app,
		      &head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      key_size,
		      elt_size,
		      new_int,
		      new_int,
		      val_int,
		      val_int,
		      NULL,
		      NULL);
  start_val += num_ins;
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&ll_prep,
		      &ll_app,
		      &head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      key_size,
		      elt_size,
		      new_int,
		      new_int,
		      val_int,
		      val_int,
		      NULL,
		      NULL);
}

/**
   Run tests of a doubly linked list of non-contiguous int_ptr keys and
   int_ptr elements. A pointer to a pointer to an int_ptr key is passed
   as key parameter value in prepend_new and append_new ops and the pointer
   to the int_ptr key is copied into a node as a key_size block. A
   int_ptr-specific free_key is necessary to delete a node. The same
   applies to an element.
*/

struct int_ptr{
  int *val;
};

void new_int_ptr(void *a, int val){
  struct int_ptr **s = a;
  (*s) = malloc_perror(1, sizeof(struct int_ptr));
  (*s)->val = malloc_perror(1, sizeof(int));
  *((*s)->val) = val;
}

int val_int_ptr(const void *a){
  return *((*(struct int_ptr **)a)->val);
}

int cmp_int_ptr(const void *a, const void *b){
  if (*((*(struct int_ptr **)a)->val) > *((*(struct int_ptr **)b)->val)){
    return 1;
  }else if (*((*(struct int_ptr **)a)->val) < *((*(struct int_ptr **)b)->val)){
    return -1;
  }else{
    return 0;
  }
}

void free_int_ptr(void *a){
  struct int_ptr **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
}

void run_prepend_append_free_int_ptr_test(int log_ins){
  int num_ins;
  int start_val = C_START_VAL;
  size_t key_size = sizeof(struct int_ptr *);
  size_t elt_size = sizeof(struct int_ptr *);
  struct dll ll_prep, ll_app;
  struct dll_node *head_prep, *head_app; /* uninitialized pointers for test */
  num_ins = pow_two_perror(log_ins);
  dll_init(&ll_prep, &head_prep, key_size);
  dll_init(&ll_app, &head_app, key_size);
  dll_align_elt(&ll_prep, sizeof(struct int_ptr *));
  dll_align_elt(&ll_app, sizeof(struct int_ptr *));
  printf("Run prepend, append, free test on noncontiguous int_ptr "
	 "keys and elements \n");
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&ll_prep,
		      &ll_app,
		      &head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      key_size,
		      elt_size,
		      new_int_ptr,
		      new_int_ptr,
		      val_int_ptr,
		      val_int_ptr,
		      free_int_ptr,
		      free_int_ptr);
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d (repeat test)\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&ll_prep,
		      &ll_app,
		      &head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      key_size,
		      elt_size,
		      new_int_ptr,
		      new_int_ptr,
		      val_int_ptr,
		      val_int_ptr,
		      free_int_ptr,
		      free_int_ptr);
  start_val += num_ins;
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&ll_prep,
		      &ll_app,
		      &head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      key_size,
		      elt_size,
		      new_int_ptr,
		      new_int_ptr,
		      val_int_ptr,
		      val_int_ptr,
		      free_int_ptr,
		      free_int_ptr);
}

/**
   Runs a corner cases test.
*/
void run_corner_cases_test(){
  int res = 1;
  int key;
  int i;
  size_t key_size = sizeof(int);
  size_t elt_size = sizeof(int);
  struct dll ll_n;
  struct dll ll_prep1, ll_app1;
  struct dll ll_prep2, ll_app2;
  struct dll_node *head_n;
  struct dll_node *head_prep1, *head_app1;
  struct dll_node *head_prep2, *head_app2;
  dll_init(&ll_n, &head_n, key_size);
  dll_init(&ll_prep1, &head_prep1, key_size);
  dll_init(&ll_app1, &head_app1, key_size);
  dll_init(&ll_prep2, &head_prep2, key_size);
  dll_init(&ll_app2, &head_app2, key_size);
  dll_align_elt(&ll_n, sizeof(int));
  dll_align_elt(&ll_prep1, sizeof(int));
  dll_align_elt(&ll_app1, sizeof(int));
  dll_align_elt(&ll_prep2, sizeof(int));
  dll_align_elt(&ll_app2, sizeof(int));
  for (i = 0; i < 2; i++){
    if (i < 1){
      dll_prepend_new(&ll_prep1, &head_prep1, &i, &i, key_size, elt_size);
      dll_append_new(&ll_app1, &head_app1, &i, &i, key_size, elt_size);
    }
    dll_prepend_new(&ll_prep2, &head_prep2, &i, &i, key_size, elt_size);
    dll_append_new(&ll_app2, &head_app2, &i, &i, key_size, elt_size);
  }
  /* search */
  key = 0;
  res *=
    (NULL == dll_search_key(&ll_n, &head_n, &key, key_size, NULL) &&
     NULL != dll_search_key(&ll_prep1, &head_prep1, &key, key_size, NULL) &&
     NULL != dll_search_key(&ll_app1, &head_app1, &key, key_size, NULL) &&
     NULL != dll_search_key(&ll_prep2, &head_prep2, &key, key_size, NULL) &&
     NULL != dll_search_key(&ll_app2, &head_app2, &key, key_size, NULL));
  key = 1;
  res *=
    (NULL == dll_search_key(&ll_n, &head_n, &key, key_size, NULL) &&
     NULL == dll_search_key(&ll_prep1, &head_prep1, &key, key_size, NULL) &&
     NULL == dll_search_key(&ll_app1, &head_app1, &key, key_size, NULL) &&
     NULL != dll_search_key(&ll_prep2, &head_prep2, &key, key_size, NULL) &&
     NULL != dll_search_key(&ll_app2, &head_app2, &key, key_size, NULL));
  key = 2;
  res *=
    (NULL == dll_search_key(&ll_n, &head_n, &key, key_size, NULL) &&
     NULL == dll_search_key(&ll_prep1, &head_prep1, &key, key_size, NULL) &&
     NULL == dll_search_key(&ll_app1, &head_app1, &key, key_size, NULL) &&
     NULL == dll_search_key(&ll_prep2, &head_prep2, &key, key_size, NULL) &&
     NULL == dll_search_key(&ll_app2, &head_app2, &key, key_size, NULL));
  /* delete */
  dll_delete(&ll_n, &head_n, NULL, NULL, NULL);
  dll_delete(&ll_prep1, &head_prep1, NULL, NULL, NULL);
  dll_delete(&ll_app1, &head_app1, NULL, NULL, NULL);
  dll_delete(&ll_prep2, &head_prep2, NULL, NULL, NULL);
  dll_delete(&ll_app2, &head_app2, NULL, NULL, NULL);
  res *=
    (NULL == head_n &&
     0 == *(int *)dll_elt_ptr(&ll_prep1, head_prep1) &&
     0 == *(int *)dll_key_ptr(&ll_prep1, head_prep1) &&
     0 == *(int *)dll_elt_ptr(&ll_app1, head_app1) &&
     0 == *(int *)dll_key_ptr(&ll_app1, head_app1) &&
     1 == *(int *)dll_elt_ptr(&ll_prep2, head_prep2) &&
     1 == *(int *)dll_key_ptr(&ll_prep2, head_prep2) &&
     0 == *(int *)dll_elt_ptr(&ll_app2, head_app2) &&
     0 == *(int *)dll_key_ptr(&ll_app2, head_app2));
  dll_delete(&ll_prep1, &head_prep1, head_prep1, NULL, NULL);
  dll_delete(&ll_app1, &head_app1, head_app1, NULL, NULL);
  dll_delete(&ll_prep2, &head_prep2, head_prep2, NULL, NULL);
  dll_delete(&ll_app2, &head_app2, head_app2, NULL, NULL);
  res *=
    (NULL == head_prep1 &&
     NULL == head_app1 &&
     0 == *(int *)dll_elt_ptr(&ll_prep2, head_prep2) &&
     0 == *(int *)dll_key_ptr(&ll_prep2, head_prep2) &&
     1 == *(int *)dll_elt_ptr(&ll_app2, head_app2) &&
     1 == *(int *)dll_key_ptr(&ll_app2, head_app2));
  dll_delete(&ll_prep2, &head_prep2, head_prep2, NULL, NULL);
  dll_delete(&ll_app2, &head_app2, head_app2, NULL, NULL);
  res *= (NULL == head_prep2 && NULL == head_app2);
  printf("Run corner cases test --> ");
  print_test_result(res);
}

/** Helper functions */

/**
   Runs the prepend, append, and free test routine.
*/
void prepend_append_free(const struct dll *ll_prep,
			 const struct dll *ll_app,
			 struct dll_node **head_prep,
			 struct dll_node **head_app,
			 int start_val,
			 int num_ins,
			 size_t key_size,
			 size_t elt_size,
			 void (*new_key)(void *, int),
			 void (*new_elt)(void *, int),
			 int (*val_key)(const void *),
			 int (*val_elt)(const void *),
			 void (*free_key)(void *),
			 void (*free_elt)(void *)){
  int res = 1;
  int sum_val = 2 * start_val + num_ins - 1; /* < 2**(C_INT_BIT - 1) - 1 */
  int i;
  void *keys_prep = NULL, *keys_app = NULL;
  void *elts_prep = NULL, *elts_app = NULL;
  struct dll_node *node_prep = NULL, *node_app = NULL;
  clock_t t_prep, t_app, t_free_prep, t_free_app;
  keys_prep = malloc_perror(num_ins, key_size);
  keys_app = malloc_perror(num_ins, key_size);
  elts_prep = malloc_perror(num_ins, elt_size);
  elts_app = malloc_perror(num_ins, elt_size);
  for (i = 0; i < num_ins; i++){
    new_key(ptr(keys_prep, i, key_size), start_val + i);
    new_key(ptr(keys_app, i, key_size), start_val + i);
    new_elt(ptr(elts_prep, i, elt_size), start_val + i);
    new_elt(ptr(elts_app, i, elt_size), start_val + i);
  }  
  t_prep = clock();
  for (i = 0; i < num_ins; i++){
    dll_prepend_new(ll_prep,
		    head_prep,
		    ptr(keys_prep, i, key_size),
		    ptr(elts_prep, i, elt_size),
		    key_size,
		    elt_size);
  }
  t_prep = clock() - t_prep;
  t_app = clock();
  for (i = 0; i < num_ins; i++){
    dll_append_new(ll_app,
		   head_app,
		   ptr(keys_app, i, key_size),
		   ptr(elts_app, i, elt_size),
		   key_size,
		   elt_size);
  }
  t_app = clock() - t_app;
  node_prep = *head_prep;
  node_app = *head_app;
  for (i = 0; i < num_ins; i++){
    res = (val_key(dll_key_ptr(ll_prep, node_prep)) +
	   val_key(dll_key_ptr(ll_app, node_app)) ==
	   sum_val);
    res = (val_elt(dll_elt_ptr(ll_prep, node_prep)) +
	   val_elt(dll_elt_ptr(ll_app, node_app)) ==
	   sum_val);
    node_prep = node_prep->next;
    node_app = node_app->next;
  }
  t_free_prep = clock();
  dll_free(ll_prep, head_prep, free_key, free_elt);
  t_free_prep = clock() - t_free_prep;
  t_free_app = clock();
  dll_free(ll_app, head_app, free_key, free_elt);
  t_free_app = clock() - t_free_app;
  res *= (head_prep != NULL && *head_prep == NULL);
  res *= (head_app != NULL && *head_app == NULL);
  printf("\t\tprepend time:            %.4f seconds\n",
	 (float)t_prep / CLOCKS_PER_SEC);
  printf("\t\tappend time:             %.4f seconds\n",
	 (float)t_app / CLOCKS_PER_SEC);
  printf("\t\tfree after prepend time: %.4f seconds\n",
	 (float)t_free_prep / CLOCKS_PER_SEC);
  printf("\t\tfree after append time:  %.4f seconds\n",
	 (float)t_free_app / CLOCKS_PER_SEC);
  printf("\t\tcorrectness:             ");
  print_test_result(res);
  free(keys_prep);
  free(keys_app);
  free(elts_prep);
  free(elts_app);
  keys_prep = NULL;
  keys_app = NULL;
  elts_prep = NULL;
  elts_app = NULL;
}

/**
   Computes a pointer to an element in an array of elements of size size.
*/
void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

/**
   Prints a test result.
*/
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
  if (args[0] > C_INT_BIT - 3 ||
      args[1] > 1 ||
      args[2] > 1 ||
      args[3] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[1]){
    run_prepend_append_free_int_test(args[0]);
  }
  if (args[2]){
    run_prepend_append_free_int_ptr_test(args[0]);
  }
  if (args[3]){
    run_corner_cases_test();
  }
  free(args);
  args = NULL;
  return 0;
}

