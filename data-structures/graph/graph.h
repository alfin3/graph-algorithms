/**
   graph.h

   Struct declarations and declarations of accessible functions for 
   representing a graph with generic integer vertices and generic
   contiguous weights.

   Each list in an adjacency list is represented by a dynamically growing 
   stack. A vertex is of any integer type with values starting from 0 and
   is copied into an adjacency list as a "vt_size block". If a graph is
   weighted, an edge weight is an object within a contiguous memory block,
   such as an object of basic type (e.g. char, int, double) or a struct or
   an array, and is copied into the adjacency list as a "wt_size block".
  
   A single stack of vt_size and wt_size block pairs with adjustable
   alignment in memory is used to achieve cache efficiency in graph
   algorithms. Depending on the problem size and a given system, the
   choice of an integer type of a lower size for vertices may provide
   additional cache efficiency in addition to reducing the space
   requirements.

   The user-defined and predefined operations for reading and writing
   integer values into the vt_size blocks of vertices use size_t as the
   user interface. This design is portable because vertex values start
   from zero and are used as indices in the array of stacks in an
   adjacency list. As a consequence, a valid vertex value of any integer
   type cannot exceed the maximum value of size_t.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.

   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.

   TODO: add non-contiguous weights to store pointers to data such
   as images as graph weights
*/

#ifndef GRAPH_H  
#define GRAPH_H

#include <stdlib.h>
#include "stack.h"

typedef struct{
  size_t num_vts; 
  size_t num_es;
  size_t vt_size;
  size_t wt_size; /* 0 if a graph is not weighted */
  void *u;        /* u of (u, v) edges, NULL if no edges */
  void *v;        /* v of (u, v) edges, NULL if no edges */
  void *wts;      /* NULL if no edges or wt_size is 0 */
} graph_t;

typedef struct{
  size_t num_vts;
  size_t num_es;
  size_t vt_size;
  size_t wt_size;
  size_t pair_size; /* size of a vertex weight pair aligned in memory */
  size_t wt_offset; /* number of bytes from beginning of pair to weight */
  void *buf;        /* buffer that is only used by adj_lst_ functions */
  stack_t **vt_wts; /* stacks of vertex weight pairs, NULL if no vertices */
} adj_lst_t;

/**
   Initializes a weighted or unweighted graph with num_vts vertices
   and no edges, providing a basis for graph construction. Makes no
   allocations.
   g           : pointer to a preallocated block of size sizeof(graph_t)
   num_vts     : number of vertices
   vt_size     : non-zero size of the integer type used to represent a
                 vertex according to sizeof; equals to the size of the
                 vt_size block of a vertex
   wt_size     : - 0 if a graph is not weighted
                 - otherwise non-zero size of the wt_size block of a weight;
                 must account for internal and trailing padding according to
                 sizeof
*/
void graph_base_init(graph_t *g,
		     size_t num_vts,
		     size_t vt_size,
		     size_t wt_size);

/**
   Initializes an empty adjacency list according to a graph. The alignment
   of vt_size and wt_size blocks in the adjacency list is computed by
   default according to their sizes because size of a type T >= alignment
   requirement of T (due to the structure of arrays), which may result in
   overalignment.
   a           : pointer to a preallocated block of size sizeof(adj_lst_t)
   g           : pointer to the graph_t struct of a graph that was
                 initialized with at least graph_base_init
*/
void adj_lst_base_init(adj_lst_t *a, const graph_t *g);

/**
   Aligns the vt_size and wt_size blocks in an adjacency list according to 
   the values of the alignment parameters. If the alignment requirement
   of only one type is known, then the size of the other type can be used
   as a value of the other alignment parameter because size of
   type >= alignment requirement of type (due to structure of arrays), 
   which may result in overalignment. The call to this operation may 
   result in the reduction of space requirements as compared to
   adj_lst_base_init alone. The operation is optionally called after
   adj_lst_base_init is completed and before any other adj_list_ operation
   is called.
   a            : pointer to an adj_lst_t struct initialized with
                  adj_lst_base_init
   vt_alignment : alignment requirement or size of the type of the vt_size
                  block of a vertex, which is the representation of an
                  integer type value; if size, must account for padding
                  according to sizeof
   wt_alignment : alignment requirement or size of the type of the wt_size
                  block of a weight; if size, must account for internal and
                  trailing padding according to sizeof
*/
void adj_lst_align(adj_lst_t *a,
		   size_t vt_alignment,
		   size_t wt_alignment);

