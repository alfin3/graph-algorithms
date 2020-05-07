/**
   double-ll.c

   Imlementation of a dynamicaly allocated doubly linked list.
*/

#include <stdio.h>
#include <stdlib.h>

typedef struct node{
  int data;
  struct node *next;
  struct node *prev;
} node_t;

/** Main functions */

/**
   Inserts a new node in a dynamically allocated doubly linked list.
*/
void insert(node_t **head, int data){
  node_t *n = (node_t *)malloc(sizeof(node_t));
  n->data = data;
  if (*head == NULL){
    n->next = NULL;
    n->prev = NULL;
  } else {
    n->next = *head;
    (*head)->prev = n;
  }
  *head = n;
}

/**
   Frees a dynamically allocated doubly linked list.
*/
void free_ll(node_t *head){
  if (head == NULL){return;}
  else if (head->next != NULL){
    free_ll(head->next);
  }
  else if (head->prev != NULL){
    head->prev->next = NULL;
    node_t *n = head->prev;
    free(head);
    free_ll(n);
  } else {
    free(head);
  }
}

/** Helper functions */

/**
   Prints a doubly linked list.
*/
void print_ll(node_t *head){
  if (head != NULL){
    printf("%d ", head->data);
    print_ll(head->next);
  } else {
    printf("\n");
  }
}

/** Use examples */

int main(){
  node_t *head = NULL;
  int num_nodes = 10;
  for (int i = 0; i < num_nodes; i++){
    insert(&head, i + 1);
  }
  printf("Before freeing.\n");
  print_ll(head);
  free_ll(head);
  printf("After freeing.\n");
  printf("%d \n", head->data);
  head = NULL;
  return 0;
}
