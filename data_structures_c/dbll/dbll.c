/**
   dbll.c

   Imlementation of a generic dynamically allocated doubly linked list.
   
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
   element does not point to additional memory blocks (e.g. basic type).
*/
static void dbll_free_helper(dbll_node_t *node, void (*elt_free_fn)(void *));
static void elt_free_helper(void *elt, void (*elt_free_fn)(void *));

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
    elt_free_helper(n->next->elt, elt_free_fn);
    free(n->next);
    n->next = NULL;
    dbll_free_helper(n, elt_free_fn);
  } else {
    elt_free_helper(node->elt, elt_free_fn);
    free(node);
    node = NULL;
  }
}

static void elt_free_helper(void *elt, void (*elt_free_fn)(void *)){
  if (elt_free_fn != NULL){
    elt_free_fn(elt);
  } else {
    free(elt);
  }
}
  
