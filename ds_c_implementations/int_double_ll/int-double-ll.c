/**
   int-double-ll.c

   Imlementation of a dynamicaly allocated non-generic doubly linked list
   of integer entries.
   
*/

#include <stdio.h>
#include <stdlib.h>
#include "int-double-ll.h"

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
  if (head == NULL){
    return;
  } else if (head->next != NULL){
    free_ll(head->next);
  } else if (head->prev != NULL){
    head->prev->next = NULL;
    node_t *n = head->prev;
    free(head);
    free_ll(n);
  } else {
    free(head);
  }
}

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
