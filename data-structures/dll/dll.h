/**
   dll.h

   Struct declarations and declarations of accessible functions of a generic
   dynamically allocated doubly linked list in a circular representation.

   Given the circular representation of the list, the head pointer in the
   provided list operations is not limited to a fixed position in the list.
   The head pointer determines the "beginning" and "end" of a list each time
   with respect to a call to an operation and can be used as a positional
   pointer for searching and modifying the list from and at any position,
   including a fixed position if desired.
   
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

#ifndef DLL_H  
#define DLL_H

typedef struct dll_node{
  size_t key_size;
  size_t elt_size;
  void *key; //can be NULL
  void *elt; //non-NULL
  struct dll_node *next;
  struct dll_node *prev;
} dll_node_t;

/**
   Initializes an empty doubly linked list by setting a head pointer to NULL.
   head        : pointer to a preallocated block of the size of a head pointer
*/
void dll_init(dll_node_t **head);

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
		 size_t elt_size);

/**
   Appends a node relative to a head pointer. A head pointer is NULL if
   the list is empty, or points to any node in the list to determine the
   position for an append operation. Please see the parameter specification
   in dll_prepend.
*/
void dll_append(dll_node_t **head,
		const void *key,
		const void *elt,
		size_t key_size,
		size_t elt_size);

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
			   int (*cmp_key)(const void *, const void *));

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
			   int (*cmp_elt)(const void *, const void *));

/**
   Deletes a node in a doubly linked list.
   head        : pointer to a head pointer to an initialized list
   node        : pointer to a node in an initialized list; if the pointer
                 points to the node pointed to by the head pointer, then the
                 head pointer is set to point to the next node from the
                 deleted node, or to NULL if the last node is deleted
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
		void (*free_elt)(void *));

/**
   Frees a doubly linked list. Please see the parameter specification in
   dll_delete.
*/
void dll_free(dll_node_t **head, void (*free_elt)(void *));

#endif
