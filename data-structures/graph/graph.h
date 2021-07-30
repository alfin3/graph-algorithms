/**
   graph.h

   Struct declarations and declarations of accessible functions for 
   representing a graph with generic integer vertices and generic
   weights.

   Each list in an adjacency list is represented by a dynamically growing 
   stack. A vertex is of any inteter type with values starting from 0. If
   a graph is weighted, an edge weight is an object within a contiguous 
   memory block, such as an object of basic type (e.g. char, int, double)
   or a struct (e.g. two unsigned integers).
  
   A single stack of adjacent vertex weight pairs with adjustable alignment
   in memory is used to achieve cache efficiency in graph algorithms.
   Depending on the problem size and a given system, the choice of an integer
   type with a smaller size for vertices  may provide additional
   cache efficiency in addition to reducing space requirements.

   The implementation only uses integer and pointer operations (any non-
   integer operations on weights are defined by the user). Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.

   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.

   Optimization notes:

   -  The implementation resulted in upto 1.3 - 1.4x speedups for dijkstra
   and prim and upto 1.1x for tsp over an implementation with separate vertex
   and weight stacks in tests on a machine with the following caches (cache,
   capacity, k-way associativity, line size): (L1inst, 32768, 8, 64),
   (L1data, 32768, 8, 64), (L2, 262144, 4, 64), (L3, 3145728, 12, 64).
   Compilation was performed with gcc and -flto -O3. No notable decrease of
   performance was recorded in tests of bfs and dfs on unweighted graphs.
*/

#ifndef GRAPH_H  
#define GRAPH_H

#include <stdlib.h>
#include "stack.h"

typedef struct{
  size_t num_vts; 
  size_t num_es;
  size_t vt_size;
  size_t wt_size; /* 0 if a graph is unweighted */
  void *u;        /* u of (u, v) edges, NULL if no edges */
  void *v;        /* v of (u, v) edges, NULL if no edges */
  void *wts;      /* NULL if no edges or wt_size is 0 */
  size_t (*read_vt)(const void *);
  void (*write_vt)(void *, size_t);
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
  size_t (*read_vt)(const void *);
  void (*write_vt)(void *, size_t);
} adj_lst_t;

/**
   Read and write vertices of different integer types.
*/

size_t graph_read_uchar(const void *a);
size_t graph_read_ushort(const void *a);
size_t graph_read_uint(const void *a);
size_t graph_read_ulong(const void *a);
size_t graph_read_sz(const void *a);

void graph_write_uchar(void *a, size_t val);
void graph_write_ushort(void *a, size_t val);
void graph_write_uint(void *a, size_t val);
void graph_write_ulong(void *a, size_t val);
void graph_write_sz(void *a, size_t val);

/**
   Initializes a weighted or unweighted graph with num_vts vertices and
   no edges, providing a basis for graph construction.
   g           : pointer to a preallocated block of size sizeof(graph_t)
   num_vts     : number of vertices
   vt_size     : > 0 size of the integer type used to represent a vertex
   wt_size     : > 0 size of object used to represent a weight, if a graph
                 is weighted; 0 if the graph is not weighted
   read_vt     : non-NULL pointer to a function for reading a vertex, which
                 may be one of the provided functions
   write_vt    : non-NULL pointer to a function for writing a vertex, which
                 may be one of the provided functions
*/
void graph_base_init(graph_t *g,
		     size_t num_vts,
		     size_t vt_size,
		     size_t wt_size,
		     size_t (*read_vt)(const void *),
		     void (*write_vt)(void *, size_t));

/**
   Frees a graph and leaves a block of size sizeof(graph_t) pointed to by 
   the g parameter.
*/
void graph_free(graph_t *g);

/**
   Initializes an empty adjacency list according to a graph. Aligns vertices
   and weights according to their sizes, which may result in overalignment.
   a           : pointer to a preallocated block of size sizeof(adj_lst_t)
   g           : pointer to a graph previously constructed with at least
                 graph_base_init
*/
void adj_lst_base_init(adj_lst_t *a, const graph_t *g);

/**
   Aligns the vertices and weights of an adjacency list according to 
   the values of the alignment parameters. If the alignment requirement
   of only one type is known, then the size of the other type can be used
   as a value of the other alignment parameter because size of
   type >= alignment requirement of type (due to structure of arrays), 
   which may result in overalignment. The call to this operation may 
   result in reduction of space requirements as compared to adj_lst_base_init
   alone. The operation is optionally called after adj_lst_base_init is
   completed and before any other adj_list_ operation is called. 
   a            : pointer to a adj_lst_t struct initialized with
                  adj_lst_base_init
   vt_alignment : alignment requirement or size of the integer type of
                  a vertex
   wt_alignment : alignment requirement or size of the type of a weight
*/
void adj_lst_align(adj_lst_t *a,
		   size_t vt_alignment,
		   size_t wt_alignment);

/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(adj_lst_t *a, const graph_t *g);

/**
   Builds the adjacency list of an undirected graph.
*/
void adj_lst_undir_build(adj_lst_t *a, const graph_t *g);

/**
   Adds a directed edge (u, v) according to the Bernoulli distribution
   provided by bern that takes arg as its parameter. The edge is added if
   bern returns nonzero. If a graph is weighted, wt points to a weight
   of size wt_size, otherwise wt is NULL.
*/
void adj_lst_add_dir_edge(adj_lst_t *a,
			  size_t u,
			  size_t v,
			  const void *wt,
			  int (*bern)(void *),
			  void *arg);

/**
   Adds an undirected edge (u, v) according to the Bernoulli distribution
   provided by bern that takes arg as its parameter. The edge is added if
   bern returns nonzero. If a graph is weighted, wt points to a weight of
   size wt_size, otherwise wt is NULL.
*/
void adj_lst_add_undir_edge(adj_lst_t *a,
			    size_t u,
			    size_t v,
			    const void *wt,
			    int (*bern)(void *),
			    void *arg);

/**
   Builds the adjacency list of a directed unweighted graph with num_vts
   vertices, where each of num_vts(num_vts - 1) possible edges is added
   according to the Bernoulli distribution provided by bern that takes arg
   as its parameter. An edge is added if bern returns nonzero. a is pointer
   to a preallocated block of size sizeof(adj_lst_t), not initialized.
   Please see the parameter specification in graph_base_init and
   adj_lst_base_init.
*/
void adj_lst_rand_dir(adj_lst_t *a,
		      size_t num_vts,
		      size_t vt_size,
		      size_t (*read_vt)(const void *),
		      void (*write_vt)(void *, size_t),
		      int (*bern)(void *),
		      void *arg);

/**
   Builds the adjacency list of an undirected unweighted graph with num_vts 
   vertices, where each of num_vts(num_vts - 1)/2 possible edges is added
   according to the Bernoulli distribution provided by bern that takes arg
   as its parameter. An edge is added if bern returns nonzero. a is pointer
   to a preallocated block of size sizeof(adj_lst_t), not initialized.
   Please see the parameter specification in graph_base_init and
   adj_lst_base_init.
*/
void adj_lst_rand_undir(adj_lst_t *a,
			size_t num_vts,
			size_t vt_size,
			size_t (*read_vt)(const void *),
			void (*write_vt)(void *, size_t),
			int (*bern)(void *),
			void *arg);

/**
   Frees an adjacency list and leaves a block of size sizeof(adj_lst_t)
   pointed to by the a parameter.
*/
void adj_lst_free(adj_lst_t *a);

#endif
