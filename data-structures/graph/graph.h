/**
   graph.h

   Struct declarations and declarations of accessible functions for 
   representing a graph with generic weights.

   Each list in an adjacency list is represented by a dynamically growing 
   stack. A vertex is a size_t index starting from 0. If a graph is weighted,
   the edge weights are of any basic type (e.g. char, int, double).

   The implementation uses a single stack of adjacent vertex weight pairs
   to achieve cache efficiency in graph algorithms. The value of step_size
   (in bytes) enables a user to iterate with a char *p pointer across a stack
   and access a weight by p + sizeof(size_t) when p points to an adjacent
   vertex.

   Optimization:

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
  size_t wt_size; /* 0 if a graph is unweighted */
  size_t *u;      /* u of (u, v) edges, NULL if no edges */
  size_t *v;      /* v of (u, v) edges, NULL if no edges */
  void *wts;      /* NULL if no edges or wt_size is 0 */
} graph_t;

typedef struct{
  size_t num_vts;
  size_t num_es;
  size_t wt_size;
  size_t step_size; /* sizeof(size_t) + wt_size */
  void *buf; /* buffer that is only used by adj_lst_ functions */
  stack_t **vt_wts;  /* stacks of vertex weight pairs, NULL if no vertices */
} adj_lst_t; /* vertex weight pairs are contiguous to decrease cache misses */

/**
   Initializes a weighted or unweighted graph with n vertices and no edges,
   providing a basis for graph construction.
   g           : pointer to a preallocated block of size sizeof(graph_t)
   n           : number of vertices
   wt_size     : 0 if the graph is unweighted, > 0 otherwise
*/
void graph_base_init(graph_t *g, size_t n, size_t wt_size);

/**
   Frees a graph and leaves a block of size sizeof(graph_t) pointed to by 
   the g parameter.
*/
void graph_free(graph_t *g);

/**
   Initializes the adjacency list of a graph.
   a           : pointer to a preallocated block of size sizeof(adj_lst_t)
   g           : pointer to a graph previously constructed with at least
                 graph_base_init
*/
void adj_lst_init(adj_lst_t *a, const graph_t *g);

size_t adj_lst_v(const adj_lst_t *a, size_t u, size_t i);

void *adj_lst_wt_ptr(const adj_lst_t *a, size_t u, size_t i);

/**
   Frees an adjacency list and leaves a block of size sizeof(adj_lst_t)
   pointed to by the a parameter.
*/
void adj_lst_free(adj_lst_t *a);

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
   Builds the adjacency list of a directed unweighted graph with n vertices,
   where each of n(n - 1) possible edges is added according to the Bernoulli
   distribution provided by bern that takes arg as its parameter. An edge is
   added if bern returns nonzero.
*/
void adj_lst_rand_dir(adj_lst_t *a,
		      size_t n,
		      int (*bern)(void *),
		      void *arg);

/**
   Builds the adjacency list of an undirected unweighted graph with n 
   vertices, where each of n(n - 1)/2 possible edges is added according to
   the Bernoulli distribution provided by bern that takes arg as its
   parameter. An edge is added if bern returns nonzero.
*/
void adj_lst_rand_undir(adj_lst_t *a,
			size_t n,
			int (*bern)(void *),
			void *arg);

#endif
