/**
   dll-main.c

   Tests of a generic dynamically allocated doubly linked list in a
   circular representation.  
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dll.h"
#include "utilities-mem.h"

void prepend_append_free(dll_node_t **head_prep,
			 dll_node_t **head_app,
			 int start_val,
			 int count,
			 int elt_size,
			 void (*new_elt)(void *, int),
			 int (*val_elt)(const void *),
			 void (*free_elt)(void *));
void *elt_ptr(const void *elts, size_t i, size_t elt_size);
void print_dll(dll_node_t **head, int (*val)(const dll_node_t *));
void print_test_result(int res);

/**
   Run tests of a doubly linked list of integer keys and integer elements. 
   A pointer to an integer is passed as elt in dll_prepend and dll_append,
   and the integer element is fully copied into the block pointed to by elt
   in a node. NULL as free_elt is sufficient to delete a node.
*/

void new_int_fn(void *a, int val){
  *(int *)a = val;
}

int val_int_fn(const void *a){
  return *(int *)a;
}

int cmp_int_fn(const void *a, const void *b){
  if (*(int *)a == *(int *)b){
    return 0;
  }else{
    return 1;
  }
}

void run_prepend_append_free_int_test(){
  int count = 10000000;
  int start_val = 0;
  dll_node_t *head_prep, *head_app; //uninitialized pointers
  dll_init(&head_prep);
  dll_init(&head_app);
  printf("Run dll_{prepend, append, free} test on int keys and int "
	 "elements\n");
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, count);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      count,
		      sizeof(int),
		      new_int_fn,
		      val_int_fn,
		      NULL);
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d (repeat test)\n",
	 start_val, start_val, count);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      count,
		      sizeof(int),
		      new_int_fn,
		      val_int_fn,
		      NULL);
  start_val = count;
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, count);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      count,
		      sizeof(int),
		      new_int_fn,
		      val_int_fn,
		      NULL);
}

/**
   Run tests of a doubly linked list of integer keys and int_ptr_t elements. 
   A pointer to a pointer to an int_ptr_t element is passed as elt in 
   dll_prepend and dll_append and the pointer to the int_ptr_t element is
   copied into the block pointed to by elt in a node. A int_ptr_t-specific
   free_elt is necessary to delete a node.
*/

typedef struct{
  int *val;
} int_ptr_t;

void new_int_ptr_fn(void *a, int val){
  int_ptr_t **s = a;
  (*s) = malloc_perror(sizeof(int_ptr_t));
  (*s)->val = malloc_perror(sizeof(int));
  *((*s)->val) = val;
  s = NULL;
}

int val_int_ptr_fn(const void *a){
  return *((*(int_ptr_t **)a)->val);
}

int cmp_int_ptr_fn(const void *a, const void *b){
  if (*((*(int_ptr_t **)a)->val) > *((*(int_ptr_t **)b)->val)){
    return 1;
  }else if (*((*(int_ptr_t **)a)->val) < *((*(int_ptr_t **)b)->val)){
    return -1;
  }else{
    return 0;
  }
}

void free_int_ptr_fn(void *a){
  int_ptr_t **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

void run_prepend_append_free_int_ptr_test(){
  int count = 10000000;
  int start_val = 0;
  dll_node_t *head_prep, *head_app; //uninitialized pointers
  dll_init(&head_prep);
  dll_init(&head_app);
  printf("Run dll_{prepend, append, free} test on int keys and noncontiguous "
         "int_ptr_t elements \n");
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, count);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      count,
		      sizeof(int_ptr_t *),
		      new_int_ptr_fn,
		      val_int_ptr_fn,
		      free_int_ptr_fn);
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d (repeat test)\n",
	 start_val, start_val, count);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      count,
		      sizeof(int_ptr_t *),
		      new_int_ptr_fn,
		      val_int_ptr_fn,
		      free_int_ptr_fn);
  start_val = count;
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, count);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      count,
		      sizeof(int_ptr_t *),
		      new_int_ptr_fn,
		      val_int_ptr_fn,
		      free_int_ptr_fn);
}

