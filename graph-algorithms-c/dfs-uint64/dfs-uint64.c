/**
   dfs-uint64.c

   Functions for running the DFS algorithm on graphs with the number of 
   vertices bounded by 2 + (2^64 - 1) / sizeof(uint64_t) and vertices
   indexed from 0. The unused upper values are reserved for special values. 

   The implementation emulates the recursion in DFS on a dynamically 
   allocated stack data structure to avoid an overflow of the memory stack.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "dfs-uint64.h"
#include "graph-uint64.h"
#include "stack-uint64.h"

typedef struct{
  uint64_t u;
  uint64_t vi; //given (u, v), vi is v's index in u's stack in an adj. list
} u_vi_t;

static void search(adj_lst_uint64_t *a,
		   stack_uint64_t *s,
		   uint64_t *c,
		   uint64_t *pre,
		   uint64_t *post);
static void next_p(adj_lst_uint64_t *a, u_vi_t *p, uint64_t *pre);
static uint64_t *vt_ptr(void *vts, uint64_t i);

static const uint64_t not_reached = 0xffffffffffffffff;
static const uint64_t l_num_vts = 2 + 0xffffffffffffffff / sizeof(uint64_t);

/**
   Computes and copies pre and postvisit values to pre and post arrays. 
*/
void dfs_uint64(adj_lst_uint64_t *a, uint64_t *pre, uint64_t *post){
  assert(a->num_vts < l_num_vts);
  u_vi_t p;
  stack_uint64_t s;
  uint64_t c = 0; //counter
  uint64_t init_stack_size = 1;
  int elt_size = sizeof(u_vi_t);
  for (uint64_t i = 0; i < a->num_vts; i++){
    pre[i] = not_reached; 
    post[i] = not_reached;
  }
  stack_uint64_init(&s, init_stack_size, elt_size, NULL);
  for (uint64_t i = 0; i < a->num_vts; i++){
    if (pre[i] == not_reached){
      pre[i] = c;
      c++;
      p.u = i;
      p.vi = 0;
      stack_uint64_push(&s, &p);
      search(a, &s, &c, pre, post);
      assert(s.num_elts == 0);
    }
  }
  stack_uint64_free(&s);
}

static void search(adj_lst_uint64_t *a,
		   stack_uint64_t *s,
		   uint64_t *c,
		   uint64_t *pre,
		   uint64_t *post){
  u_vi_t p;
  uint64_t v;
  while (s->num_elts > 0){
    stack_uint64_pop(s, &p);
    next_p(a, &p, pre);
    if (p.vi == a->vts[p.u]->num_elts){
      post[p.u] = *c;
      (*c)++;
    }else{
      stack_uint64_push(s, &p); //push again the unfinished vertex
      v = *vt_ptr(a->vts[p.u]->elts, p.vi);
      pre[v] = *c;
      (*c)++;
      p.u = v;
      p.vi = 0;
      stack_uint64_push(s, &p); //then push an unexplored vertex
    }
  }
}

/** Helper functions */

/**
   Given a u_vi_t pair, computes the index of the next unexplored vertex 
   in u's stack in an adjacency list, in order to emulate the recursion 
   in DFS on a dynamically allocated stack data structure.
*/
static void next_p(adj_lst_uint64_t *a, u_vi_t *p, uint64_t *pre){
  uint64_t v;
  for (uint64_t i = p->vi; i < a->vts[p->u]->num_elts; i++){
    v = *vt_ptr(a->vts[p->u]->elts, i);
    if (pre[v] == not_reached){
      p->vi = i;
      return;
    }
  }
  p->vi = a->vts[p->u]->num_elts; //no next valid index
}

/**
   Computes a pointer to an entry in an array of vertices.
*/
static uint64_t *vt_ptr(void *vts, uint64_t i){
  return (uint64_t *)((char *)vts + i * sizeof(uint64_t));
}
