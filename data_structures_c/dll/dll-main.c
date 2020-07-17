/**
   dll-main.c

   Examples of a generic dynamically allocated doubly linked list.
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "dll.h"

void print_test_result(int result);
static void dll_traverse(dll_node_t **head,
			 int start_val,
			 int end_val,
			 int *result,
			 int (*val_fn)(dll_node_t *));
static void search_delete_key_elt(dll_node_t **head,
				  int start_val,
				  int end_val,
				  int start_sd_val,
				  int num_sd_nodes,
				  int (*cmp_key_fn)(void *, void *),
				  int (*cmp_elt_fn)(void *, void *),
				  void (*cstr_elt_fn)(void *, int),
				  void (*free_elt_fn)(void *),
				  int (*key_val_fn)(dll_node_t *),
				  int (*elt_val_fn)(dll_node_t *));

/**
   Run tests of a doubly linked list of integer keys and integer elements. 
   A pointer to an integer is passed as elt in dll_insert and the integer 
   element is fully  copied into the block pointed to by elt in a node. 
   NULL as free_int_fn is sufficient to delete a node.
*/

int int_elt_val_fn(dll_node_t *n){
  return *(int *)n->elt;
}

int int_key_val_fn(dll_node_t *n){
  return *(int *)n->key;
}

int cmp_int_fn(void *a, void *b){
  return *(int *)a - *(int *)b;
}

static void insert_free_int_test_helper(dll_node_t **head,
					int start_val,
					int num_elts);
/**
   Run a dll_{insert, free} test on integer keys and integer elements.
*/
void run_insert_free_int_test(){
  dll_node_t *head;
  int num_nodes = 10000000;
  int start_val;
  dll_init(&head);
  start_val = 0;
  printf("Run dll_{insert, free} test on int keys and int elements \n");
  printf("\tstart key value: %d, start elt value: %d, "
	 "# nodes: %d\n", start_val, start_val, num_nodes);
  insert_free_int_test_helper(&head, start_val, num_nodes);
  printf("\tstart key value: %d, start elt value: %d, "
	 "# nodes: %d (repeat test)\n", start_val, start_val, num_nodes);
  insert_free_int_test_helper(&head, start_val, num_nodes);
  start_val = num_nodes;
  printf("\tstart key value: %d, start elt value: %d, "
	 "# nodes: %d\n", start_val, start_val, num_nodes);
  insert_free_int_test_helper(&head, start_val, num_nodes);
}

/** Helper functions for dll_{insert, free} test */

static void insert_int_test_helper(dll_node_t **head,
				   int start_val,
				   int num_nodes){
  for (int i = start_val  + num_nodes - 1; i >= start_val; i--){
    dll_insert(head, &i, &i, sizeof(int), sizeof(int));
  }
}

static void free_int_test_helper(dll_node_t **head){
  dll_free(head, NULL);
}

