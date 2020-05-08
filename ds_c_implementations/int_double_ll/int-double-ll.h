/**
   int-double-ll.h

   Struct declarations and declarations of accessible functions of a dynamicaly
   allocated non-generic doubly linked list of integer entries.
*/

#ifndef INT_DOUBLE_LL_H  
#define INT_DOUBLE_LL_H

typedef struct node{
  int data; 
  struct node *next;
  struct node *prev;
} node_t;

/**
   Inserts a new node in a dynamically allocated doubly linked list.
*/
void insert(node_t **head, int data);

/**
   Frees a dynamically allocated doubly linked list.
*/
void free_ll(node_t *head);

/**
   Prints a doubly linked list.
*/
void print_ll(node_t *head);

#endif
