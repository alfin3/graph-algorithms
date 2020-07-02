/**
   dfs.c

   Functions for running the DFS algorithm. 

   The implementation emulates the recursion in DFS on a dynamically 
   allocated stack data structure to avoid an overflow of the memory stack.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "dfs.h"
#include "graph.h"
#include "stack.h"

typedef struct{
  int u;
  int vi; //given (u, v), vi is v's index in u's stack in an adjacency list
} u_vi_t;

static void search(adj_lst_t *a, stack_t *s, int *c, int *pre, int *post);
static int next_vi(adj_lst_t *a, u_vi_t p, int *pre);
static int *vt_ptr(void *vts, int i);

/**
   Computes and copies pre and postvisit values to pre and post arrays. 
*/
void dfs(adj_lst_t *a, int *pre, int *post){
  u_vi_t p;
  stack_t s;
  int c = 0; //counter
  int stack_init_size = 1;
  int elt_size = sizeof(u_vi_t);
  for (int i = 0; i < a->num_vts; i++){
    pre[i] = -1; 
    post[i] = -1;
  }
  stack_init(&s, stack_init_size, elt_size, NULL);
  for (int i = 0; i < a->num_vts; i++){
    if (pre[i] < 0){
      pre[i] = c;
      c++;
      p.u = i;
      p.vi = 0;
      stack_push(&s, &p);
      search(a, &s, &c, pre, post);
      assert(s.num_elts == 0);
    }
  }
  stack_free(&s);
}

static void search(adj_lst_t *a, stack_t *s, int *c, int *pre, int *post){
  u_vi_t p;
  int v, vi;
  while (s->num_elts > 0){
    stack_pop(s, &p);
    vi = next_vi(a, p, pre);
    if (vi == a->vts[p.u]->num_elts){
      post[p.u] = *c;
      (*c)++;
    }else{
      p.vi = vi;
      stack_push(s, &p); //push again the unfinished pair
      v = *vt_ptr(a->vts[p.u]->elts, vi);
      pre[v] = *c;
      (*c)++;
      p.u = v;
      p.vi = 0;
      stack_push(s, &p); //then push a pair with an unexplored vertex
    }
  }
}

/** Helper functions */

/**
   Given a u_vi_t pair, computes the index of the next unexplored vertex 
   in u's stack in an adjacency list, in order to emulate the recursion 
   in DFS on a dynamically allocated stack data structure.
*/
static int next_vi(adj_lst_t *a, u_vi_t p, int *pre){
  int v;
  for (int i = p.vi; i < a->vts[p.u]->num_elts; i++){
    v = *vt_ptr(a->vts[p.u]->elts, i);
    if (pre[v] < 0){return i;}
  }
  return a->vts[p.u]->num_elts; //no next valid index
}

/**
   Computes a pointer to an entry in an array of vertices.
*/
static int *vt_ptr(void *vts, int i){
  return (int *)((char *)vts + i * sizeof(int));
}
