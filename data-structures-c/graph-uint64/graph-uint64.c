/**
   graph-uint64.c

   Functions for representing a graph with generic weights. 

   Each list in an adjacency list is represented by a dynamically growing 
   generic stack. A vertex is an uint64_t index starting from 0. If a graph 
   has edge weights, the edge weights are of any basic type (e.g. char, int, 
   double). 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "graph-uint64.h"
#include "stack-uint64.h"
#include "utilities-ds.h"

static uint64_t *u_ptr(graph_uint64_t *g, uint64_t i);
static uint64_t *v_ptr(graph_uint64_t *g, uint64_t i);
static void *wt_ptr(graph_uint64_t *g, uint64_t i);

/**
   Initializes a graph with n vertices and no edges.
*/
void graph_uint64_base_init(graph_uint64_t *g, uint64_t n){
  g->num_vts = n;
  g->num_es = 0;
  g->wt_size = 0;
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Frees  a graph and leaves a block of size sizeof(graph_uint64_t) pointed
   to by the g parameter.
*/
void graph_uint64_free(graph_uint64_t *g){
  free(g->u); //free(NULL) does nothing
  free(g->v);
  free(g->wts);
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Initializes the adjacency list of a graph.
*/
static void adj_lst_uint64_init_helper(stack_uint64_t **s, int elt_size);

void adj_lst_uint64_init(adj_lst_uint64_t *a, graph_uint64_t *g){
  a->num_vts = g->num_vts;
  a->num_es = 0; 
  a->wt_size = g->wt_size;
  if (g->num_vts == 0){
    //no vertices
    a->vts = NULL;
  }else{
    a->vts = malloc(a->num_vts * sizeof(stack_uint64_t *));
    assert(a->vts != NULL);
  }
  if (g->wts == NULL){
    //no weights
    a->wts = NULL;
  }else{
    a->wts = malloc(a->num_vts * sizeof(stack_uint64_t *));
    assert(a->wts != NULL);
  }
  //initialize stacks pointed from vts and wts arrays
  for (uint64_t i = 0; i < a->num_vts; i++){
    adj_lst_uint64_init_helper(&(a->vts[i]), sizeof(uint64_t));
    if (a->wts != NULL){
      adj_lst_uint64_init_helper(&(a->wts[i]), a->wt_size);
    }
  }
}

static void adj_lst_uint64_init_helper(stack_uint64_t **s, int elt_size){
  uint64_t init_stack_size = 1;
  *s = malloc(sizeof(stack_uint64_t));
  assert(*s != NULL);
  stack_uint64_init(*s, init_stack_size, elt_size, NULL);
}

/**
   Frees an adjacency list and leaves a block of size 
   sizeof(adj_lst_uint64_t) pointed to by the a parameter.
*/
void adj_lst_uint64_free(adj_lst_uint64_t *a){
  for (uint64_t i = 0; i < a->num_vts; i++){
    stack_uint64_free(a->vts[i]);
    free(a->vts[i]);
    a->vts[i] = NULL;
    if (a->wts != NULL){
      stack_uint64_free(a->wts[i]);
      free(a->wts[i]);
      a->wts[i] = NULL;
    }
  }
  free(a->vts); //free(NULL) does nothing
  a->vts = NULL;
  free(a->wts); //free(NULL) does nothing
  a->wts = NULL;
}
   
/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_uint64_dir_build(adj_lst_uint64_t *a, graph_uint64_t *g){
  uint64_t u_ix;
  for (uint64_t i = 0; i < g->num_es; i++){
    //at least one edge
    u_ix = *(u_ptr(g, i));
    stack_uint64_push(a->vts[u_ix], v_ptr(g, i));
    a->num_es++;
    if (a->wts != NULL){
      stack_uint64_push(a->wts[u_ix], wt_ptr(g, i));
    }
  }
}

/**
   Builds the adjacency list of an undirected graph.
*/
void adj_lst_uint64_undir_build(adj_lst_uint64_t *a, graph_uint64_t *g){
  uint64_t u_ix;
  uint64_t v_ix;
  for (uint64_t i = 0; i < g->num_es; i++){
    //at least one edge
    u_ix = *(u_ptr(g, i));
    v_ix = *(v_ptr(g, i));
    stack_uint64_push(a->vts[u_ix], v_ptr(g, i));
    stack_uint64_push(a->vts[v_ix], u_ptr(g, i));
    a->num_es += 2;
    if (a->wts != NULL){
      stack_uint64_push(a->wts[u_ix], wt_ptr(g, i));
      stack_uint64_push(a->wts[v_ix], wt_ptr(g, i));
    }
  }
}

/**
   Adds a directed unweighted edge (u, v) with probability num/denom. If 
   num == denom, there is no overhead of generating a random number.
*/
void adj_lst_uint64_add_dir_edge(adj_lst_uint64_t *a,
				 uint64_t u,
				 uint64_t v,
				 uint32_t num,
				 uint32_t denom){
  assert(num <= denom && denom > 0);
  if (bern_uint32(num, 0, denom)){
    stack_uint64_push(a->vts[u], &v);
    a->num_es++;
  }
}

/**
   Adds an undirected unweighted edge (u, v) with probability nom/denom. If 
   nom == denom, there is no overhead of generating a random number.
*/
void adj_lst_uint64_add_undir_edge(adj_lst_uint64_t *a,
				   uint64_t u,
				   uint64_t v,
				   uint32_t num,
				   uint32_t denom){
  assert(num <= denom && denom > 0);
  if (bern_uint32(num, 0, denom)){
    stack_uint64_push(a->vts[u], &v);
    stack_uint64_push(a->vts[v], &u);
    a->num_es += 2;
  }
}

/**
   Builds the adjacency list of a directed unweighted graph with n vertices,
   where each of n(n - 1) possible edges is added with probability nom/denom.
   If nom == denom, there is no overhead of generating a random number.
*/
void adj_lst_uint64_rand_dir(adj_lst_uint64_t *a,
			     uint64_t n,
			     uint32_t num,
			     uint32_t denom){
  graph_uint64_t g;
  graph_uint64_base_init(&g, n);
  adj_lst_uint64_init(a, &g);
  if (n < 1){return;}
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      adj_lst_uint64_add_dir_edge(a, i, j, num, denom);
      adj_lst_uint64_add_dir_edge(a, j, i, num, denom);
    }
  }
}

/**
   Builds the adjacency list of an undirected unweighted graph with n 
   vertices, where each of n(n - 1)/2 possible edges is added with 
   probability nom/denom. If nom == denom, there is no overhead of generating
   a random number.
*/
void adj_lst_uint64_rand_undir(adj_lst_uint64_t *a,
			       uint64_t n,
			       uint32_t num,
			       uint32_t denom){
  graph_uint64_t g;
  graph_uint64_base_init(&g, n);
  adj_lst_uint64_init(a, &g);
  if (n < 1){return;}
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      adj_lst_uint64_add_undir_edge(a, i, j, num, denom);
    }
  }
}

/** Helper functions */

/**
   Compute pointers to vertices and weights in a graph.
*/
static uint64_t *u_ptr(graph_uint64_t *g, uint64_t i){
  return &(g->u[i]);
}

static uint64_t *v_ptr(graph_uint64_t *g, uint64_t i){
  return &(g->v[i]);
}

static void *wt_ptr(graph_uint64_t *g, uint64_t i){
  return (void *)((char *)g->wts + i * g->wt_size);
}