/**
   Runs a corner cases tests.
*/
void run_corner_cases_test(){
  dll_node_t *head_none;
  dll_node_t *head_one_prep, *head_one_app;
  dll_node_t *head_two_prep, *head_two_app;
  int res = 1;
  int key, elt;
  dll_init(&head_none);
  dll_init(&head_one_prep);
  dll_init(&head_one_app);
  dll_init(&head_two_prep);
  dll_init(&head_two_app);
  for (int i = 0; i < 2; i++){
    if (i < 1){
      dll_prepend(&head_one_prep, &i, &i, sizeof(int), sizeof(int));
      dll_append(&head_one_app, &i, &i, sizeof(int), sizeof(int));
    }
  dll_prepend(&head_two_prep, &i, &i, sizeof(int), sizeof(int));
  dll_append(&head_two_app, &i, &i, sizeof(int), sizeof(int));
  }
  //search
  key = 0;
  elt = 0;
  res *= (NULL == dll_search_key(&head_none, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_none, &elt, cmp_int_fn));
  res *= (NULL != dll_search_key(&head_one_prep, &key, cmp_int_fn));
  res *= (NULL != dll_search_elt(&head_one_prep, &elt, cmp_int_fn));
  res *= (NULL != dll_search_key(&head_one_app, &key, cmp_int_fn));
  res *= (NULL != dll_search_elt(&head_one_app, &elt, cmp_int_fn));
  res *= (NULL != dll_search_key(&head_two_prep, &key, cmp_int_fn));
  res *= (NULL != dll_search_elt(&head_two_prep, &elt, cmp_int_fn));
  res *= (NULL != dll_search_key(&head_two_app, &key, cmp_int_fn));
  res *= (NULL != dll_search_elt(&head_two_app, &elt, cmp_int_fn));
  key = 1;
  elt = 1;
  res *= (NULL == dll_search_key(&head_none, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_none, &elt, cmp_int_fn));
  res *= (NULL == dll_search_key(&head_one_prep, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_one_prep, &elt, cmp_int_fn));
  res *= (NULL == dll_search_key(&head_one_app, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_one_app, &elt, cmp_int_fn));
  res *= (NULL != dll_search_key(&head_two_prep, &key, cmp_int_fn));
  res *= (NULL != dll_search_elt(&head_two_prep, &elt, cmp_int_fn));
  res *= (NULL != dll_search_key(&head_two_app, &key, cmp_int_fn));
  res *= (NULL != dll_search_elt(&head_two_app, &elt, cmp_int_fn));
  key = 2;
  elt = 2;
  res *= (NULL == dll_search_key(&head_none, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_none, &elt, cmp_int_fn));
  res *= (NULL == dll_search_key(&head_one_prep, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_one_prep, &elt, cmp_int_fn));
  res *= (NULL == dll_search_key(&head_one_app, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_one_app, &elt, cmp_int_fn));
  res *= (NULL == dll_search_key(&head_two_prep, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_two_prep, &elt, cmp_int_fn));
  res *= (NULL == dll_search_key(&head_two_app, &key, cmp_int_fn));
  res *= (NULL == dll_search_elt(&head_two_app, &elt, cmp_int_fn));
  //delete
  dll_delete(&head_none, NULL, NULL);
  dll_delete(&head_one_prep, NULL, NULL);
  dll_delete(&head_one_app, NULL, NULL);
  dll_delete(&head_two_prep, NULL, NULL);
  dll_delete(&head_two_app, NULL, NULL);
  res *= (NULL == head_none);
  res *= (0 == *(int *)head_one_prep->elt);
  res *= (0 == *(int *)head_one_prep->key);
  res *= (0 == *(int *)head_one_app->elt);
  res *= (0 == *(int *)head_one_app->key);
  res *= (1 == *(int *)head_two_prep->elt);
  res *= (1 == *(int *)head_two_prep->key);
  res *= (0 == *(int *)head_two_app->elt);
  res *= (0 == *(int *)head_two_app->key);
  dll_delete(&head_one_prep, head_one_prep, NULL);
  dll_delete(&head_one_app, head_one_app, NULL);
  dll_delete(&head_two_prep, head_two_prep, NULL);
  dll_delete(&head_two_app, head_two_app, NULL);
  res *= (NULL == head_one_prep);
  res *= (NULL == head_one_app);
  res *= (0 == *(int *)head_two_prep->elt);
  res *= (0 == *(int *)head_two_prep->key);
  res *= (1 == *(int *)head_two_app->elt);
  res *= (1 == *(int *)head_two_app->key);
  dll_delete(&head_two_prep, head_two_prep, NULL);
  dll_delete(&head_two_app, head_two_app, NULL);
  res *= (NULL == head_two_prep);
  res *= (NULL == head_two_app);
  printf("Run corner cases test --> ");
  print_test_result(res);
}

/** Helper functions */

/**
   Runs the prepend, append, and free test routine.
*/
void prepend_append_free(dll_node_t **head_prep,
			 dll_node_t **head_app,
			 int start_val,
			 int count,
			 int elt_size,
			 void (*new_elt)(void *, int),
			 int (*val_elt)(const void *),
			 void (*free_elt)(void *)){
  int res = 1;
  int sum_val = 2 * start_val + count - 1;
  int *keys = NULL;
  void *elts_prep = NULL, *elts_app = NULL;
  dll_node_t *node_prep = NULL, *node_app = NULL;
  clock_t t_prep, t_app, t_free_prep, t_free_app;
  keys = malloc_perror(count * sizeof(int));
  elts_prep = malloc_perror(count * elt_size);
  elts_app = malloc_perror(count * elt_size);
  for (int i = 0; i < count; i++){
    keys[i] = start_val + i;
    new_elt(elt_ptr(elts_prep, i, elt_size), start_val + i);
    new_elt(elt_ptr(elts_app, i, elt_size), start_val + i);
  }  
  t_prep = clock();
  for (int i = 0; i < count; i++){
    dll_prepend(head_prep,
		&keys[i],
		elt_ptr(elts_prep, i, elt_size),
		sizeof(int),
		elt_size);
  }
  t_prep = clock() - t_prep;
  t_app = clock();
  for (int i = 0; i < count; i++){
    dll_append(head_app,
	       &keys[i],
	       elt_ptr(elts_app, i, elt_size),
	       sizeof(int),
	       elt_size);
  }
  t_app = clock() - t_app;
  node_prep = *head_prep;
  node_app = *head_app;
  for (int i = 0; i < count; i++){
    res = (*(int *)node_prep->key + *(int *)node_app->key == sum_val);
    res = (val_elt(node_prep->elt) + val_elt(node_app->elt) == sum_val);
    node_prep = node_prep->next;
    node_app = node_app->next;
  }
  t_free_prep = clock();
  dll_free(head_prep, free_elt);
  t_free_prep = clock() - t_free_prep;
  t_free_app = clock();
  dll_free(head_app, free_elt);
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
  free(keys);
  free(elts_prep);
  free(elts_app);
  keys = NULL;
  elts_prep = NULL;
  elts_app = NULL;
}

/**
   Computes a pointer to an element in an element array.
*/
void *elt_ptr(const void *elts, size_t i, size_t elt_size){
  return (void *)((char *)elts + i * elt_size);
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

int main(){
  run_prepend_append_free_int_test();
  run_prepend_append_free_int_ptr_test();
  run_corner_cases_test();
  return 0;
}
