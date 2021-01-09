/**
   dfs.c

   Functions for running the DFS algorithm on graphs with vertices
   indexed from 0.

   The implementation emulates the recursion in DFS on a dynamically 
   allocated stack data structure to avoid an overflow of the memory stack.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dfs.h"
#include "graph.h"
#include "stack.h"

typedef struct{
  size_t u;
  size_t vi; //given (u, v), vi is v's index in u's stack in an adj. list
} uvi_t;

static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t u,
		   size_t *c,
		   size_t *pre,
		   size_t *post);
static void next_uvi(const adj_lst_t *a, uvi_t *uvi, const size_t *pre);

static const size_t NR = SIZE_MAX; //not reached as index
static const size_t STACK_INIT_COUNT = 1;

/**
   Computes and copies pre and postvisit values to pre and post arrays.
*/
void dfs(const adj_lst_t *a, size_t start, size_t *pre, size_t *post){
  size_t c = 0; //counter
  size_t vt_size = sizeof(size_t);
  size_t uvi_size = sizeof(uvi_t);
  stack_t s;
  memset(pre, 0xff, a->num_vts * vt_size); //initialize both arrays to NR
  memset(post, 0xff, a->num_vts * vt_size);
  stack_init(&s, STACK_INIT_COUNT, uvi_size, NULL);
  for (size_t i = start; i < a->num_vts; i++){
    if (pre[i] == NR){
      search(a, &s, i, &c, pre, post);
    }
  }
  for (size_t i = 0; i < start; i++){
    if (pre[i] == NR){
      search(a, &s, i, &c, pre, post);
    }
  }
  stack_free(&s);
}

//searches from an unexplored vertex u
static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t u,
		   size_t *c,
		   size_t *pre,
		   size_t *post){
  size_t *vts = NULL;
  uvi_t uvi;
  pre[u] = *c;
  (*c)++;
  uvi.u = u;
  uvi.vi = 0;
  stack_push(s, &uvi);
  while (s->num_elts > 0){
    stack_pop(s, &uvi);
    next_uvi(a, &uvi, pre);
    if (uvi.vi == a->vts[uvi.u]->num_elts){
      post[uvi.u] = *c;
      (*c)++;
    }else{
      stack_push(s, &uvi); //push again the unfinished vertex
      vts = a->vts[uvi.u]->elts;
      pre[vts[uvi.vi]] = *c;
      (*c)++;
      uvi.u = vts[uvi.vi];
      uvi.vi = 0;
      stack_push(s, &uvi); //then push an unexplored vertex
    }
  }
}

/** Helper functions */

/**
   Given a uvi_t pair, computes the index of the next unexplored vertex 
   in u's stack in an adjacency list, in order to emulate the recursion 
   in DFS on a dynamically allocated stack data structure.
*/
static void next_uvi(const adj_lst_t *a, uvi_t *uvi, const size_t *pre){
  size_t *vts = a->vts[uvi->u]->elts;
  for (size_t i = uvi->vi; i < a->vts[uvi->u]->num_elts; i++){
    if (pre[vts[i]] == NR){
      uvi->vi = i;
      return;
    }
  }
  uvi->vi = a->vts[uvi->u]->num_elts; //no next valid index
}
