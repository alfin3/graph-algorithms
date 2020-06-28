/**
   dll.c

   Imlementation of a generic dynamically allocated doubly linked list.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the doubly linked list form.

   A unique bit pattern can be included, if not already present, in an
   element for hashing applications to provide for fast membership testing 
   and access to nodes for operations such as predecessor- and successor-
   relation-based operations that are not implemented here. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dll.h"

/**
   Initializes a doubly linked list.
*/
void dll_init(dll_node_t **head){
  *head = NULL;
}

/**
   Inserts a node containing an element in a doubly linked list.
*/
void dll_insert(dll_node_t **head, void *elt, int elt_size){
  dll_node_t *n = malloc(sizeof(dll_node_t));
  assert(n != NULL);
  n->elt = malloc(elt_size);
  assert(n->elt != NULL);
  n->elt_size = elt_size;
  memcpy(n->elt, elt, n->elt_size);
  if (*head == NULL){
    n->next = NULL;
    n->prev = NULL;
  }else{
    n->next = *head;
    n->next->prev = n;
    n->prev = NULL;
  }
  *head = n;
}

/**
   Frees a doubly linked list. 
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in dll_insert, 
                then the element is fully copied into the block pointed to by 
                elt in a node, and a NULL as free_elt_fn is sufficient to 
                free the doubly linked list;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in dll_insert, then the
                pointer to the element is copied into the block pointed to by
                elt in a node, and an element-specific free_elt_fn is 
                necessary to free the doubly linked list.
*/
static void dll_free_helper(dll_node_t *node, void (*elt_free_fn)(void *));

void dll_free(dll_node_t **head, void (*elt_free_fn)(void *)){
  if (*head == NULL){return;}
  dll_node_t *cur_node = *head;
  while(cur_node->next != NULL){
    dll_free_helper(cur_node, elt_free_fn);
    cur_node = cur_node->next;
  }
  dll_free_helper(cur_node, elt_free_fn);
  free(cur_node);
  *head = NULL;  
}

static void dll_free_helper(dll_node_t *node, void (*elt_free_fn)(void *)){
  if (elt_free_fn != NULL){
    elt_free_fn(node->elt);
    node->elt = NULL;
  }else{
    free(node->elt);
    node->elt = NULL;
  }
  if (node->prev != NULL){free(node->prev);}
}

  
