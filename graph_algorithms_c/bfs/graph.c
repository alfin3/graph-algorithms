/**
   graph.c

   Functions for representing a graph with generic weights.

   Adjacency list : 

   Each list is represented by a dynamically growing generic stack.
   A vertex is an int index starting from 0. If a graph has edges and 
   edge weights, edge weights are of any basic type (e.g. char, int, double).
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "stack.h"
#include "graph.h"

static int *u_ptr(graph_t *g, int i);
static int *v_ptr(graph_t *g, int i);
static void *wts_ptr(graph_t *g, int i);
static void free_fn(void *a){}

/**
   Initializes an adjacency list.
*/
static void adj_lst_init_helper(stack_t **s, int elt_size);

void adj_lst_init(graph_t *g, adj_lst_t *a){
  a->num_vts = g->num_vts;
  a->num_e = g->num_e;
  a->wt_size = g->wt_size;
  a->vts = malloc(a->num_vts * sizeof(stack_t *));
  assert(a->vts != NULL);
  if (g->wts != NULL){
    a->wts = malloc(a->num_vts * sizeof(stack_t *));
    assert(a->wts != NULL);
  }else{a->wts = NULL;}
  for (int i = 0; i < a->num_vts; i++){
    adj_lst_init_helper(&(a->vts[i]), sizeof(int));
    if (a->wts != NULL){
      adj_lst_init_helper(&(a->wts[i]), g->wt_size);
    }
  }
}

static void adj_lst_init_helper(stack_t **s, int elt_size){
  int stack_init_size = 1;
  *s = malloc(sizeof(stack_t));
  assert(*s != NULL);
  stack_init(*s, stack_init_size, elt_size, free_fn);
}
   
/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(graph_t *g, adj_lst_t *a){
  int u_ix;
  for (int i = 0; i < g->num_e; i++){
    int u_ix = *(u_ptr(g, i));
    stack_push(a->vts[u_ix], v_ptr(g, i));
    if (a->wts != NULL){
      stack_push(a->wts[u_ix], wts_ptr(g, i));
    }
  }
}

/**
   Builds the adjacency of an undirected graph.
*/
void adj_lst_undir_build(graph_t *g, adj_lst_t *a){
  int u_ix;
  int v_ix;
  for (int i = 0; i < g->num_e; i++){
    u_ix = *(u_ptr(g, i));
    v_ix = *(v_ptr(g, i));
    stack_push(a->vts[u_ix], v_ptr(g, i));
    stack_push(a->vts[v_ix], u_ptr(g, i));
    if (a->wts != NULL){
      stack_push(a->wts[u_ix], wts_ptr(g, i));
      stack_push(a->wts[v_ix], wts_ptr(g, i));
    }
  }
}

/**
   Deallocates the arrays of stacks of an adjacency list.
*/
void adj_lst_free(adj_lst_t *a){
  for (int i = 0; i < a->num_vts; i++){
    stack_free(a->vts[i]);
    if (a->wts != NULL){stack_free(a->wts[i]);}
  }
  free(a->vts);
  a->vts = NULL;
  if (a->wts != NULL){
    free(a->wts);
    a->wts = NULL;
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

static void *wts_ptr(graph_t *g, int i){
  return (void *)((char *)g->wts + i * g->wt_size);
}
