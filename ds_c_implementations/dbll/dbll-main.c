/**
   dbll-main.c

   Examples of a generic dynamicaly allocated doubly linked list.
   
*/

#include <stdio.h>
#include <stdlib.h>
#include "dbll.h"

typedef struct{
  int val_0;
  int val_1;
} two_int_t;

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
   Prints sum of values in two_int_t structures in a doubly linked list .
*/
void two_int_ll_print(dbll_node_t *head){
  if (head != NULL){
    two_int_t *s = (two_int_t *)(head->elt);
    printf("%d ", s->val_0 + s->val_1);
    two_int_ll_print(head->next);
  } else {
    printf("\n");
  }
}

int main(){
  dbll_node_t **head;
  int num_nodes = 10;
  int int_elt;
  two_int_t two_int_elt;
  dbll_init(head);
  for (int i = 0; i < num_nodes; i++){
    int_elt = i;
    dbll_insert(head, &int_elt, sizeof(int));
  }
  printf("Linked list of ints:\n");
  int_ll_print(*head);
  dbll_free(head);
  for (int i = 0; i < num_nodes; i++){
    two_int_elt.val_0 = i;
    two_int_elt.val_1 = i;
    dbll_insert(head, &two_int_elt, sizeof(two_int_t));
  }
  printf("Linked list of two_int_t structs:\n");
  two_int_ll_print(*head);
  dbll_free(head);
  return 0;
}
