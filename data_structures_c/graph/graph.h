/**
   graph.h

   Struct declarations and declarations of accessible functions for 
   representing a graph with generic weights.

   Adjacency list : 

   Each list is represented by a dynamically growing generic stack.
   A vertex is an int index starting from 0. If a graph has edge weights, 
   the edge weights are of any basic type (e.g. char, int, double). 
*/

#ifndef GRAPH_H  
#define GRAPH_H

#include "stack.h"

typedef struct{
  int num_vts;
  int num_es;
  int wt_size;
  int *u; // u's of edges (u, v), NULL if no edges
  int *v; // v's of edges (u, v), NULL if no edges
  void *wts; // NULL if no edges or edge weights
} graph_t;

typedef struct{
  int num_vts;
  int num_es;
  int wt_size;
  stack_t **vts;
  stack_t **wts; // NULL if no edges or edge weights
} adj_lst_t;

/**
   Initializes an adjacency list.
*/
void adj_lst_init(graph_t *g, adj_lst_t *a);

/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(graph_t *g, adj_lst_t *a);

/**
   Builds the adjacency of an undirected graph.
*/
void adj_lst_undir_build(graph_t *g, adj_lst_t *a);

/**
   Frees an adjacency list.
*/
void adj_lst_free(adj_lst_t *a);

#endif
