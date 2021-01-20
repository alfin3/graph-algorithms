/**
   dll.c

   A generic dynamically allocated doubly linked list in a circular
   representation.

   Given the circular representation of the list, the head pointer in the
   provided list operations is not limited to and is not treated as a pointer
   to a fixed position in the list. The head pointer determines the
   "beginning" and "end" of a list each time with respect to a call to an
   operation and can be used as a positional pointer for searching and
   modifying the list from and at any position, including a fixed position
   if desired.
   
   A list node contains i) a pointer to a key that is an object within a 
   contiguous memory block (e.g. basic type, array, or struct), and ii) a 
   pointer to an element that is an object within a contiguous or
   noncontiguous memory block. 

   The node implementation facilitates hashing applications, such as
   mapping a key to a node pointer for fast in-list access and using a list
   for chaining hash keys and their elements in a hash table. In combination
   with the circular representation, the node implementation also facilitates
   the parallelization of search in future versions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dll.h"
#include "utilities-mem.h"

static void dll_free_key_elt(dll_node_t *node, void (*free_elt)(void *));

/**
   Initializes an empty doubly linked list by setting a head pointer to NULL.
   head        : pointer to a preallocated block of the size of a head pointer
*/
void dll_init(dll_node_t **head){
  *head = NULL;
}

/**
   Prepends a node relative to a head pointer. A head pointer is NULL if
   the list is empty, or points to any node in the list to determine the
   position for a prepend operation. 
   head        : pointer to a head pointer to an initialized list           
   key         : pointer to a key object of size key_size within a contiguous
                 memory block (e.g. basic type, array, struct), or NULL if 
                 there is no key
   elt         : - pointer to an element object of size elt_size, if the
                 element object is within a contiguous memory block,
                 - pointer to a pointer to an element object, if the element
                 object is within a noncontiguous memory block
   key_size    : size of a key object
   elt_size    : - size of an element object, if the element object is
                 within a contiguous memory block,
                 - size of a pointer to an element object, if the element
                 object is within a noncontiguous memory block
*/
void dll_prepend(dll_node_t **head,
		 const void *key,
		 const void *elt,
		 size_t key_size,
		 size_t elt_size){
  dll_node_t *node = malloc_perror(sizeof(dll_node_t));
  if (key != NULL){
    node->key = malloc_perror(key_size);
    node->key_size = key_size;
    memcpy(node->key, key, key_size);
  }else{
    node->key = NULL;
    node->key_size = key_size;
  }
  node->elt = malloc_perror(elt_size);
  node->elt_size = elt_size;
  memcpy(node->elt, elt, elt_size);
  if (*head == NULL){
    node->next = node;
    node->prev = node;
  }else{
    node->next = *head;
    node->prev = (*head)->prev;
    (*head)->prev->next = node;
    (*head)->prev = node;
  }
  *head = node;
}

/**
   Appends a node relative to a head pointer. A head pointer is NULL if
   the list is empty, or points to any node in the list to determine the
   position for an append operation. 
   head        : pointer to a head pointer to an initialized list           
   key         : pointer to a key object of size key_size within a contiguous
                 memory block (e.g. basic type, array, struct), or NULL if 
                 there is no key
   elt         : - pointer to an element object of size elt_size, if the
                 element object is within a contiguous memory block,
                 - pointer to a pointer to an element object, if the element
                 object is within a noncontiguous memory block
   key_size    : size of a key object
   elt_size    : - size of an element object, if the element object is
                 within a contiguous memory block,
                 - size of a pointer to an element object, if the element
                 object is within a noncontiguous memory block
*/
void dll_append(dll_node_t **head,
		const void *key,
		const void *elt,
		size_t key_size,
		size_t elt_size){
  dll_prepend(head, key, elt, key_size, elt_size);
  *head = (*head)->next;
}

