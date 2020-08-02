/**
   graph.h

   Struct declarations and declarations of accessible functions for 
   representing a graph with generic weights.

   Adjacency list : 

   Each list in an adjacency list is represented by a dynamically growing 
   generic stack. A vertex is an int index starting from 0. If a graph has 
   edge weights, the edge weights are of any basic type (e.g. char, int, 
   double).  
*/

#ifndef GRAPH_H  
#define GRAPH_H

#include <stdint.h>
#include "stack.h"

typedef struct{
  int num_vts;
  int num_es;
  int wt_size;
  int *u; //u's of edges (u, v), NULL if no edges
  int *v; //v's of edges (u, v), NULL if no edges
  void *wts; //NULL if no edges or edge weights
} graph_t;

typedef struct{
  int num_vts;
  int num_es;
  int wt_size;
  stack_t **vts; //NULL if no vertices
  stack_t **wts; //NULL if no edges or edge weights
} adj_lst_t;

/**
   Initializes a graph with n vertices and no edges, where n >= 0.
*/
void graph_base_init(graph_t *g, int n);

/**
   Frees a graph.
*/
void graph_free(graph_t *g);

/**
   Initializes an adjacency list.
*/
void adj_lst_init(graph_t *g, adj_lst_t *a);

/**
   Frees an adjacency list.
*/
void adj_lst_free(adj_lst_t *a);

/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(graph_t *g, adj_lst_t *a);

/**
   Builds the adjacency of an undirected graph.
*/
void adj_lst_undir_build(graph_t *g, adj_lst_t *a);

/**
   Adds a directed unweighted edge (u, v) with probability nom/denom. If 
   nom = denom, there is no overhead of generating a random number.
*/
void adj_lst_add_dir_edge(adj_lst_t *a,
			  int u,
			  int v,
			  uint32_t nom,
			  uint32_t denom);

/**
   Adds an undirected unweighted edge (u, v) with probability nom/denom. If 
   nom = denom, there is no overhead of generating a random number.
*/
void adj_lst_add_undir_edge(adj_lst_t *a,
			    int u,
			    int v,
			    uint32_t nom,
			    uint32_t denom);

/**
   Builds the adjacency list of a directed graph with n vertices, where each
   of n(n - 1) possible edges is added with probability nom/denom. If 
   nom = denom, there is no overhead of generating a random number.
*/
void adj_lst_rand_dir(adj_lst_t *a, int n, uint32_t nom, uint32_t denom);

/**
   Builds the adjacency list of an undirected graph with n vertices, where 
   each of n(n - 1)/2 possible edges is added with probability nom/denom. 
   If nom = denom, there is no overhead of generating a random number.
*/
void adj_lst_rand_undir(adj_lst_t *a, int n, uint32_t nom, uint32_t denom);

#endif
