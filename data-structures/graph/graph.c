/**
   graph.c

   Functions for representing a graph with generic weights. 

   Each list in an adjacency list is represented by a dynamically growing 
   stack. A vertex is a size_t index starting from 0. If a graph is
   weighted, the edge weights are of any basic type (e.g. char, int,
   double).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

static size_t *u_ptr(const graph_t *g, size_t i);
static size_t *v_ptr(const graph_t *g, size_t i);
static void *wt_ptr(const graph_t *g, size_t i);

const size_t STACK_INIT_COUNT = 1;

/**
   Initializes a weighted or unweighted graph with n vertices and no edges,
   providing a basis for graph construction.
   g           : pointer to a preallocated block of size sizeof(graph_t)
   n           : number of vertices
   wt_size     : 0 if the graph is unweighted, > 0 otherwise
*/
void graph_base_init(graph_t *g, size_t n, size_t wt_size){
  g->num_vts = n;
  g->num_es = 0;
  g->wt_size = wt_size;
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Frees a graph and leaves a block of size sizeof(graph_t) pointed to by 
   the g parameter.
*/
void graph_free(graph_t *g){
  free(g->u); //free(NULL) performs no operation
  free(g->v);
  free(g->wts);
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Initializes the adjacency list of a graph.
   a           : pointer to a preallocated block of size sizeof(adj_lst_t)
   g           : pointer to a graph previously constructed with at least
                 graph_base_init
*/
void adj_lst_init(adj_lst_t *a, const graph_t *g){
  a->num_vts = g->num_vts;
  a->num_es = 0; 
  a->wt_size = g->wt_size;
  a->vts = NULL;
  a->wts = NULL;
  if (a->num_vts > 0){
    a->vts = malloc_perror(a->num_vts * sizeof(stack_t *));
    if (a->wt_size > 0){
      a->wts = malloc_perror(a->num_vts * sizeof(stack_t *));
    }
  }
  //initialize stacks
  for (size_t i = 0; i < a->num_vts; i++){
    a->vts[i] = malloc_perror(sizeof(stack_t));
    stack_init(a->vts[i], STACK_INIT_COUNT, sizeof(size_t), NULL);
    if (a->wt_size > 0){
      a->wts[i] = malloc_perror(sizeof(stack_t));
      stack_init(a->wts[i], STACK_INIT_COUNT, a->wt_size, NULL);
    }
  }
}

/**
   Frees an adjacency list and leaves a block of size sizeof(adj_lst_t)
   pointed to by the a parameter.
*/
void adj_lst_free(adj_lst_t *a){
  for (size_t i = 0; i < a->num_vts; i++){
    stack_free(a->vts[i]);
    free(a->vts[i]);
    a->vts[i] = NULL;
    if (a->wt_size > 0){
      stack_free(a->wts[i]);
      free(a->wts[i]);
      a->wts[i] = NULL;
    }
  }
  free(a->vts); //free(NULL) performs no operation
  free(a->wts);
  a->vts = NULL;
  a->wts = NULL;
}
   
/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(adj_lst_t *a, const graph_t *g){
  for (size_t i = 0; i < g->num_es; i++){
    //at least one edge
    stack_push(a->vts[*u_ptr(g, i)], v_ptr(g, i));
    a->num_es++;
    if (a->wt_size > 0){
      stack_push(a->wts[*u_ptr(g, i)], wt_ptr(g, i));
    }
  }
}

/**
   Builds the adjacency list of an undirected graph.
*/
void adj_lst_undir_build(adj_lst_t *a, const graph_t *g){
  for (size_t i = 0; i < g->num_es; i++){
    //at least one edge
    stack_push(a->vts[*u_ptr(g, i)], v_ptr(g, i));
    stack_push(a->vts[*v_ptr(g, i)], u_ptr(g, i));
    a->num_es += 2;
    if (a->wt_size > 0){
      stack_push(a->wts[*u_ptr(g, i)], wt_ptr(g, i));
      stack_push(a->wts[*v_ptr(g, i)], wt_ptr(g, i));
    }
  }
}

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
			  void *arg){
  if (bern(arg)){
    stack_push(a->vts[u], &v);
    if (wt != NULL){
      stack_push(a->wts[u], wt);
    }
    a->num_es++;
  }
}

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
			    void *arg){
  if (bern(arg)){
    stack_push(a->vts[u], &v);
    stack_push(a->vts[v], &u);
    if (wt != NULL){
      stack_push(a->wts[u], wt);
      stack_push(a->wts[v], wt);
    }
    a->num_es += 2;
  }
}

/**
   Builds the adjacency list of a directed unweighted graph with n vertices,
   where each of n(n - 1) possible edges is added according to the Bernoulli
   distribution provided by bern that takes arg as its parameter. An edge is
   added if bern returns nonzero.
*/
void adj_lst_rand_dir(adj_lst_t *a,
		      size_t n,
		      int (*bern)(void *),
		      void *arg){
  graph_t g;
  graph_base_init(&g, n, 0);
  adj_lst_init(a, &g);
  if (n < 1) return;
  for (size_t i = 0; i < n - 1; i++){
    for (size_t j = i + 1; j < n; j++){
      adj_lst_add_dir_edge(a, i, j, NULL, bern, arg);
      adj_lst_add_dir_edge(a, j, i, NULL, bern, arg);
    }
  }
}

/**
   Builds the adjacency list of an undirected unweighted graph with n 
   vertices, where each of n(n - 1)/2 possible edges is added according to
   the Bernoulli distribution provided by bern that takes arg as its
   parameter. An edge is added if bern returns nonzero.
*/
void adj_lst_rand_undir(adj_lst_t *a,
			size_t n,
			int (*bern)(void *),
			void *arg){
  graph_t g;
  graph_base_init(&g, n, 0);
  adj_lst_init(a, &g);
  if (n < 1) return;
  for (size_t i = 0; i < n - 1; i++){
    for (size_t j = i + 1; j < n; j++){
      adj_lst_add_undir_edge(a, i, j, NULL, bern, arg);
    }
  }
}

/** Helper functions */

/**
   Compute pointers to vertices and weights in a graph.
*/

static size_t *u_ptr(const graph_t *g, size_t i){
  return &(g->u[i]);
}

static size_t *v_ptr(const graph_t *g, size_t i){
  return &(g->v[i]);
}

static void *wt_ptr(const graph_t *g, size_t i){
  return (void *)((char *)g->wts + i * g->wt_size);
}
