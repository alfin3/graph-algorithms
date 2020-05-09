/**
   dbll.c

   Imlementation of a generic dynamicaly allocated doubly linked list.
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dbll.h"

/**
   Initializes a generic doubly linked list.
*/
void dbll_init(dbll_node_t **head){
  *head = NULL;
}

/**
   Inserts a node in a generic doubly linked list.
*/
void dbll_insert(dbll_node_t **head, void *elt, int elt_size){
  dbll_node_t *n = (dbll_node_t *)malloc(sizeof(dbll_node_t));
  assert(n != NULL);
  n->elt = malloc(elt_size);
  assert(n->elt != NULL);
  n->elt_size = elt_size;
  memcpy(n->elt, elt, n->elt_size);
  if (*head == NULL){
    n->next = NULL;
    n->prev = NULL;
  } else {
    n->next = *head;
    (*head)->prev = n;
    n->prev = NULL;
  }
  *head = n;
}

/**
   Frees a generic doubly linked list.
*/
static void dbll_free_helper(dbll_node_t *node);

void dbll_free(dbll_node_t **head){
  if (*head == NULL){return;}
  dbll_free_helper(*head);
  *head = NULL;  
}

static void dbll_free_helper(dbll_node_t *node){
  if (node->next != NULL){
    dbll_free_helper(node->next);
  } else if (node->prev != NULL){
    dbll_node_t *n = node->prev;
    free(n->next->elt);
    free(n->next);
    n->next = NULL;
    dbll_free_helper(n);
  } else {
    free(node->elt);
    free(node);
  }
}