static void insert_free_int_test_helper(dll_node_t **head,
					int start_val,
					int num_nodes){
  int end_val = start_val + num_nodes - 1;
  int result = 1;
  clock_t t;
  t = clock();
  insert_int_test_helper(head, start_val, num_nodes);
  t = clock() - t;
  printf("\t\tinsert time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  dll_traverse(head, start_val, end_val, &result, int_key_val_fn);
  dll_traverse(head, start_val, end_val, &result, int_elt_val_fn);
  t = clock();
  free_int_test_helper(head);
  t = clock() - t;
  result *= (head != NULL && *head == NULL);
  printf("\t\tfree time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness --> ");
  print_test_result(result);
}

/**
   Run a dll_search_{key, elt} and dll_delete test on integer keys and 
   integer elements.
*/
void run_search_delete_int_test(){
  dll_node_t *head;
  int num_nodes = 10000000;
  int num_sd_nodes = 200;
  int start_sd_val;
  int start_val;
  int end_val;
  dll_init(&head);
  printf("Run dll_search_{key, elt} and dll_delete test on int keys and int "
	 "elements in a list of %d nodes\n", num_nodes);
  start_val = 0;
  insert_int_test_helper(&head, start_val, num_nodes);
  //search the entire list for values that are not in the list
  printf("\tsearch for %d nodes not in the list: \n", num_sd_nodes / 2);
  start_sd_val = num_nodes;
  end_val = num_nodes - 1;
  search_delete_key_elt(&head,
			start_val,
			end_val,
			start_sd_val,
			num_sd_nodes,
			cmp_int_fn,
			cmp_int_fn,
			NULL,
			NULL,
			int_key_val_fn,
			int_elt_val_fn);
  //search and delete the last 200 nodes in total
  printf("\tsearch and delete %d nodes at the end of the list: \n",
	 num_sd_nodes / 2);
  start_sd_val = num_nodes - num_sd_nodes;
  end_val = start_sd_val - 1;
  search_delete_key_elt(&head,
			start_val,
			end_val,
			start_sd_val,
			num_sd_nodes,
			cmp_int_fn,
			cmp_int_fn,
			NULL,
			NULL,
			int_key_val_fn,
			int_elt_val_fn);
  //search and delete the first 200 nodes in total
  printf("\tsearch and delete %d nodes at the beginning of the list: \n",
	 num_sd_nodes / 2);
  start_sd_val = 0;
  start_val = num_sd_nodes;
  search_delete_key_elt(&head,
			start_val,
			end_val,
			start_sd_val,
			num_sd_nodes,
			cmp_int_fn,
			cmp_int_fn,
			NULL,
			NULL,
			int_key_val_fn,
			int_elt_val_fn);
  free_int_test_helper(&head);
}

/**
   Run a corner cases test.
*/
void run_corner_cases_test(){
  dll_node_t *head_none;
  dll_node_t *head_one;
  dll_node_t *head_two;
  int start_val = 0;
  int result = 1;
  int key;
  int elt;
  dll_init(&head_none);
  dll_init(&head_one);
  dll_init(&head_two);
  insert_int_test_helper(&head_one, start_val, 1);
  insert_int_test_helper(&head_two, start_val, 2);
  //search
  key = 0;
  elt = 0;
  result *= (NULL == dll_search_key(&head_none, &key, cmp_int_fn));
  result *= (NULL == dll_search_elt(&head_none, &elt, cmp_int_fn));
  result *= (NULL != dll_search_key(&head_one, &key, cmp_int_fn));
  result *= (NULL != dll_search_elt(&head_one, &elt, cmp_int_fn));
  result *= (NULL != dll_search_key(&head_two, &key, cmp_int_fn));
  result *= (NULL != dll_search_elt(&head_two, &elt, cmp_int_fn));
  key = 2;
  elt = 2;
  result *= (NULL == dll_search_key(&head_none, &key, cmp_int_fn));
  result *= (NULL == dll_search_elt(&head_none, &elt, cmp_int_fn));
  result *= (NULL == dll_search_key(&head_one, &key, cmp_int_fn));
  result *= (NULL == dll_search_elt(&head_one, &elt, cmp_int_fn));
  result *= (NULL == dll_search_key(&head_two, &key, cmp_int_fn));
  result *= (NULL == dll_search_elt(&head_two, &elt, cmp_int_fn));
  dll_traverse(&head_one, 0, 0, &result, int_elt_val_fn);
  dll_traverse(&head_one, 0, 0, &result, int_key_val_fn);
  dll_traverse(&head_two, 0, 1, &result, int_elt_val_fn);
  dll_traverse(&head_two, 0, 1, &result, int_key_val_fn);
  //delete
  dll_delete(&head_none, NULL, NULL);		
  result *= (NULL == head_none);
  dll_delete(&head_one, NULL, NULL);
  dll_delete(&head_two, NULL, NULL);
  dll_traverse(&head_one, 0, 0, &result, int_elt_val_fn);
  dll_traverse(&head_one, 0, 0, &result, int_key_val_fn);
  dll_traverse(&head_two, 0, 1, &result, int_elt_val_fn);
  dll_traverse(&head_two, 0, 1, &result, int_key_val_fn);
  dll_delete(&head_one, head_one, NULL);
  result *= (NULL == head_one);
  dll_delete(&head_two, head_two, NULL);
  dll_traverse(&head_two, 1, 1, &result, int_elt_val_fn);
  dll_traverse(&head_two, 1, 1, &result, int_key_val_fn);
  dll_delete(&head_two, head_two, NULL);
  result *= (NULL == head_two); 
  //free
  dll_free(&head_two, NULL);
  result *= (NULL == head_two);
  printf("Run corner cases test --> ");
  print_test_result(result);
}

/**
   Run tests of a doubly linked list of integer keys and int_ptr_t elements. 
   A pointer to a pointer to an int_ptr_t element is passed as elt in 
   dll_insert and the pointer to the int_ptr_t element is copied into the 
   block pointed to by elt in a node. A int_ptr_t-specific free_int_ptr_t_fn
   is  necessary to delete a node.
*/

typedef struct{
  int *val;
} int_ptr_t;

void cstr_int_ptr_t_fn(void *elt, int val){
  int_ptr_t **s = elt;
  (*s) = malloc(sizeof(int_ptr_t));
  assert((*s) != NULL);
  (*s)->val = malloc(sizeof(int));
  assert((*s)->val != NULL);
  *((*s)->val) = val;
}

void free_int_ptr_t_fn(void *elt){
  int_ptr_t **s = elt;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

int cmp_int_ptr_t_fn(void *elt_a, void *elt_b){
  return *((*(int_ptr_t **)elt_a)->val) - *((*(int_ptr_t **)elt_b)->val);
}

int int_ptr_t_elt_val_fn(dll_node_t *n){
  int_ptr_t **s  = n->elt;
  return *((*s)->val);
}

static void insert_free_int_ptr_t_test_helper(dll_node_t **head,
					      int start_val,
					      int num_nodes);

/**
   Run a dll_{insert, free} test on integer keys and int_ptr_t elements.
*/
void run_insert_free_int_ptr_t_test(){
  dll_node_t *head;
  int num_nodes = 10000000;
  int start_val;
  dll_init(&head);
  start_val = 0;
  printf("Run dll_{insert, free} test on int keys and multilayered "
	 "int_ptr_t elements\n");
  printf("\tstart key value: %d, start elt->val value: %d, "
	 "# nodes: %d\n", start_val, start_val, num_nodes);
  insert_free_int_ptr_t_test_helper(&head, start_val, num_nodes);
  printf("\tstart key value: %d, start elt->val value: %d, "
	 "# nodes: %d (repeat test)\n", start_val, start_val, num_nodes);
  insert_free_int_ptr_t_test_helper(&head, start_val, num_nodes);
  start_val = num_nodes;
  printf("\tstart key value: %d, start elt->val value: %d, "
	 "# nodes: %d\n", start_val, start_val, num_nodes);
  insert_free_int_ptr_t_test_helper(&head, start_val, num_nodes);
}

/** Helper functions for dll_{insert, free} test */

static void insert_int_ptr_t_test_helper(dll_node_t **head,
					 int start_val,
					 int num_nodes){
  int_ptr_t *n;
  for (int i = start_val  + num_nodes - 1; i >= start_val; i--){
    n = malloc(sizeof(int_ptr_t));
    assert(n != NULL);
    n->val = malloc(sizeof(int));
    assert(n->val != NULL);
    *(n->val) = i;
    //a pointer to a pointer to an element in dll_insert
    dll_insert(head, &i, &n, sizeof(int), sizeof(int_ptr_t *));
    n = NULL;
  }
}

static void free_int_ptr_t_test_helper(dll_node_t **head){
  dll_free(head, free_int_ptr_t_fn);
}

static void insert_free_int_ptr_t_test_helper(dll_node_t **head,
					      int start_val,
					      int num_nodes){
  int end_val = start_val + num_nodes - 1;
  int result = 1;
  clock_t t;
  t = clock();
  insert_int_ptr_t_test_helper(head, start_val, num_nodes);
  t = clock() - t;
  printf("\t\tinsert time: %.4f seconds (incl. element allocation)\n",
	 (float)t / CLOCKS_PER_SEC);
  dll_traverse(head, start_val, end_val, &result, int_key_val_fn);
  dll_traverse(head, start_val, end_val, &result, int_ptr_t_elt_val_fn);
  t = clock();
  free_int_ptr_t_test_helper(head);
  t = clock() - t;
  result *= (head != NULL && *head == NULL);
  printf("\t\tfree time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  printf("\t\torder correctness --> ");
  print_test_result(result);
}

/**
   Run a dll_search_{key, elt} and dll_delete test on integer keys and 
   int_ptr_t elements.
*/
void run_search_delete_int_ptr_t_test(){
  dll_node_t *head;
  int num_nodes = 10000000;
  int num_sd_nodes = 200;
  int start_sd_val;
  int start_val;
  int end_val;
  dll_init(&head);
  printf("Run dll_search_{key, elt} and dll_delete test on int keys and "
	 "int_ptr_t elements in a list of %d nodes\n", num_nodes);
  start_val = 0;
  insert_int_ptr_t_test_helper(&head, start_val, num_nodes);
  //search the entire list for values that are not in the list
  printf("\tsearch for %d nodes not in the list: \n", num_sd_nodes / 2);
  start_sd_val = num_nodes;
  end_val = num_nodes - 1;
  search_delete_key_elt(&head,
			start_val,
			end_val,
			start_sd_val,
			num_sd_nodes,
			cmp_int_fn,
			cmp_int_ptr_t_fn,
			cstr_int_ptr_t_fn,
			free_int_ptr_t_fn,
			int_key_val_fn,
			int_ptr_t_elt_val_fn);
  //search and delete the last 200 nodes in total
  printf("\tsearch and delete %d nodes at the end of the list: \n",
	 num_sd_nodes / 2);
  start_sd_val = num_nodes - num_sd_nodes;
  end_val = start_sd_val - 1;
  search_delete_key_elt(&head,
			start_val,
			end_val,
			start_sd_val,
			num_sd_nodes,
			cmp_int_fn,
			cmp_int_ptr_t_fn,
			cstr_int_ptr_t_fn,
			free_int_ptr_t_fn,
			int_key_val_fn,
			int_ptr_t_elt_val_fn);
  //search and delete the first 200 nodes in total
  printf("\tsearch and delete %d nodes at the beginning of the list: \n",
	 num_sd_nodes / 2);
  start_sd_val = 0;
  start_val = num_sd_nodes;
  search_delete_key_elt(&head,
			start_val,
			end_val,
			start_sd_val,
			num_sd_nodes,
			cmp_int_fn,
			cmp_int_ptr_t_fn,
			cstr_int_ptr_t_fn,
			free_int_ptr_t_fn,
			int_key_val_fn,
			int_ptr_t_elt_val_fn);
  free_int_ptr_t_test_helper(&head);
}

/** General helper functions */

/**
   Searches and deletes num_sd_nodes nodes with key values starting from 
   start_sd_val.
*/
static void search_delete_key(dll_node_t **head,
			      int start_sd_val,
			      int num_sd_nodes,
			      int (*cmp_key_fn)(void *, void *),
			      void (*free_elt_fn)(void *)){
  dll_node_t *n_ptr;
  for (int i = start_sd_val; i < start_sd_val + num_sd_nodes; i++){
    n_ptr = dll_search_key(head, &i, cmp_key_fn);
    dll_delete(head, n_ptr, free_elt_fn);
  }
}

/**
   Searches and deletes num_sd_nodes nodes with element values starting from 
   start_sd_val.
*/
static void search_delete_elt(dll_node_t **head,
			      int start_sd_val,
			      int num_sd_nodes,
			      int (*cmp_elt_fn)(void *, void *),
			      void (*cstr_elt_fn)(void *, int),
			      void (*free_elt_fn)(void *)){
  dll_node_t *n_ptr;
  void *elt;
  for (int i = start_sd_val; i < start_sd_val + num_sd_nodes; i++){
    if (cstr_elt_fn != NULL){
      cstr_elt_fn(&elt, i);
      n_ptr = dll_search_elt(head, &elt, cmp_elt_fn);
    }else{
      n_ptr = dll_search_elt(head, &i, cmp_elt_fn);
    }
    dll_delete(head, n_ptr, free_elt_fn);
  }
}
/**
   Performs search and delete tests with respect to key and element values.
*/
static void search_delete_key_elt(dll_node_t **head,
				  int start_val,
				  int end_val,
				  int start_sd_val,
				  int num_sd_nodes,
				  int (*cmp_key_fn)(void *, void *),
				  int (*cmp_elt_fn)(void *, void *),
				  void (*cstr_elt_fn)(void *, int),
				  void (*free_elt_fn)(void *),
				  int (*key_val_fn)(dll_node_t *),
				  int (*elt_val_fn)(dll_node_t *)){
  int result = 1;
  clock_t t;
  assert(!(num_sd_nodes % 2));
  int num_sd_nodes_half = num_sd_nodes / 2;
  t = clock();
  search_delete_key(head,
		    start_sd_val,
		    num_sd_nodes_half,
		    cmp_key_fn,
		    free_elt_fn);
  t = clock() - t;
  printf("\t\tby key time: %.8f seconds\n", (float)t / CLOCKS_PER_SEC);
  start_sd_val = start_sd_val + num_sd_nodes_half;
  t = clock();
  search_delete_elt(head,
		    start_sd_val,
		    num_sd_nodes_half,
		    cmp_elt_fn,
		    cstr_elt_fn,
		    free_elt_fn);
  t = clock() - t;
  printf("\t\tby elt time: %.8f seconds\n", (float)t / CLOCKS_PER_SEC);
  dll_traverse(head, start_val, end_val, &result, key_val_fn);
  dll_traverse(head, start_val, end_val, &result, elt_val_fn);
  printf("\t\torder correctness --> ");
  print_test_result(result);
}

/**
   Traverses a doubly linked list with keys or elements containing integers
   in increasing order and tests the order of integer values.
*/
static void dll_traverse(dll_node_t **head,
			 int start_val,
			 int end_val,
			 int *result,
			 int (*val_fn)(dll_node_t *)){
  int cur_val = start_val;
  dll_node_t *cur_node = *head;
  if (*head == NULL){
    return;
  }else if (cur_node->next == NULL && cur_node->prev == NULL){
    *result *= (cur_val == val_fn(cur_node) &&
		cur_val == end_val);
  }else{
    //printf("list begin value: %d\n", val_fn(cur_node));
    while(cur_node->next != NULL){
      *result *= (cur_val == val_fn(cur_node));
      cur_node = cur_node->next;
      cur_val++;
    }
    *result *= (cur_val == end_val);
    //printf("list end value: %d\n", val_fn(cur_node));
    while(cur_node->prev != NULL){
      *result *= (cur_val == val_fn(cur_node));
      cur_node = cur_node->prev;
      cur_val--;
    }
    //printf("list begin value: %d\n", val_fn(cur_node));
    *result *= (cur_val == val_fn(cur_node));
    *result *= (cur_val == start_val);
  }
}

/**
   Print test result.
*/
void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_insert_free_int_test();
  run_search_delete_int_test();
  run_corner_cases_test();
  run_insert_free_int_ptr_t_test();
  run_search_delete_int_ptr_t_test();
  return 0;
}
