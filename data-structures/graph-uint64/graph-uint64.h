/**
   graph-uint64.h

   Struct declarations and declarations of accessible functions for 
   representing a graph with generic weights.

   Each list in an adjacency list is represented by a dynamically growing 
   generic stack. A vertex is an uint64_t index starting from 0. If a graph 
   has edge weights, the edge weights are of any basic type (e.g. char, int, 
   double).
*/

#ifndef GRAPH_UINT64_H  
#define GRAPH_UINT64_H

#include <stdint.h>
#include "stack-uint64.h"

typedef struct{
  uint64_t num_vts; 
  uint64_t num_es;
  int wt_size; //0 if no edge weights
  uint64_t *u; //u of edges (u, v), NULL if no edges
  uint64_t *v; //v of edges (u, v), NULL if no edges
  void *wts; //NULL if no edges or edge weights;
             //not NULL if there are vertices, edges, and weights
} graph_uint64_t;

typedef struct{
  uint64_t num_vts;
  uint64_t num_es;
  int wt_size;
  stack_uint64_t **vts; //NULL if no vertices
  stack_uint64_t **wts; //NULL if no vertices or edge weights
} adj_lst_uint64_t;

/**
   Initializes a graph with n vertices and no edges.
   wt_size: 0 if no edge weights, > 0 otherwise
*/
void graph_uint64_base_init(graph_uint64_t *g, uint64_t n, int wt_size);

/**
   Frees  a graph and leaves a block of size sizeof(graph_uint64_t) pointed
   to by the g parameter.
*/
void graph_uint64_free(graph_uint64_t *g);

/**
   Initializes the adjacency list of a graph.
*/
void adj_lst_uint64_init(adj_lst_uint64_t *a, graph_uint64_t *g);

/**
   Frees an adjacency list and leaves a block of size 
   sizeof(adj_lst_uint64_t) pointed to by the a parameter.
*/
void adj_lst_uint64_free(adj_lst_uint64_t *a);

/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_uint64_dir_build(adj_lst_uint64_t *a, graph_uint64_t *g);

/**
   Builds the adjacency list of an undirected graph.
*/
void adj_lst_uint64_undir_build(adj_lst_uint64_t *a, graph_uint64_t *g);

/**
   Adds a directed unweighted edge (u, v) with probability num/denom. If 
   num == denom, there is no overhead of generating a random number.
*/
void adj_lst_uint64_add_dir_edge(adj_lst_uint64_t *a,
				 uint64_t u,
				 uint64_t v,
				 uint32_t numer,
				 uint32_t denom);

/**
   Adds an undirected unweighted edge (u, v) with probability nom/denom. If 
   nom == denom, there is no overhead of generating a random number.
*/
void adj_lst_uint64_add_undir_edge(adj_lst_uint64_t *a,
				   uint64_t u,
				   uint64_t v,
				   uint32_t num,
				   uint32_t denom);

/**
   Builds the adjacency list of a directed unweighted graph with n vertices,
   where each of n(n - 1) possible edges is added with probability nom/denom.
   If nom == denom, there is no overhead of generating a random number.
*/
void adj_lst_uint64_rand_dir(adj_lst_uint64_t *a,
			     uint64_t n,
			     uint32_t num,
			     uint32_t denom);

/**
   Builds the adjacency list of an undirected unweighted graph with n 
   vertices, where each of n(n - 1)/2 possible edges is added with 
   probability nom/denom. If nom == denom, there is no overhead of generating
   a random number.
*/
void adj_lst_uint64_rand_undir(adj_lst_uint64_t *a,
			       uint64_t n,
			       uint32_t num,
			       uint32_t denom);

#endif
