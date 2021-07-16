/**
   dll.c

   A generic dynamically allocated doubly linked list in a circular
   representation.

   Given the circular representation of the list, the head pointer in the
   provided list operations is not limited to a fixed position in the list.
   The head pointer determines the "beginning" and "end" of a list each time
   with respect to a call to an operation and can be used as a positional
   pointer for searching and modifying the list from and at any position,
   including a fixed position if desired.
   
   A list node contains i) a dll_node_t struct for pointer operations, ii)
   a contiguous key and iii) an element or a pointer to an element. A key is
   an object within a contiguous memory block (e.g. basic type, array, or
   struct). If cmp_key is NULL it is treated as an array of bytes. An
   element is contiguous or non-contiguous. Given a char *p pointer to a
   node, its key is at p + sizeof(dll_node_t) and its element/element pointer
   is at p + sizeof(key_elt_t) + key_size. Access is simplified by the
   dll_ptr function.

   The implementation provides a guarantee that a block with a dll_node_t
   struct, a key, and an element/element pointer keeps its address in memory
   throughout its lifetime in a list. The implementation may not be slower
   (as tested) than a singly linked list due to instruction-level
   parallelism.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.

   The node implementation reduces memory footprint and facilitates hashing
   applications, such as mapping a key to a node pointer for fast in-list
   access and using a list for chaining hash keys and their elements in a
   hash table. In combination with the circular representation, the node
   implementation also facilitates the parallelization of search.

   The implementation does not use stdint.h, and is portable under C89/C90
   and C99.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dll.h"
#include "utilities-mem.h"

/**
   Initializes an empty doubly linked list by setting a head pointer to NULL.
   head        : pointer to a preallocated block of size of a head pointer
*/
void dll_init(dll_node_t **head){
  *head = NULL;
}

