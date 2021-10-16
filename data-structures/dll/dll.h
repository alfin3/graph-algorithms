/**
   dll.h

   Struct declarations and declarations of accessible functions of a doubly
   linked list with cache-efficient allocation of nodes with two type-
   generic data blocks. The list is in a circular representation.

   Given the circular representation of the list, the head pointer in the
   provided list operations is not limited to a fixed position in the list.
   The head pointer determines the "beginning" and "end" of a list each time
   with respect to a call to an operation and can be used as a positional
   pointer for searching and modifying the list from and at any position,
   including a fixed position if desired.

   A list node contains i) a malloc-aligned generic block of size key_size
   (referred to as "key_size block"), ii) a dll_node_t struct for pointer
   operations, and iii) an optionally aligned generic block of size elt_size
   (referred to as "elt_size block"), all within a single allocated block
   for cache-efficiency and to reduce administrative bytes.

   A distinction is made between a key and a key_size block, and an element
   and an elt_size block. Given a key, which may be within a contiguous or a
   non-contiguous block of memory, the user decides what is copied into the
   key_size block of a new node. If the key is within a contiguous memory
   block, then it can be entirely copied as a key_size block, or a pointer
   to it can be copied as a key_size block. If the key is within a non-
   contiguous memory block, then a pointer to it is copied as a key_size
   block. The same applies to an element.

   When a pointer to a key is copied into a node as a key_size block,
   the user can also decide if only the pointer or the entire key is deleted
   during the delete and free operations. By using NULL as free_key, only
   the pointer is deleted. Otherwise, the deletion is performed according to
   a non-NULL free_key. For example, when an in-memory set of images are
   used as keys and pointers are copied into the nodes of a list, then
   using NULL as free_key will not affect the original set of images.
   The same applies to elements and free_elt.

   The implementation provides a guarantee that a key_size block, a
   dll_node_t struct, and an elt_size block belonging to the same node keep
   their addresses in memory throughout the lifetime of the node in a list.
   The implementation may not be slower (as tested) than a singly linked
   list due to instruction-level parallelism.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.

   The node implementation facilitates type-generic hashing applications,
   such as mapping a key to a node pointer for fast in-list access and
   using a list for chaining hash keys and their elements in a hash table.
   In combination with the circular representation, the implementation
   also facilitates the parallelization of search.

   The implementation does not use stdint.h, and is portable under C89/C90
   and C99.
*/

#ifndef DLL_H  
#define DLL_H

typedef struct{
  size_t key_offset; /* subtracted */
  size_t elt_offset; /* added */
} dll_t; /* given char *p pointer to a dll_node_t,
            p - key_offset points to key_size block and 
            p + elt_offset points to elt_size block;
            see dll_key_ptr and dll_elt_ptr functions */

typedef struct dll_node{
  struct dll_node *next;
  struct dll_node *prev;
} dll_node_t;

/**
   Initializes an empty doubly linked list by setting a head pointer to NULL
   and key_offset and elt_offset in a dll_t struct to values according to
   the memory alignment requirements for the key_size and elt_size blocks of
   a node. An in-list key_size block can be accessed with a pointer to any
   type with which a malloc'ed block can be accessed. An in-list elt_size
   block is guaranteed to be accessible only with a pointer to a character
   (e.g. for memcpy), unless additional alignment is performed by calling
   dll_align_elt.
   ll          : pointer to a preallocated block of size of a dll_t struct
   head        : pointer to a preallocated block of size of a dll_node_t
                 pointer (head pointer)
   key_size    : non-zero size of a key_size block; must account for internal
                 and trailing padding according to sizeof
*/
void dll_init(dll_t *ll,
	      dll_node_t **head,
	      size_t key_size);

/**
   Aligns each in-list elt_size block to be accessible with a pointer to a 
   type T other than character (in addition to a character pointer). If
   alignment requirement of T is unknown, the size of T can be used
   as a value of the alignment parameter because size of T >= alignment
   requirement of T (due to structure of arrays), which may result in
   overalignment. The list keeps the effective type of a copied
   elt_size block, if it had one at the time of insertion, and T must
   be compatible with the type to comply with the strict aliasing rules.
   T can be the same or a cvr-qualified/signed/unsigned version of the
   type. The operation is optionally called after dll_init is
   completed and before any other operation is called.
   ll          : pointer to an initialized dll_t struct
   alignment   : alignment requirement or size of the type, a pointer to
                 which is used to access the elt_size block of a node; if
                 size, must account for internal and trailing padding
                 according to sizeof
*/
void dll_align_elt(dll_t *ll, size_t alignment);

/**
   Creates and prepends a node relative to a head pointer. A head pointer is
   NULL if the list is empty, or points to any node in the list to determine
   the position for the prepend operation.
   ll          : pointer to an initialized dll_t struct
   head        : pointer to a head pointer to an initialized list           
   key         : non-NULL pointer to the key_size block of a key
   elt         : non-NULL pointer to the elt_size block of an element
   key_size    : non-zero size of a key_size block; must account for internal
                 and trailing padding according to sizeof
   elt_size    : non-zero size of an elt_size block; must account for
                 internal and trailing padding according to sizeof
*/
void dll_prepend_new(const dll_t *ll,
		     dll_node_t **head,
		     const void *key,
		     const void *elt,
		     size_t key_size,
		     size_t elt_size);

