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
			 int init_val,
			 int end_val,
			 int *result,
			 int (*val_fn)(void *));

/**
   Run tests of a doubly linked list of integer elements. A pointer to an 
   integer is passed as elt in dll_insert and the integer element is fully  
   copied into the block pointed to by elt in a node. NULL as free_int_fn 
   is sufficient to free the doubly linked list.
*/
static void int_dll_test_helper(dll_node_t **head,
                                int init_val,
				int num_elts);

int int_val_fn(void *elt){
  return *(int *)elt;
}

void run_int_dll_test(){
  dll_node_t *head;
  int num_elts = 10000000;
  int init_val;
  dll_init(&head);
  init_val = 0;
  printf("Run dll tests on int elements \n");
  printf("\tinitial value: %d, number of elements: %d\n", init_val, num_elts);
  int_dll_test_helper(&head, init_val, num_elts);
  printf("\tinitial value: %d, number of elements: %d (repeat test)\n",
	 init_val, num_elts);
  int_dll_test_helper(&head, init_val, num_elts);
  init_val = num_elts;
  printf("\tinitial value: %d, number of elements: %d\n", init_val, num_elts);
  int_dll_test_helper(&head, init_val, num_elts);
}

static void int_dll_test_helper(dll_node_t **head,
                                int init_val,
				int num_elts){
  int end_val = init_val + num_elts - 1;
  int result = 1;
  clock_t t;
  t = clock();
  for (int i = init_val; i < init_val + num_elts; i++){
    dll_insert(head, &i, sizeof(int));
  }
  t = clock() - t;
  printf("\t\tinsert time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
  dll_traverse(head, init_val, end_val, &result, int_val_fn);
  t = clock();
  dll_free(head, NULL);
  t = clock() - t;
  result *= (head != NULL && *head == NULL);
  printf("\t\torder correctness --> ");
  print_test_result(result);
  printf("\t\tfree time: %.4f seconds\n", (float)t / CLOCKS_PER_SEC);
}

/**
   Run tests of a doubly linked list of int_ptr_t elements. A pointer to a 
   pointer to an int_ptr_t element is passed as elt in dll_insert and the 
   pointer to the int_ptr_t element is copied into the block pointed to by 
   elt in a node. A int_ptr_t-specific free_int_fn is necessary to free 
   the doubly linked list.
*/

typedef struct{
  int *val;
} int_ptr_t;

void free_int_ptr_t_fn(void *elt){
  int_ptr_t **s = elt;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

int int_ptr_t_val_fn(void *elt){
  int_ptr_t **s  = elt;
  return *((*s)->val);
}

static void int_ptr_t_dll_test_helper(dll_node_t **head,
				      int init_val,
				      int num_elts);

void run_int_ptr_t_dll_test(){
  dll_node_t *head;
  int num_elts = 10000000;
  int init_val;
  dll_init(&head);
  init_val = 0;
  printf("Run dll tests on int_ptr_t elements (multilayered objects)\n");
  printf("\tinitial value: %d, number of elements: %d\n", init_val, num_elts);
  int_ptr_t_dll_test_helper(&head, init_val, num_elts);
  printf("\tinitial value: %d, number of elements: %d (repeat test)\n",
	 init_val, num_elts);
  int_ptr_t_dll_test_helper(&head, init_val, num_elts);
  init_val = num_elts;
  printf("\tinitial value: %d, number of elements: %d\n", init_val, num_elts);
  int_ptr_t_dll_test_helper(&head, init_val, num_elts);
}

static void int_ptr_t_dll_test_helper(dll_node_t **head,
				      int init_val,
				      int num_elts){
  int end_val = init_val + num_elts - 1;
  int result = 1;
  int_ptr_t *ins;
  clock_t t;
  t = clock();
  for (int i = init_val; i < init_val + num_elts; i++){
    ins = malloc(sizeof(int_ptr_t));
    assert(ins != NULL);
    ins->val = malloc(sizeof(int));
    assert(ins->val != NULL);
    *(ins->val) = i;
    dll_insert(head, &ins, sizeof(int_ptr_t *));
    ins = NULL;
  }
  t = clock() - t;
  printf("\t\tinsert time: %.4f seconds (incl. element allocation)\n",
	 (float)t / CLOCKS_PER_SEC);
  dll_traverse(head, init_val, end_val, &result, int_ptr_t_val_fn);
  t = clock();
  dll_free(head, free_int_ptr_t_fn);
  t = clock() - t;
  result *= (head != NULL && *head == NULL);
  printf("\t\torder correctness --> ");
  print_test_result(result);
  printf("\t\tfree time: %.4f seconds\n", 
         (float)t / CLOCKS_PER_SEC);
}

/**
   Traverses a doubly linked list and tests the order of integer values.
*/
static void dll_traverse(dll_node_t **head,
			 int init_val,
			 int end_val,
			 int *result,
			 int (*val_fn)(void *)){
  if (*head == NULL){return;}
  int cur_val = end_val;
  dll_node_t *cur_node = *head;
  while(cur_node->next != NULL){
    *result *= (cur_val == val_fn(cur_node->elt));
    cur_node = cur_node->next;
    cur_val--;
  }
  *result *= (cur_val == init_val);
  while(cur_node->prev != NULL){
    *result *= (cur_val == val_fn(cur_node->elt));
    cur_node = cur_node->prev;
    cur_val++;
  }
  *result *= (cur_val == val_fn(cur_node->elt));
  *result *= (cur_val == end_val);
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
  run_int_dll_test();
  run_int_ptr_t_dll_test();
  return 0;
}
