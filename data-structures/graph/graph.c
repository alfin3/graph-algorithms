/**
   graph.c

   Functions for representing a graph with generic vertices and weights. 

   Each list in an adjacency list is represented by a dynamically growing 
   stack. A vertex is of any inteter type starting with values starting from
   0. If a graph is weighted, the edge weights are of any basic type (e.g.
   char, int, double) or struct (e.g. two unsigned integer). A single stack
   of adjacent vertex weight pairs with suitable alignment is used to
   achieve cache efficiency in graph algorithms.

   The implementation only uses integer and pointer operations (any non-
   integer operations on weights are defined by the user). Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.

   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.

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

const size_t STACK_INIT_COUNT = 1;

/**
   Initializes a weighted or unweighted graph with n vertices and no edges,
   providing a basis for graph construction.
   g           : pointer to a preallocated block of size sizeof(graph_t)
   n           : number of vertices
   wt_size     : 0 if the graph is unweighted, > 0 otherwise
*/
void graph_base_init(graph_t *g,
		     size_t num_vts,
		     size_t vt_size,
		     size_t wt_size,
		     size_t (*read_vt)(const void *),
		     void (*write_vt)(void *, size_t)){
  g->num_vts = num_vts;
  g->num_es = 0;
  g->vt_size = vt_size;
  g->wt_size = wt_size;
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
  g->read_vt = read_vt;
  g->write_vt = write_vt;
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
   Initializes an empty adjacency list according to a graph.
   a           : pointer to a preallocated block of size sizeof(adj_lst_t)
   g           : pointer to a graph previously constructed with at least
                 graph_base_init
*/
void adj_lst_base_init(adj_lst_t *a, const graph_t *g){
  size_t i;
  size_t wt_rem, vt_rem;
  a->num_vts = g->num_vts;
  a->num_es = 0;
  a->vt_size = g->vt_size;
  a->wt_size = g->wt_size;
  /* align weight relative to a malloc's pointer and compute pair_size */
  if (a->wt_size == 0){
    a->wt_offset = a->vt_size;
  }else if (a->vt_size <= a->wt_size){
    a->wt_offset = a->wt_size;
  }else{
    wt_rem = a->vt_size % a->wt_size;
    a->wt_offset = a->vt_size;
    a->wt_offset = add_sz_perror(a->wt_offset,
				 (wt_rem > 0) * (a->wt_size - wt_rem));
    
  }
  vt_rem = add_sz_perror(a->wt_offset, a->wt_size) % a->vt_size;
  a->pair_size = add_sz_perror(a->wt_offset + a->wt_size,
			       (vt_rem > 0) * (a->vt_size - vt_rem));
  a->buf = calloc_perror(1, a->pair_size);
  a->vt_wts = NULL;
  if (a->num_vts > 0){
    a->vt_wts = malloc_perror(a->num_vts, sizeof(stack_t *));
  }
  /* initialize stacks */
  for (i = 0; i < a->num_vts; i++){
    a->vt_wts[i] = malloc_perror(1, sizeof(stack_t));
    stack_init(a->vt_wts[i], STACK_INIT_COUNT, a->pair_size, NULL);
  }
  a->read_vt = g->read_vt;
  a->write_vt = g->write_vt;
}

/**
   Aligns the vertices and weights of an adjacency list according to 
   the values of the alignment parameters. If the alignment requirement
   of only one type is known, then the size of the other type can be used
   as a value of the other alignment parameter because size of
   type >= alignment requirement of type (due to structure of arrays), 
   which may result in overalignment. The call to this operation may 
   result in reduction of space requirements as compared to adj_lst_base_init
   alone. The operation is optionally called after adj_lst_base_init is
   completed and before any other operation is adj_list_ operation is
   called. 
   a            : pointer to a adj_lst_t struct initialized with
                  adj_lst_base_init
   vt_alignment : alignment requirement or size of the integer type of
                  a vertex
   wt_alignment : alignment requirement or size of the type of a weight
*/
void adj_lst_align(adj_lst_t *a,
		   size_t vt_alignment,
		   size_t wt_alignment){
  size_t i;
  size_t wt_rem, vt_rem;
  if (a->wt_size == 0){
    a->wt_offset = a->vt_size;
  }else if (a->vt_size <= wt_alignment){
    a->wt_offset = wt_alignment;
  }else{
    wt_rem = a->vt_size % wt_alignment;
    a->wt_offset = a->vt_size;
    a->wt_offset = add_sz_perror(a->wt_offset,
				 (wt_rem > 0) * (wt_alignment - wt_rem));
  }
  vt_rem = add_sz_perror(a->wt_offset, a->wt_size) % vt_alignment;
  a->pair_size = add_sz_perror(a->wt_offset + a->wt_size,
			       (vt_rem > 0) * (vt_alignment - vt_rem));
  a->buf = realloc_perror(a->buf, 1, a->pair_size);
  memset(a->buf, 0, a->pair_size);
  a->vt_wts = NULL;
  /* initialize stacks */
  for (i = 0; i < a->num_vts; i++){
    stack_free(a->vt_wts[i]);
    stack_init(a->vt_wts[i], STACK_INIT_COUNT, a->pair_size, NULL);
  }
}
   
/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(adj_lst_t *a, const graph_t *g){
  size_t i;
  const char *u = g->u;
  const char *v = g->v;
  const char *wt = g->wts;
  char *buf_wt = (char *)a->buf + a->wt_offset;
  for (i = 0; i < g->num_es; i++){
    memcpy(a->buf, v, a->vt_size);
    if (a->wt_size > 0){
      memcpy(buf_wt, wt, a->wt_size);
    }
    stack_push(a->vt_wts[a->read_vt(u)], a->buf);
    a->num_es++;
    u += a->vt_size;
    v += a->vt_size;
    wt += a->wt_size;
  }
}

/**
   Builds the adjacency list of an undirected graph.
*/
void adj_lst_undir_build(adj_lst_t *a, const graph_t *g){
  size_t i;
  const char *u = g->u;
  const char *v = g->v;
  const char *wt = g->wts;
  char *buf_wt = (char *)a->buf + a->wt_offset;
  for (i = 0; i < g->num_es; i++){
    memcpy(a->buf, v, a->vt_size);
    if (a->wt_size > 0){
      memcpy(buf_wt, wt, a->wt_size);
    }
    stack_push(a->vt_wts[a->read_vt(u)], a->buf);
    memcpy(a->buf, u, a->vt_size);
    stack_push(a->vt_wts[a->read_vt(v)], a->buf);
    a->num_es += 2;
    u += a->vt_size;
    v += a->vt_size;
    wt += a->wt_size;
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
    a->write_vt(a->buf, v);
    if (a->wt_size > 0 && wt != NULL){
      memcpy((char *)a->buf + a->wt_offset, wt, a->wt_size);
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
    a->write_vt(a->buf, v);
    if (a->wt_size > 0 && wt != NULL){
      memcpy((char *)a->buf + a->wt_offset, wt, a->wt_size);
    }
    stack_push(a->vt_wts[u], a->buf);
    a->write_vt(a->buf, u);
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
		      size_t num_vts,
		      size_t vt_size,
		      size_t (*read_vt)(const void *),
		      void (*write_vt)(void *, size_t),
		      int (*bern)(void *),
		      void *arg){
  size_t i, j;
  graph_t g;
  graph_base_init(&g, num_vts, vt_size, 0, read_vt, write_vt);
  adj_lst_base_init(a, &g);
  if (num_vts > 0){
    for (i = 0; i < num_vts - 1; i++){
      for (j = i + 1; j < num_vts; j++){
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
			size_t num_vts,
			size_t vt_size,
			size_t (*read_vt)(const void *),
			void (*write_vt)(void *, size_t),
			int (*bern)(void *),
			void *arg){
  size_t i, j;
  graph_t g;
  graph_base_init(&g, num_vts, vt_size, 0, read_vt, write_vt);
  adj_lst_base_init(a, &g);
  if (num_vts > 0){
    for (i = 0; i < num_vts - 1; i++){
      for (j = i + 1; j < num_vts; j++){
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
