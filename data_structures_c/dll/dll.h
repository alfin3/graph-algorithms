/**
   dll.h

   Struct declarations and declarations of accessible functions of a generic
   dynamicaly allocated doubly linked list.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects in the doubly linked list form.

   A unique bit pattern can be included, if not already present, in an
   element for hashing applications to provide for fast membership testing 
   and access to nodes for operations such as predecessor- and successor-
   relation-based operations that are not implemented here.
*/

#ifndef DLL_H  
#define DLL_H

typedef struct dll_node{
  int elt_size;
  void *elt;
  struct dll_node *next;
  struct dll_node *prev;
}dll_node_t;

/**
   Initializes a doubly linked list.
*/
void dll_init(dll_node_t **head);

/**
   Inserts a node containing an element in a doubly linked list.
*/
void dll_insert(dll_node_t **head, void *elt, int elt_size);

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
void dll_free(dll_node_t **head, void (*elt_free_fn)(void *));

#endif
