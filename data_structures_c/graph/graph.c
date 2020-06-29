/**
   graph.c

   Functions for representing a graph with generic weights.

   Adjacency list : 

   Each list in an adjacency list is represented by a dynamically growing 
   generic stack. A vertex is an int index starting from 0. If a graph has 
   edge weights, the edge weights are of any basic type (e.g. char, int, 
   double). 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "utilities-ds.h"
#include "stack.h"
#include "graph.h"

static int *u_ptr(graph_t *g, int i);
static int *v_ptr(graph_t *g, int i);
static void *wt_ptr(graph_t *g, int i);

/**
   Initializes a graph with n vertices and no edges, where n >= 0.
*/
void graph_base_init(graph_t *g, int n){
  g->num_vts = n;
  g->num_es = 0;
  g->wt_size = 0;
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Frees the dynamically allocated arrays of a graph, and sets all int 
   variables to 0.
*/
void graph_free(graph_t *g){
  g->num_vts = 0;
  g->num_es = 0;
  g->wt_size = 0;
  free(g->u); //free(NULL) does nothing
  free(g->v);
  free(g->wts);
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Initializes an adjacency list.
*/
static void adj_lst_init_helper(stack_t **s, int elt_size);

void adj_lst_init(graph_t *g, adj_lst_t *a){
  a->num_vts = g->num_vts;
  a->num_es = 0; //increment in build and edge add functions
  a->wt_size = g->wt_size;
  //no vertices
  if (g->num_vts == 0){
    a->vts = NULL;
  }else{
    a->vts = malloc(a->num_vts * sizeof(stack_t *));
    assert(a->vts != NULL);
  }
  //no weights
  if (g->wts == NULL){
    a->wts = NULL;
  }else{
    a->wts = malloc(a->num_vts * sizeof(stack_t *));
    assert(a->wts != NULL);
  }
  for (int i = 0; i < a->num_vts; i++){
    //at least one vertex
    adj_lst_init_helper(&(a->vts[i]), sizeof(int));
    if (a->wts != NULL){
      adj_lst_init_helper(&(a->wts[i]), g->wt_size);
    }
  }
}

static void adj_lst_init_helper(stack_t **s, int elt_size){
  int init_stack_size = 1;
  *s = malloc(sizeof(stack_t));
  assert(*s != NULL);
  stack_init(*s, init_stack_size, elt_size, NULL);
}

/**
   Frees an adjacency list. Deallocates the arrays in the stacks of 
   an adjacency list and the stack pointer arrays of the adjacency list, 
   and sets all int variables to 0. 
*/
void adj_lst_free(adj_lst_t *a){
  for (int i = 0; i < a->num_vts; i++){
    stack_free(a->vts[i]);
    if (a->wts != NULL){stack_free(a->wts[i]);}
  }
  a->num_vts = 0;
  a->num_es = 0;
  a->wt_size = 0;
  free(a->vts); //free(NULL) does nothing
  a->vts = NULL;
  free(a->wts); //free(NULL) does nothing
  a->wts = NULL;
}
   
/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(graph_t *g, adj_lst_t *a){
  int u_ix;
  for (int i = 0; i < g->num_es; i++){
    //at least one edge
    u_ix = *(u_ptr(g, i));
    stack_push(a->vts[u_ix], v_ptr(g, i));
    a->num_es++;
    if (a->wts != NULL){
      stack_push(a->wts[u_ix], wt_ptr(g, i));
    }
  }
}

/**
   Builds the adjacency list of an undirected graph.
*/
void adj_lst_undir_build(graph_t *g, adj_lst_t *a){
  int u_ix;
  int v_ix;
  for (int i = 0; i < g->num_es; i++){
    //at least one edge
    u_ix = *(u_ptr(g, i));
    v_ix = *(v_ptr(g, i));
    stack_push(a->vts[u_ix], v_ptr(g, i));
    stack_push(a->vts[v_ix], u_ptr(g, i));
    a->num_es += 2;
    if (a->wts != NULL){
      stack_push(a->wts[u_ix], wt_ptr(g, i));
      stack_push(a->wts[v_ix], wt_ptr(g, i));
    }
  }
}

/**
   Adds a directed unweighted edge (u, v) with probability nom/denom. If 
   nom = denom, there is no overhead of generating a random number.
*/
void adj_lst_add_dir_edge(adj_lst_t *a,
			  int u,
			  int v,
			  uint32_t nom,
			  uint32_t denom){
  assert(nom <= denom && denom > 0);
  if (bern_uint32(nom, 0, denom)){
    stack_push(a->vts[u], &v);
    a->num_es++;
  }
}

/**
   Adds an undirected unweighted edge (u, v) with probability nom/denom. If 
   nom = denom, there is no overhead of generating a random number.
*/
void adj_lst_add_undir_edge(adj_lst_t *a,
			    int u,
			    int v,
			    uint32_t nom,
			    uint32_t denom){
  assert(nom <= denom && denom > 0);
  if (bern_uint32(nom, 0, denom)){
    stack_push(a->vts[u], &v);
    stack_push(a->vts[v], &u);
    a->num_es += 2;
  }
}

/**
   Builds the adjacency list of a directed graph with n vertices, where each
   of n(n - 1) possible edges is added with probability nom/denom. If 
   nom = denom, there is no overhead of generating a random number.
*/
void adj_lst_rand_dir(adj_lst_t *a, int n, uint32_t nom, uint32_t denom){
  graph_t g;
  graph_base_init(&g, n); //no dynamic allocation
  adj_lst_init(&g, a);
  adj_lst_dir_build(&g, a);
  if (n < 1){return;} //if the type of n is changed in later versions
  for (int i = 0; i < n - 1; i++){
    for (int j = i + 1; j < n; j++){
      adj_lst_add_dir_edge(a, i, j, nom, denom);
      adj_lst_add_dir_edge(a, j, i, nom, denom);
    }
  }
}

/**
   Builds the adjacency list of an undirected graph with n vertices, where 
   each of n(n - 1)/2 possible edges is added with probability nom/denom. 
   If nom = denom, there is no overhead of generating a random number.
*/
void adj_lst_rand_undir(adj_lst_t *a, int n, uint32_t nom, uint32_t denom){
  graph_t g;
  graph_base_init(&g, n); //no dynamic allocation
  adj_lst_init(&g, a);
  adj_lst_undir_build(&g, a);
  if (n < 1){return;} //if the type of n is changed in later versions
  for (int i = 0; i < n - 1; i++){
    for (int j = i + 1; j < n; j++){
      adj_lst_add_undir_edge(a, i, j, nom, denom);
    }
  }
}

/** Helper functions */

/**
   Compute pointers to vertices and weights in a graph struct.
*/
static int *u_ptr(graph_t *g, int i){
  return &(g->u[i]);
}

static int *v_ptr(graph_t *g, int i){
  return &(g->v[i]);
}

static void *wt_ptr(graph_t *g, int i){
  return (void *)((char *)g->wts + i * g->wt_size);
}
