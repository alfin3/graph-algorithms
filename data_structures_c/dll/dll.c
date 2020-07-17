/**
   dll.c

   A generic dynamically allocated doubly linked list. 
   
   A node contains i) a pointer to a key that is an object within a 
   contininuous memory block (e.g. basic type, array, or struct), and ii) a 
   pointer to an element that is either an object within a continuous memory 
   block or a pointer to a multilayered object.

   The provided implementation facilitates hashing applications, such as
   mapping a key to a node pointer for fast in-list access to a node, 
   and chaining the colliding hash keys and their elements in a hash table.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dll.h"

static void dll_free_key_elt(dll_node_t *node, void (*free_elt_fn)(void *));

/**
   Initializes a doubly linked list.
*/
void dll_init(dll_node_t **head){
  *head = NULL;
}

/**
   Inserts a node at the beginning of a doubly linked list.
   key: a pointer to an object of size key_size within a contininuous memory 
        block (e.g. basic type, array, struct). Can be NULL.
   elt: see free_elt_fn description in dll_delete. Non-NULL.
*/
void dll_insert(dll_node_t **head,
		void *key,
		void *elt,
		int key_size,
		int elt_size){
  dll_node_t *node = malloc(sizeof(dll_node_t));
  assert(node != NULL);
  if (key != NULL){
    node->key = malloc(key_size);
    assert(node->key != NULL);
    node->key_size = key_size;
    memcpy(node->key, key, key_size);
  }else{
    node->key = NULL;
    node->key_size = key_size;
  }
  node->elt = malloc(elt_size);
  assert(node->elt != NULL);
  node->elt_size = elt_size;
  memcpy(node->elt, elt, elt_size);
  if (*head == NULL){
    node->next = NULL;
    node->prev = NULL;
  }else{
    node->next = *head;
    node->next->prev = node;
    node->prev = NULL;
  }
  *head = node;
}

/**
   Returns a pointer to the first node with a key that satisfies cmp_key_fn, 
   or NULL if such a node in not found.
   cmp_key_fn: 0 if two keys are equal.
*/
dll_node_t *dll_search_key(dll_node_t **head,
			   void *key,
			   int (*cmp_key_fn)(void *, void *)){
  dll_node_t *cur_node = *head;
  while(cur_node != NULL){
    if (cmp_key_fn(cur_node->key, key) == 0){
      return cur_node;
    }else{
      cur_node = cur_node->next;
    }
  }
  return NULL;
}

/**
   Returns a pointer to the first node with an element that satisfies 
   cmp_elt_fn, or NULL if such a node is not found.
   cmp_elt_fn: 0 if two elements are equal.
*/
dll_node_t *dll_search_elt(dll_node_t **head,
			   void *elt,
			   int (*cmp_elt_fn)(void *, void *)){
  dll_node_t *cur_node = *head;
  while(cur_node != NULL){
    if (cmp_elt_fn(cur_node->elt, elt) == 0){
      return cur_node;
    }else{
      cur_node = cur_node->next;
    }
  }
  return NULL;
}

/**
   Deletes a node in a doubly linked list.
   free_elt_fn: - if an element is of a basic type or is an array or struct 
                within a continuous memory block, as reflected by elt_size, 
                and a pointer to the element is passed as elt in dll_insert, 
                then the element is fully copied into the block pointed to by 
                elt in a node, and a NULL as free_elt_fn is sufficient to 
                delete a node;
                - if an element is multilayered, and a pointer to a pointer
                to the element is passed as elt in dll_insert, then the
                pointer to the element is copied into the block pointed to by
                elt in a node, and an element-specific free_elt_fn is 
                necessary to delete a node. 
*/
void dll_delete(dll_node_t **head,
		dll_node_t *node,
		void (*free_elt_fn)(void *)){
  if (*head == NULL || node == NULL){
    return;
  }else if (node->prev == NULL && node->next == NULL){
    dll_free_key_elt(node, free_elt_fn);
    *head = NULL;
    free(node);
  }else if (node->prev == NULL){
    dll_free_key_elt(node, free_elt_fn);
    node->next->prev = NULL;
    *head = node->next;
    free(node);
  }else if (node->next == NULL){
    dll_free_key_elt(node, free_elt_fn);
    node->prev->next = NULL;
    free(node);
  }else{
    dll_free_key_elt(node, free_elt_fn);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
  }
}

static void dll_free_key_elt(dll_node_t *node, void (*free_elt_fn)(void *)){
  free(node->key); //within a continuous memory block
  node->key = NULL;
  if (free_elt_fn != NULL){
    free_elt_fn(node->elt);
    node->elt = NULL;
  }else{
    free(node->elt);
    node->elt = NULL;
  }
}

/**
   Frees a doubly linked list.
*/
void dll_free(dll_node_t **head, void (*free_elt_fn)(void *)){
  dll_node_t *cur_node = *head;
  dll_node_t *next_node = NULL;
  while(cur_node != NULL){
    next_node = cur_node->next;
    dll_free_key_elt(cur_node, free_elt_fn);
    free(cur_node);
    cur_node = next_node;
  }
  *head = NULL;  
}
  
