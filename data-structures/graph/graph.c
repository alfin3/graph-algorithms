/**
   graph.c

   Functions for representing a graph with generic weights. 

   Each list in an adjacency list is represented by a dynamically growing 
   stack. A vertex is a size_t index starting from 0. If a graph is weighted,
   the edge weights are of any basic type (e.g. char, int, double).

   The implementation uses a single stack of adjacent vertex weight pairs
   to achieve cache efficiency in graph algorithms. The value of pair_size
   (in bytes) enables a user to iterate with a char *p pointer across a stack
   and access a weight by p + offset when p points to a vertex of a pair.

   Due to cache-efficient allocation, the implementation requires that
   sizeof(size_t) and the size of a generic weight are powers of two.
   The size of weight can also be 0.

   Optimization:

   -  The implementation resulted in upto 1.3 - 1.4x speedups for dijkstra
   and prim and upto 1.1x for tsp over an implementation with separate vertex
   and weight stacks in tests on a machine with the following caches (cache,
   capacity, k-way associativity, line size): (L1inst, 32768, 8, 64),
   (L1data, 32768, 8, 64), (L2, 262144, 4, 64), (L3, 3145728, 12, 64).
   Compilation was performed with gcc and -flto -O3. No notable decrease of
   performance was recorded in tests of bfs and dfs on unweighted graphs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

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
  free(g->u); /* free(NULL) performs no operation */
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
  size_t i;
  a->num_vts = g->num_vts;
  a->num_es = 0; 
  a->wt_size = g->wt_size;
  /* compute vertex weight pair size with general alignment in memory */
  if (a->wt_size == 0){
    a->pair_size = sizeof(size_t);
    a->offset = 0;
  }else if (a->wt_size <= sizeof(size_t)){
    /* sizeof(size_t) is mult. of  wt_size */
    a->pair_size = mul_sz_perror(2, sizeof(size_t));
    a->offset = sizeof(size_t);
  }else{
    /* wt_size is mult of sizeof(size_t); malloc's pointer is wt aligned */
    a->pair_size = mul_sz_perror(2, a->wt_size);
    a->offset = a->wt_size;
  }
  a->buf = malloc_perror(1, a->pair_size);
  a->vt_wts = NULL;
  if (a->num_vts > 0){
    a->vt_wts = malloc_perror(a->num_vts, sizeof(stack_t *));
  }
  /* initialize stacks */
  for (i = 0; i < a->num_vts; i++){
    a->vt_wts[i] = malloc_perror(1, sizeof(stack_t));
    stack_init(a->vt_wts[i], STACK_INIT_COUNT, a->pair_size, NULL);
  }
}    
   
/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(adj_lst_t *a, const graph_t *g){
  size_t i;
  for (i = 0; i < g->num_es; i++){
    memcpy(a->buf, &g->v[i], sizeof(size_t));
    if (a->wt_size > 0){
      memcpy((char *)a->buf + a->offset, wt_ptr(g, i), a->wt_size);
    }
    stack_push(a->vt_wts[g->u[i]], a->buf);
    a->num_es++;
  }
}

/**
   Builds the adjacency list of an undirected graph.
*/
void adj_lst_undir_build(adj_lst_t *a, const graph_t *g){
  size_t i;
  for (i = 0; i < g->num_es; i++){
    memcpy(a->buf, &g->v[i], sizeof(size_t));
    if (a->wt_size > 0){
      memcpy((char *)a->buf + a->offset, wt_ptr(g, i), a->wt_size);
    }
    stack_push(a->vt_wts[g->u[i]], a->buf);
    memcpy(a->buf, &g->u[i], sizeof(size_t));
    stack_push(a->vt_wts[g->v[i]], a->buf);
    a->num_es += 2;
  }
}

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
			  void *arg){
  if (bern(arg)){
    memcpy(a->buf, &v, sizeof(size_t));
    if (a->wt_size > 0){
      memcpy((char *)a->buf + a->offset, wt, a->wt_size);
    }
    stack_push(a->vt_wts[u], a->buf);
    a->num_es++;
  }
}

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
			    void *arg){
  if (bern(arg)){
    memcpy(a->buf, &v, sizeof(size_t));
    if (a->wt_size > 0){
      memcpy((char *)a->buf + a->offset, wt, a->wt_size);
    }
    stack_push(a->vt_wts[u], a->buf);
    memcpy(a->buf, &u, sizeof(size_t));
    stack_push(a->vt_wts[v], a->buf);
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
  size_t i, j;
  graph_t g;
  graph_base_init(&g, n, 0);
  adj_lst_init(a, &g);
  if (n > 0){
    for (i = 0; i < n - 1; i++){
      for (j = i + 1; j < n; j++){
	adj_lst_add_dir_edge(a, i, j, NULL, bern, arg);
	adj_lst_add_dir_edge(a, j, i, NULL, bern, arg);
      }
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
  size_t i, j;
  graph_t g;
  graph_base_init(&g, n, 0);
  adj_lst_init(a, &g);
  if (n > 0){
    for (i = 0; i < n - 1; i++){
      for (j = i + 1; j < n; j++){
	adj_lst_add_undir_edge(a, i, j, NULL, bern, arg);
      }
    }
  }
}

/**
   Frees an adjacency list and leaves a block of size sizeof(adj_lst_t)
   pointed to by the a parameter.
*/
void adj_lst_free(adj_lst_t *a){
  size_t i;
  for (i = 0; i < a->num_vts; i++){
    stack_free(a->vt_wts[i]);
    free(a->vt_wts[i]);
    a->vt_wts[i] = NULL;
  }
  free(a->buf);
  free(a->vt_wts); /* free(NULL) performs no operation */
  a->buf = NULL;
  a->vt_wts = NULL;
}

/** Helper functions */

static void *wt_ptr(const graph_t *g, size_t i){
  return (void *)((char *)g->wts + i * g->wt_size);
}
