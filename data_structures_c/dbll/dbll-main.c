/**
   dbll-main.c

   Examples of a generic dynamically allocated doubly linked list.
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dbll.h"

typedef struct{
  int val;
} int_t;

typedef struct{
  int *val;
} int_ptr_t;

/**
   Frees an int_ptr_t structure.
*/
void int_ptr_t_free(void *s){
  free(((int_ptr_t *)s)->val);
  free((int_ptr_t *)s);
}

/**
   Prints a doubly linked list of integers.
*/
void int_ll_print(dbll_node_t *head){
  if (head != NULL){
    printf("%d ", *(int *)(head->elt));
    int_ll_print(head->next);
  } else {
    printf("\n");
  }
}

/**
   Prints values of int_t structures in a doubly linked list .
*/
void int_t_ll_print(dbll_node_t *head){
  if (head != NULL){
    int_t *s = (int_t *)(head->elt);
    printf("%d ", s->val);
    int_t_ll_print(head->next);
  } else {
    printf("\n");
  }
}

/**
   Prints values pointed to by int_ptr_t structures in a doubly linked list .
*/
void int_ptr_t_ll_print(dbll_node_t *head){
  if (head != NULL){
    int_ptr_t *s = (int_ptr_t *)(head->elt);
    printf("%d ", *(s->val));
    int_ptr_t_ll_print(head->next);
  } else {
    printf("\n");
  }
}

int main(){
  dbll_node_t *head;
  int num_nodes = 10;
  int int_elt;
  int_t int_t_elt;
  int_ptr_t int_ptr_t_elt;
  dbll_init(&head);
  //create, print, and free a doubly linked list of integers.
  for (int i = 0; i < num_nodes; i++){
    int_elt = i;
    dbll_insert(&head, &int_elt, sizeof(int));
  }
  printf("Linked list of ints:\n");
  int_ll_print(head);
  printf("Last int_elt value before freeing: %d\n", int_elt);
  dbll_free(&head, NULL);
  printf("Last int_elt value after freeing: %d\n\n", int_elt);
  // create, print, and free a doubly linked list of int_t structs.
  for (int i = 0; i < num_nodes; i++){
    int_t_elt.val = i;
    dbll_insert(&head, &int_t_elt, sizeof(int_t_elt));
  }
  printf("Linked list of int_t structs:\n");
  int_t_ll_print(head);
  printf("Last int_t value before freeing: %d\n", int_t_elt.val);
  dbll_free(&head, NULL);
  printf("Last int_t value after freeing: %d\n\n", int_t_elt.val);
  //create, print, and free a doubly linked list of int_ptr_t structs.
  for (int i = 0; i < num_nodes; i++){
    int_ptr_t_elt.val = (int *)malloc(sizeof(int));
    assert(int_ptr_t_elt.val != NULL);
    *(int_ptr_t_elt.val) = i;
    dbll_insert(&head, &int_ptr_t_elt, sizeof(int_ptr_t_elt));
  }
  printf("Linked list of int_ptr_t structs:\n");
  int_ptr_t_ll_print(head);
  printf("Last int_ptr_t value before freeing: %d\n", *(int_ptr_t_elt.val));
  dbll_free(&head, int_ptr_t_free);
  printf("Last int_ptr_t value after freeing: %d\n", *(int_ptr_t_elt.val));
  return 0;
}
