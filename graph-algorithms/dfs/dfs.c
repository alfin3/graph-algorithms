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
#include "dfs.h"
#include "graph.h"
#include "stack.h"

typedef struct{
  size_t u;
  char *vp; /* given (u, v), vp is pointer to v in u's stack in an adj. list */
  char *vp_end; /* pointer to the end of u's stack */
} uvp_t;

static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t u,
		   size_t *c,
		   size_t *pre,
		   size_t *post);
static void move_uvp(const adj_lst_t *a, uvp_t *uvp, const size_t *pre);

static const size_t NR = (size_t)-1; /* not reached as index */
static const size_t STACK_INIT_COUNT = 1;

/**
   Computes and copies to the arrays pointed to by pre and post the previsit
   and postvisit values of a DFS search from a start vertex. Assumes start
   is valid and there is at least one vertex.
   a           : pointer to an adjacency list with at least one vertex
   start       : a start vertex for running dfs
   pre         : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list
   post        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list
*/
void dfs(const adj_lst_t *a, size_t start, size_t *pre, size_t *post){
  size_t c = 0; /* counter */
  size_t vt_size = sizeof(size_t);
  size_t i;
  stack_t s;
  memset(pre, 0xff, a->num_vts * vt_size); /* initialize both arrays to NR */
  memset(post, 0xff, a->num_vts * vt_size);
  stack_init(&s, STACK_INIT_COUNT, sizeof(uvp_t), NULL);
  for (i = start; i < a->num_vts; i++){
    if (pre[i] == NR){
      search(a, &s, i, &c, pre, post);
    }
  }
  for (i = 0; i < start; i++){
    if (pre[i] == NR){
      search(a, &s, i, &c, pre, post);
    }
  }
  stack_free(&s);
}

/**
   Performs a DFS search of a graph component reachable from an unexplored
   vertex provided by the u parameter by emulating the recursion in DFS on
   a dynamically allocated stack data structure.
*/
static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t u,
		   size_t *c,
		   size_t *pre,
		   size_t *post){
  uvp_t uvp;
  pre[u] = *c;
  (*c)++;
  uvp.u = u;
  uvp.vp = a->vt_wts[uvp.u]->elts;
  uvp.vp_end = uvp.vp + a->vt_wts[uvp.u]->num_elts * a->step_size;
  stack_push(s, &uvp);
  while (s->num_elts > 0){
    stack_pop(s, &uvp);
    move_uvp(a, &uvp, pre);
    if (uvp.vp == uvp.vp_end){
      post[uvp.u] = *c;
      (*c)++;
    }else{
      stack_push(s, &uvp); /* push the unfinished vertex */
      pre[*(size_t *)uvp.vp] = *c;
      (*c)++;
      uvp.u = *(size_t *)uvp.vp;
      uvp.vp = a->vt_wts[uvp.u]->elts;
      uvp.vp_end = uvp.vp + a->vt_wts[uvp.u]->num_elts * a->step_size;
      stack_push(s, &uvp); /* then push an unexplored vertex */
    }
  }
}

/**
   Updates a uvp_t by computing a pointer to the next unexplored vertex 
   in u's stack in an adjacency list.
*/
static void move_uvp(const adj_lst_t *a, uvp_t *uvp, const size_t *pre){
  char *p = NULL;
  for (p = uvp->vp; p < uvp->vp_end; p += a->step_size){
    if (pre[*(size_t *)p] == NR){
      uvp->vp = p;
      return;
    }
  }
  uvp->vp = uvp->vp_end;
}