/**
   Creates and appends a node relative to a head pointer. Please see the
   parameter specification in dll_prepend_new.
*/
void dll_append_new(const dll_t *ll,
		    dll_node_t **head,
		    const void *key,
		    const void *elt,
		    size_t key_size,
		    size_t elt_size);

/**
   Prepends a node relative to a head pointer. A head pointer is NULL if
   the list is empty, or points to any node in the list to determine the
   position for the prepend operation.
   head        : pointer to a head pointer to an initialized list           
   node        : non-NULL pointer to a node to be prepended
*/
void dll_prepend(dll_node_t **head, dll_node_t *node);

/**
   Appends a node relative to a head pointer. Please see the parameter
   specification in dll_prepend.
*/
void dll_append(dll_node_t **head, dll_node_t *node);

/**
   Returns a pointer to the key_size block of a node.
*/
void *dll_key_ptr(const dll_t *ll, const dll_node_t *node);

/**
   Returns a pointer to the elt_size block of a node.
*/
void *dll_elt_ptr(const dll_t *ll, const dll_node_t *node);

/**
   Relative to a head pointer, returns a pointer to the clockwise (next)
   first node with a key that equals the key pointed to by the key parameter
   according to cmp_key, or NULL if such a node in not found. Temporarily
   modifies a node in the list to mark the end of the list during search.
   ll          : pointer to an initialized dll_t struct
   head        : pointer to a head pointer to an initialized list
   key         : non-NULL pointer to the key_size block of a key
   key_size    : non-zero size of a key_size block; must account for internal
                 and trailing padding according to sizeof
   cmp_key     : - if NULL then a default memcmp-based comparison of key_size
                 blocks is performed
                 - otherwise comparison function is applied which returns a
                 zero integer value iff the two keys accessed through the
                 first and the second arguments are equal; each argument is
                 a pointer to the key_size block of a key
*/
dll_node_t *dll_search_key(const dll_t *ll,
			   dll_node_t * const *head,
			   const void *key,
			   size_t key_size,
			   int (*cmp_key)(const void *, const void *));

/**
   Relative to a head pointer, returns a pointer to the clockwise (next)
   first node with a key that equals the key pointed to by the key parameter
   according to cmp_key, or NULL if such a node in not found. Assumes that
   every key in a list is unique according to cmp_key. The list is not 
   modified during the operation which enables parallel queries without 
   thread synchronization overhead. Please see the parameter specification
   in dll_search_key.
*/
dll_node_t *dll_search_uq_key(const dll_t *ll,
			      dll_node_t * const *head,
			      const void *key,
			      size_t key_size,
			      int (*cmp_key)(const void *, const void *));

/**
   Removes a node in a doubly linked list.
   head        : pointer to a head pointer to an initialized list
   node        : pointer to a node in an initialized list; if the pointer
                 points to the node pointed to by the head pointer, then the
                 head pointer is set to point to the next node from the
                 removed node, or to NULL if the last node is removed
*/
void dll_remove(dll_node_t **head, const dll_node_t *node);

/**
   Deletes a node in a doubly linked list.
   ll          : pointer to an initialized dll_t struct
   head        : pointer to a head pointer to an initialized list
   node        : pointer to a node in an initialized list; if the pointer
                 points to the node pointed to by the head pointer, then the
                 head pointer is set to point to the next node from the
                 deleted node, or to NULL if the last node is deleted
   free_key    : - NULL if only a key_size block should be deleted (e.g.
                 because a key was entirely copied as a key_size block, or
                 because a pointer was copied as a key_size block and only 
                 the pointer should be deleted)
                 - otherwise takes a pointer to the key_size block of a key
                 as an argument, frees the memory of the key except the
                 key_size block pointed to by the argument
   free_elt   : - NULL if only an elt_size block should be deleted (e.g.
                 because an element was entirely copied as an elt_size
                 block, or because a pointer was copied as an elt_size
                 block and only the pointer should be deleted)
                 - otherwise takes a pointer to the elt_size block of an
                 element as an argument, frees the memory of the element
                 except the elt_size block pointed to by the argument
*/
void dll_delete(const dll_t *ll,
		dll_node_t **head,
		dll_node_t *node,
		void (*free_key)(void *),
		void (*free_elt)(void *));

/**
   Frees the memory of all keys and elements that are in a list
   according to free_key and free_elt, frees the memory of the list,
   and leaves the block of size sizeof(dll_t) pointed to by the ll
   parameter. Please see the parameter specification in dll_delete.
*/
void dll_free(const dll_t *ll,
	      dll_node_t **head,
	      void (*free_key)(void *),
	      void (*free_elt)(void *));

#endif