/**
   Builds the adjacency list of a directed graph. The adjacency list keeps
   the effective type of the copied vt_size blocks (with integer values)
   from the graph. The adjacency list also keeps the effective type of the
   copied wt_size blocks if the graph is weighted and the wt_size blocks
   have an effective type in the graph.
   a            : pointer to an adj_lst_t struct initialized with
                  adj_lst_base_init and optionally with adj_lst_align
   g            : pointer to the graph_t struct of a graph initialized with
                  at least graph_base_init
   read_vt      : reads the integer value in the vt_size block of a vertex
                  pointed to by the argument and returns a size_t value
*/
void adj_lst_dir_build(adj_lst_t *a,
		       const graph_t *g,
		       size_t (*read_vt)(const void *));

/**
   Builds the adjacency list of an undirected graph. The adjacency list
   keeps the effective type of the copied vt_size blocks (with integer
   values) from the graph. The adjacency list also keeps the effective
   type of the copied wt_size blocks if the graph is weighted and the
   wt_size blocks have an effective type in the graph.
   a            : pointer to an adj_lst_t struct initialized with
                  adj_lst_base_init and optionally with adj_lst_align
   g            : pointer to the graph_t struct of a graph initialized with
                  at least graph_base_init
   read_vt      : reads the integer value in the vt_size block of a vertex
                  pointed to by the argument and returns a size_t value
*/
void adj_lst_undir_build(adj_lst_t *a,
			 const graph_t *g,
			 size_t (*read_vt)(const void *));

/**
   Adds a directed edge (u, v) to the adjacency list of a directed
   graph according to a Bernoulli distribution.
   a            : pointer to an adj_lst_t struct initialized with
                  adj_lst_base_init and optionally with adj_lst_align
   u            : u of an (u, v) edge to be added; is less than the
                  number of vertices in an adjacency list
   v            : v of an (u, v) edge to be added; is less than the
                  number of vertices in an adjacency list
   wt           : - NULL if the graph is not weighted
                  - otherwise points to the wt_size block of a weight
   write_vt     : writes the integer value of the second argument to
                  the vt_size block pointed to by the first argument
                  as a value of the integer type used to represent vertices
   bern         : takes arg as the value of its parameter and returns
                  nonzero if the edge is added according to a Bernoulli
                  distribution
   arg          : pointer that is taken as the value of the parameter of
                  bern
*/
void adj_lst_add_dir_edge(adj_lst_t *a,
			  size_t u,
			  size_t v,
			  const void *wt,
			  void (*write_vt)(void *, size_t),
			  int (*bern)(void *),
			  void *arg);

/**
   Adds an undirected edge (u, v) to the adjacency list of an undirected
   graph according to a Bernoulli distribution. Please see the parameter
   specification in adj_lst_add_dir_edge.
*/
void adj_lst_add_undir_edge(adj_lst_t *a,
			    size_t u,
			    size_t v,
			    const void *wt,
			    void (*write_vt)(void *, size_t),
			    int (*bern)(void *),
			    void *arg);

/**
   Builds the adjacency list of a directed graph with num_vts
   vertices, where each of the num_vts(num_vts - 1) possible edges is added
   according to a Bernoulli distribution. The added pairs of vt_size and
   wt_size blocks are aligned in memory according to the preceding calls to
   adj_lst_base_init and optionally adj_lst_align. If the graph is weighted,
   then the effective type of the wt_size block in each vt_size and wt_size
   block pair is not set and can be set by writing a weight value according
   to wt_offset after the call is completed. If the graph is not weighted,
   then there are no wt_size blocks.
   a            : pointer to an adj_lst_t struct initialized with
                  adj_lst_base_init and optionally with adj_lst_align
   write_vt     : writes the integer value of the second argument to
                  the vt_size block pointed to by the first argument
                  as a value of the integer type used to represent vertices
   bern         : takes arg as the value of its parameter and returns
                  nonzero if the edge is added according to a Bernoulli
                  distribution
   arg          : pointer that is taken as the value of the parameter of
                  bern
*/
void adj_lst_rand_dir(adj_lst_t *a,
		      void (*write_vt)(void *, size_t),
		      int (*bern)(void *),
		      void *arg);

/**
   Builds the adjacency list of an undirected graph with num_vts vertices,
   where each of the num_vts(num_vts - 1)/2 possible edges is added
   according to a Bernoulli distribution. The added pairs of vt_size and
   wt_size blocks are aligned in memory according to the preceding calls to
   adj_lst_base_init and optionally adj_lst_align. If the graph is weighted,
   then the effective type of the wt_size block in each vt_size and wt_size
   block pair is not set and can be set by writing the same weight value
   into the two pairs corresponding to (u, v) and (v, u) edges according to
   wt_offset after the call is completed. If the graph is not weighted, then
   there are no wt_size blocks. Please see the parameter specification
   in adj_lst_rand_dir.
*/
void adj_lst_rand_undir(adj_lst_t *a,
			void (*write_vt)(void *, size_t),
			int (*bern)(void *),
			void *arg);