/**
   Relative to a head pointer, returns a pointer to the first node with a
   key that satisfies cmp_key, or NULL if such a node in not found.
   head        : pointer to a head pointer to an initialized list
   key         : pointer to a key object of size key_size within a contiguous
                 memory block (e.g. basic type, array, struct)
   cmp_key     : comparison function which returns a zero integer value iff
                 the two key objects pointed to by the first and second
                 arguments are equal
*/
dll_node_t *dll_search_key(dll_node_t **head,
			   const void *key,
			   int (*cmp_key)(const void *, const void *)){
  dll_node_t *node = *head;
  if (node == NULL){
    return NULL;
  }else if (cmp_key(node->key, key) == 0){
    return node;
  }else{
    node = node->next;
    while(node != *head){
      if (cmp_key(node->key, key) == 0){
	return node;
      }
      node = node->next;
    }
  }
  return NULL;
}

/**
   Relative to a head pointer, returns a pointer to the first node with an
   element that satisfies cmp_elt, or NULL if such a node in not found.
   head        : pointer to a head pointer to an initialized list
   elt         : - pointer to an element object of size elt_size, if the
                 element object is within a contiguous memory block,
                 - pointer to a pointer to an element object, if the element
                 object is within a noncontiguous memory block
   cmp_elt     : comparison function which returns a zero integer value iff
                 the two element objects pointed to by the first and the
                 second arguments are equal
*/
dll_node_t *dll_search_elt(dll_node_t **head,
			   const void *elt,
			   int (*cmp_elt)(const void *, const void *)){
  dll_node_t *node = *head;
  if (node == NULL){
    return NULL;
  }else if (cmp_elt(node->elt, elt) == 0){
    return node;
  }else{
    node = node->next;
    while(node != *head){
      if (cmp_elt(node->elt, elt) == 0){
	return node;
      }
      node = node->next;
    }
  }
  return NULL;
}

/**
   Deletes a node in a doubly linked list. If the node pointer points to the
   node pointed to by the head pointer, then the head pointer is set to point
   to the next node from the deleted node, or NULL if the last node is
   deleted.
   head        : pointer to a head pointer to an initialized list
   free_elt    : - if an element is within a contiguous memory block,
                 as reflected by elt_size, and a pointer to the element is 
                 passed as elt in dll_prepend or dll_append, then the
                 element is fully copied into a node, and NULL as free_elt
                 is sufficient to delete the node,
                 - if an element is an object within a noncontiguous memory
                 block, and a pointer to a pointer to the element is passed
                 as elt in dll_prepend or dll_append, then the pointer to
                 the element is copied into a node, and an element-specific
                 free_elt, taking a pointer to a pointer to an element as its
                 parameter, is necessary to delete the node
*/
void dll_delete(dll_node_t **head,
		dll_node_t *node,
		void (*free_elt)(void *)){
  if (*head == NULL || node == NULL){
    return;
  }else if (node->prev == node && node->next == node){
    dll_free_key_elt(node, free_elt);
    *head = NULL;
    free(node);
    node = NULL;
  }else{
    //at least two nodes
    dll_free_key_elt(node, free_elt);
    node->next->prev = node->prev;
    node->prev->next = node->next;
    if (*head == node){
      *head = node->next;
    }
    free(node);
    node = NULL;
  }
}

static void dll_free_key_elt(dll_node_t *node, void (*free_elt)(void *)){
  free(node->key);
  node->key = NULL;
  if (free_elt != NULL){
    free_elt(node->elt);
  }
  free(node->elt);
  node->elt = NULL;
}

/**
   Frees a doubly linked list. Please see the parameter specification in
   dll_delete.
*/
void dll_free(dll_node_t **head, void (*free_elt)(void *)){
  dll_node_t *node = *head, *next_node = NULL;
  if (node != NULL){
    next_node = node->next;
    dll_free_key_elt(node, free_elt);
    free(node);
    node = next_node;
    while(node != *head){
      next_node = node->next;
      dll_free_key_elt(node, free_elt);
      free(node);
      node = next_node;
    }
    *head = NULL;
  }
}