/**
   Creates and prepends a node relative to a head pointer. A head pointer is
   NULL if the list is empty, or points to any node in the list to determine
   the position for the prepend operation.
   head        : pointer to a head pointer to an initialized list           
   key         : non-NULL pointer to a key object of size key_size within a
                 contiguous memory block (e.g. basic type, array, struct);
                 if cmp_key is NULL it is treated as an array of bytes
   elt         : - non-NULL pointer to a block of size elt_size that
                 is an element, if the element is contiguous, or pointer to
                 an element, if the element is noncontiguous or a pointer to
                 a contiguous element is prepended
   key_size    : non-zero size of a key object
   elt_size    : - non-zero size of an element, if the element is within a
                 contiguous memory block and a copy of the element is
                 prepended,
                 - size of a pointer to an element, if the element
                 is within a noncontiguous memory block or a pointer to a
                 contiguous element is prepended
*/
void dll_prepend_new(dll_node_t **head,
		     const void *key,
		     const void *elt,
		     size_t key_size,
		     size_t elt_size){
  /* allocate a single block to reduce admin and alignment bytes */
  dll_node_t *node =
    malloc_perror(1, add_sz_perror(sizeof(dll_node_t),
				   add_sz_perror(key_size, elt_size)));
  memcpy(dll_ptr(node, 0), key, key_size);
  memcpy(dll_ptr(node, key_size), elt, elt_size);
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
   Creates and appends a node relative to a head pointer. Please see the
   parameter specification in dll_prepend_new.
*/
void dll_append_new(dll_node_t **head,
		    const void *key,
		    const void *elt,
		    size_t key_size,
		    size_t elt_size){
  dll_prepend_new(head, key, elt, key_size, elt_size);
  *head = (*head)->next;
}

/**
   Prepends a node relative to a head pointer. A head pointer is NULL if
   the list is empty, or points to any node in the list to determine the
   position for a prepend operation.
   head        : pointer to a head pointer to an initialized list           
   node        : non-NULL pointer to a node to be prepended
*/
void dll_prepend(dll_node_t **head, dll_node_t *node){
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
   Appends a node relative to a head pointer. Please see the parameter
   specification in dll_prepend.
*/
void dll_append(dll_node_t **head, dll_node_t *node){
  dll_prepend(head, node);
  *head = (*head)->next;
}

/**
   Returns a pointer to the key of a node (size = 0) or the element 
   of a node (size = key_size).
   node        : non-NULL pointer to a node
*/
void *dll_ptr(const dll_node_t *node, size_t size){
  return (char *)node + sizeof(dll_node_t) + size;
}

/**
   Relative to a head pointer, returns a pointer to the clockwise (next)
   first node with a key that has the same bit pattern as the block pointed
   to by key, or NULL if such a node in not found. Temporarily modifies a
   node to mark the end of the list during search.
   head        : pointer to a head pointer to an initialized list
   key         : non-NULL pointer to a key object of size key_size within a
                 contiguous memory block; if cmp_key is NULL it is treated
                 as an array of bytes
   key_size    : non-zero size of a key object in bytes
*/
dll_node_t *dll_search_key(dll_node_t * const *head,
			   const void *key,
			   size_t key_size){
  const dll_node_t *node = *head;
  if (node == NULL) return NULL;
  /* NULL marker to avoid undef. behavior of pointer comparison */
  (*head)->prev->next = NULL;
  while(node != NULL){
    if (memcmp(dll_ptr(node, 0), key, key_size) == 0){
      (*head)->prev->next = *head;
      return (dll_node_t *)node;
    }
    node = node->next;
  }
  (*head)->prev->next = *head;
  return NULL;
}

/**
   Relative to a head pointer, returns a pointer to the clockwise (next)
   first node with a key that has the same bit pattern as the block pointed
   to by key, or NULL if such a node in not found. Assumes that every key
   in a list is unique. The list is no modified during the operation which
   does not require thread synchronization overhead for parallel search
   queries.
   head        : pointer to a head pointer to an initialized list
   key         : non-NULL pointer to a key object of size key_size within a
                 contiguous memory block; if cmp_key is NULL it is treated
                 as an array of bytes
   key_size    : non-zero size of a key object in bytes
*/
dll_node_t *dll_search_uq_key(dll_node_t * const *head,
			      const void *key,
			      size_t key_size){
  const void *last_key = NULL;
  const dll_node_t *node = *head;
  if (node == NULL) return NULL;
  /* last key as marker to avoid undef. behavior of pointer comparison */
  last_key = dll_ptr((*head)->prev, 0);
  while(memcmp(dll_ptr(node, 0), last_key, key_size) != 0){
    if (memcmp(dll_ptr(node, 0), key, key_size) == 0){
      return (dll_node_t *)node;
    }
    node = node->next;
  }
  /* compare last key */
  if (memcmp(dll_ptr(node, 0), key, key_size) == 0){
      return (dll_node_t *)node;
  }
  return NULL;
}

/**
   Relative to a head pointer, returns a pointer to the clockwise (next)
   first node with an element that satisfies cmp_elt, or NULL if such a node
   in not found. Temporarily modifies a node to mark the end of the list
   during search.
   head        : pointer to a head pointer to an initialized list
   elt         : - non-NULL pointer to a block of size elt_size that
                 is an element, if the element is contiguous, or pointer to
                 an element, if the element is noncontiguous or a pointer to
                 a contiguous element was prepended or appended
   cmp_elt     : comparison function which returns a zero integer value iff
                 the two elements accessed through the first and the second 
                 arguments are equal; each argument is a pointer to an
                 elt_size block that stores an element or a pointer to an
                 element (see specification of dll_prepend_new)
                 
*/
dll_node_t *dll_search_elt(dll_node_t * const *head,
			   const void *elt,
			   size_t key_size,
			   int (*cmp_elt)(const void *, const void *)){
  const dll_node_t *node = *head;
  if (node == NULL) return NULL;
  /* NULL marker to avoid undef. behavior of pointer comparison */
  (*head)->prev->next = NULL;
  while(node != NULL){
    if (cmp_elt(dll_ptr(node, key_size), elt) == 0){
      (*head)->prev->next = *head;
      return (dll_node_t *)node;
    }
    node = node->next;
  }
  (*head)->prev->next = *head;
  return NULL;
}

/**
   Removes a node in a doubly linked list.
   head        : pointer to a head pointer to an initialized list
   node        : pointer to a node in an initialized list; if the pointer
                 points to the node pointed to by the head pointer, then the
                 head pointer is set to point to the next node from the
                 removed node, or to NULL if the last node is removed
*/
void dll_remove(dll_node_t **head, const dll_node_t *node){
  int is_same_node = 0;
  if (*head == NULL || node == NULL) return;
  /* NULL marker to avoid undef. behavior of pointer comparison */
  (*head)->prev->next = NULL;
  is_same_node = (node->prev->next == NULL);
  if (is_same_node && node->next == NULL){
    /* one node */
    (*head)->prev->next = *head;
    *head = NULL;
  }else{
    (*head)->prev->next = *head;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    if (is_same_node) *head = node->next;
  }
}

/**
   Deletes a node in a doubly linked list.
   head        : pointer to a head pointer to an initialized list
   node        : pointer to a node in an initialized list; if the pointer
                 points to the node pointed to by the head pointer, then the
                 head pointer is set to point to the next node from the
                 deleted node, or to NULL if the last node is deleted
   free_elt    : - if an element is within a contiguous memory block and
                 a copy of the element was prepended or appended, then NULL
                 as free_elt is sufficient to delete the node,
                 - if an element is within a noncontiguous memory block or
                 a pointer to a contiguous element was prepended or appended,
                 then an element-specific free_elt, taking a pointer to a
                 pointer to an element as its argument and leaving a block of
                 size elt_size pointed to by the argument, is necessary to
                 delete the node
*/
void dll_delete(dll_node_t **head,
		dll_node_t *node,
		size_t key_size,
		void (*free_elt)(void *)){
  int is_same_node = 0;
  if (*head == NULL || node == NULL) return;
  /* NULL marker to avoid undef. behavior of pointer comparison */
  (*head)->prev->next = NULL;
  is_same_node = (node->prev->next == NULL);
  if (is_same_node && node->next == NULL){
    /* one node */
    if (free_elt != NULL) free_elt(dll_ptr(node, key_size));
    *head = NULL;
    free(node);
    node = NULL;
  }else{
    if (free_elt != NULL) free_elt(dll_ptr(node, key_size));
    (*head)->prev->next = *head;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    if (is_same_node) *head = node->next;
    free(node);
    node = NULL;
  }
}

/**
   Frees a doubly linked list. Please see the parameter specification in
   dll_delete.
*/
void dll_free(dll_node_t **head,
	      size_t key_size,
	      void (*free_elt)(void *)){
  dll_node_t *node = *head, *next_node = NULL;
  if (node != NULL) (*head)->prev->next = NULL;
  while(node != NULL){
    next_node = node->next;
    if (free_elt != NULL) free_elt(dll_ptr(node, key_size));
    free(node);
    node = next_node;
  }
  *head = NULL;
}
