/**
   dbll.h

   Struct declarations and declarations of accessible functions of a generic
   dynamicaly allocated doubly linked list.
*/

#ifndef DBLL_H  
#define DBLL_H

typedef struct dbll_node{
  int elt_size;
  void *elt;
  struct dbll_node *next;
  struct dbll_node *prev;
} dbll_node_t;

/**
   Initializes a generic doubly linked list.
*/
void dbll_init(dbll_node_t **head);

/**
   Inserts a node in a generic doubly linked list.
*/
void dbll_insert(dbll_node_t **head, void *elt, int elt_size);

/**
   Frees a generic doubly linked list. elt_free_fn can be NULL if each 
   element does not point to additional blocks (e.g. basic type).
*/
void dbll_free(dbll_node_t **head, void (*elt_free_fn)(void *));

#endif
