/**
   dll.h

   Struct declarations and declarations of accessible functions of a generic
   dynamically allocated doubly linked list. 

   A node contains i) a pointer to a key that is an object within a 
   contininuous memory block (e.g. basic type, array, or struct), and ii) a 
   pointer to an element that is either an object within a continuous memory 
   block or a pointer to a multilayered object.

   The provided implementation facilitates hashing applications, such as
   mapping a key to a node pointer for fast in-list access to a node, 
   and chaining the colliding hash keys and their elements in a hash table.
*/

#ifndef DLL_H  
#define DLL_H

typedef struct dll_node{
  int key_size;
  int elt_size;
  void *key;
  void *elt;
  struct dll_node *next;
  struct dll_node *prev;
} dll_node_t;

/**
   Initializes a doubly linked list.
*/
void dll_init(dll_node_t **head);

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
		int elt_size);

/**
   Returns a pointer to the first node with a key that satisfies cmp_key_fn, 
   or NULL if such a node in not found.
   cmp_key_fn: 0 if two keys are equal.
*/
dll_node_t *dll_search_key(dll_node_t **head,
			   void *key,
			   int (*cmp_key_fn)(void *, void *));

/**
   Returns a pointer to the first node with an element that satisfies 
   cmp_elt_fn, or NULL if such a node is not found.
   cmp_elt_fn: 0 if two elements are equal.
*/
dll_node_t *dll_search_elt(dll_node_t **head,
			   void *elt,
			   int (*cmp_elt_fn)(void *, void *));

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
		void (*free_elt_fn)(void *));

/**
   Frees a doubly linked list.
*/
void dll_free(dll_node_t **head, void (*free_elt_fn)(void *));

#endif
