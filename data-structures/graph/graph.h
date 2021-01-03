/**
   graph.h

   Struct declarations and declarations of accessible functions for 
   representing a graph with generic weights.

   Each list in an adjacency list is represented by a dynamically growing 
   stack. A vertex is a size_t index starting from 0. If a graph is
   weighted, the edge weights are of any basic type (e.g. char, int,
   double).
*/

#ifndef GRAPH_H  
#define GRAPH_H

#include <stdlib.h>
#include "stack.h"

typedef struct{
  size_t num_vts; 
  size_t num_es;
  size_t wt_size; //0 if a graph is unweighted
  size_t *u;      //u of (u, v), NULL if no edges
  size_t *v;      //v of (u, v), NULL if no edges
  void *wts;      //NULL if no edges or wt_size is 0
} graph_t;

typedef struct{
  size_t num_vts;
  size_t num_es;
  size_t wt_size;
  stack_t **vts;  //NULL if no vertices
  stack_t **wts;  //NULL if no vertices or wt_size is 0
} adj_lst_t;

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
   Adds a directed unweighted edge (u, v) according to the Bernoulli
   distribution provided by bern that takes arg as its parameter. The edge
   is added if bern returns nonzero. If a graph is weighted, wt points to a
   a weight of size wt_size, otherwise wt is NULL.
*/
void adj_lst_add_dir_edge(adj_lst_t *a,
			  size_t u,
			  size_t v,
			  const void *wt,
			  int (*bern)(void *),
			  void *arg);

/**
   Adds an undirected unweighted edge (u, v) according to the Bernoulli
   distribution provided by bern that takes arg as its parameter. The edge
   is added if bern returns nonzero. If a graph is weighted, wt points to a
   a weight of size wt_size, otherwise wt is NULL.
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