/**
   Frees the memory allocated by adj_lst_base_init and any subsequent
   calls to adj_lst_ operations, and leaves a block of size
   sizeof(adj_lst_t) pointed to by the a parameter.
*/
void adj_lst_free(adj_lst_t *a);

/* A. Vertex operations */

/**
   Read values of different unsigned integer types of vertices. size_t
   can represent any vertex due to graph construction.
*/

size_t graph_read_uchar(const void *a);
size_t graph_read_ushort(const void *a);
size_t graph_read_uint(const void *a);
size_t graph_read_ulong(const void *a);
size_t graph_read_sz(const void *a);

/**
   Write values of different unsigned integer types of vertices.
*/

void graph_write_uchar(void *a, size_t val);
void graph_write_ushort(void *a, size_t val);
void graph_write_uint(void *a, size_t val);
void graph_write_ulong(void *a, size_t val);
void graph_write_sz(void *a, size_t val);

/**
   Get a pointer to the element in the array pointed to by the first
   argument at the index pointed to by the second argument; the first
   argument points to the unsigned integer type used to represent vertices
   and is not dereferenced; the second argument points to a value of the
   same unsigned integer type and is dereferenced.
*/

void *graph_at_uchar(const void *a, const void *i);
void *graph_at_ushort(const void *a, const void *i);
void *graph_at_uint(const void *a, const void *i);
void *graph_at_ulong(const void *a, const void *i);
void *graph_at_sz(const void *a, const void *i);

/**
   Compare the element pointed to by the first argument to the element
   pointed to by the second argument, and return 0 iff the two elements
   are equal; each argument points to a value of the unsigned integer
   type used to represent vertices.
*/

int graph_cmpeq_uchar(const void *a, const void *b);
int graph_cmpeq_ushort(const void *a, const void *b);
int graph_cmpeq_uint(const void *a, const void *b);
int graph_cmpeq_ulong(const void *a, const void *b);
int graph_cmpeq_sz(const void *a, const void *b);

/**
   Increment values of different unsigned integer types. The argument
   points to a value of the unsigned integer type used to represent 
   vertices.
*/

void graph_incr_uchar(void *a);
void graph_incr_ushort(void *a);
void graph_incr_uint(void *a);
void graph_incr_ulong(void *a);
void graph_incr_sz(void *a);

/* B. Weight operations */

/**
   Return a negative integer value if the value pointed to by the first
   argument is less than the value pointed to by the second, a positive
   integer value if the value pointed to by the first argument is greater
   than the value pointed to by the second, and zero integer value
   if the two values are equal; each argument points to a value of the
   integer type used to represent weights. Non-integer weight operations
   on suitable systems are defined by the user.
*/

int graph_cmp_uchar(const void *a, const void *b);
int graph_cmp_ushort(const void *a, const void *b);
int graph_cmp_uint(const void *a, const void *b);
int graph_cmp_ulong(const void *a, const void *b);
int graph_cmp_sz(const void *a, const void *b);

int graph_cmp_schar(const void *a, const void *b);
int graph_cmp_short(const void *a, const void *b);
int graph_cmp_int(const void *a, const void *b);
int graph_cmp_long(const void *a, const void *b);

/**
   Copies the sum of the values pointed to by the second and third
   arguments to the preallocated block pointed to by the first argument;
   the second and third arguments point to values of the integer type used
   to represent weights; if the block pointed to by the first argument
   has no declared type, then the operation sets the effective type to the
   type used to represent weights; the operations with the _perror suffix
   provide an error message and an exit is executed if an integer overflow
   is attempted during addition. Non-integer weight operations on suitable
   systems are defined by the user.
*/

void graph_add_uchar(void *s, const void *a, const void *b);
void graph_add_ushort(void *s, const void *a, const void *b);
void graph_add_uint(void *s, const void *a, const void *b);
void graph_add_ulong(void *s, const void *a, const void *b);
void graph_add_sz(void *s, const void *a, const void *b);

void graph_add_uchar_perror(void *s, const void *a, const void *b);
void graph_add_ushort_perror(void *s, const void *a, const void *b);
void graph_add_uint_perror(void *s, const void *a, const void *b);
void graph_add_ulong_perror(void *s, const void *a, const void *b);
void graph_add_sz_perror(void *s, const void *a, const void *b);

void graph_add_schar(void *s, const void *a, const void *b);
void graph_add_short(void *s, const void *a, const void *b);
void graph_add_int(void *s, const void *a, const void *b);
void graph_add_long(void *s, const void *a, const void *b);

void graph_add_schar_perror(void *s, const void *a, const void *b);
void graph_add_short_perror(void *s, const void *a, const void *b);
void graph_add_int_perror(void *s, const void *a, const void *b);
void graph_add_long_perror(void *s, const void *a, const void *b);

#endif
