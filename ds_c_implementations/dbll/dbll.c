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
   Frees a generic doubly linked list. elt_free_fn can be NULL if each 
   element does not point to additional blocks (e.g. basic type).
*/
static void dbll_free_helper(dbll_node_t *node, void (*elt_free_fn)(void *));

void dbll_free(dbll_node_t **head, void (*elt_free_fn)(void *)){
  if (*head == NULL){return;}
  dbll_free_helper(*head, elt_free_fn);
  *head = NULL;  
}

static void dbll_free_helper(dbll_node_t *node, void (*elt_free_fn)(void *)){
  if (node->next != NULL){
    dbll_free_helper(node->next, elt_free_fn);
  } else if (node->prev != NULL){
    dbll_node_t *n = node->prev;
    if (elt_free_fn != NULL){
      elt_free_fn(n->next->elt);
    } else {
      free(n->next->elt);
    }
    free(n->next);
    n->next = NULL;
    dbll_free_helper(n, elt_free_fn);
  } else {
    if (elt_free_fn != NULL){
      elt_free_fn(node->elt);
    } else {
      free(node->elt);
    }
    free(node);
    node = NULL;
  }
}
