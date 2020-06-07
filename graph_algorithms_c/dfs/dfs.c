/**
   dfs.c

   Functions for running the DFS algorithm. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "stack.h"
#include "graph.h"
#include "dfs.h"

static int *vt_ptr(void *vts, int i);
static void search(adj_lst_t *a, int u, int *c, int *pre, int *post);

/**
   Compute and copy pre and postvisit values to pre and post arrays.
*/
void dfs(adj_lst_t *a, int *pre, int *post){
  int c = 0; //counter
  for (int i = 0; i < a->num_vts; i++){
    pre[i] = -1; //not explored
    post[i] = -1;
  }
  for (int i = 0; i < a->num_vts; i++){
    if (pre[i] < 0){
      search(a, i, &c, pre, post);
    }
  }
}

static void search(adj_lst_t *a, int u, int *c, int *pre, int *post){
  pre[u] = *c;
  (*c)++;
  int v;
  for (int i = 0; i < a->vts[u]->num_elts; i++){
    v = *vt_ptr(a->vts[u]->elts, i);
    if (pre[v] < 0){
      search(a, v, c, pre, post);
    }
  }
  post[u] = *c;
  (*c)++;
}

/** Helper functions */

/**
   Computes a pointer to an entry in an array of vertices.
*/
static int *vt_ptr(void *vts, int i){
  return (int *)((char *)vts + i * sizeof(int));
}
