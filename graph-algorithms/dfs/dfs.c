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
#include "utilities-mem.h"

typedef struct{
  size_t u;
  const char *vp; /* vp is pointer to v in u's stack in an adj. list */
  const char *vp_end; /* pointer to the end of u's stack */
} uvp_t;

static const size_t STACK_INIT_COUNT = 1;

static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t u,
		   void *c,
		   void *pre,
		   void *post,
		   const void *nr,
                   int (*cmpat_vt)(const void *, const void *, const void *),
		   void (*incr_vt)(void *));
static void *ptr(const void *block, size_t i, size_t size);

void dfs_incr_ushort(void *a){
  (*(unsigned short *)a)++;
}

void dfs_incr_uint(void *a){
  (*(unsigned int *)a)++;
}

void dfs_incr_ulong(void *a){
  (*(unsigned long *)a)++;
}

void dfs_incr_sz(void *a){
  (*(size_t *)a)++;
}

int dfs_cmpat_ushort(const void *a, const void *i, const void *v){
  return ((const unsigned short *)a)[*(const unsigned short *)i] !=
    *(const unsigned short *)v;
}

int dfs_cmpat_uint(const void *a, const void *i, const void *v){
  return ((const unsigned int *)a)[*(const unsigned int *)i] !=
    *(const unsigned int *)v;
}

int dfs_cmpat_ulong(const void *a, const void *i, const void *v){
  return ((const unsigned long *)a)[*(const unsigned long *)i] !=
    *(const unsigned long *)v;
}

int dfs_cmpat_sz(const void *a, const void *i, const void *v){
  return ((const size_t *)a)[*(const size_t *)i] != *(const size_t *)v;
}

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
void dfs(const adj_lst_t *a,
	 size_t start,
	 void *pre,
	 void *post,
	 int (*cmpat_vt)(const void *, const void *, const void *),
	 void (*incr_vt)(void *)){
  size_t u;
  char *p = NULL;
  void *cnri = NULL; /* counter, not reached vertex value, index */
  void *c = NULL, *nr = NULL, *i = NULL;
  stack_t s;
  /* single block for cache-efficiency; same type */
  cnri = malloc_perror(3, a->vt_size);
  c = cnri;
  nr = ptr(cnri, 1, a->vt_size);
  i = ptr(cnri, 2, a->vt_size);
  a->write_vt(c, 0);
  a->write_vt(nr, mul_sz_perror(2, a->num_vts));
  a->write_vt(i, start);
  for (p = pre; p != ptr(pre, a->num_vts, a->vt_size); p += a->vt_size){
    memcpy(p, nr, a->vt_size);
  }
  stack_init(&s, STACK_INIT_COUNT, sizeof(uvp_t), NULL);
  for (u = start; u < a->num_vts; u++){
    if (cmpat_vt(pre, i, nr) == 0){
      search(a, &s, u, c, pre, post, nr, cmpat_vt, incr_vt);
    }
    incr_vt(i);
  }
  a->write_vt(i, 0);
  for (u = 0; u < start; u++){
    if (cmpat_vt(pre, i, nr) == 0){
      search(a, &s, u, c, pre, post, nr, cmpat_vt, incr_vt);
    }
    incr_vt(i);
  }
  stack_free(&s);
  free(cnri);
  cnri = NULL;
}

/**
   Performs a DFS search of a graph component reachable from an unexplored
   vertex provided by the u parameter by emulating the recursion in DFS on
   a dynamically allocated stack data structure.
*/
static void search(const adj_lst_t *a,
		   stack_t *s,
		   size_t u,
		   void *c,
		   void *pre,
		   void *post,
		   const void *nr,
                   int (*cmpat_vt)(const void *, const void *, const void *),
		   void (*incr_vt)(void *)){
  const char *p = NULL;
  uvp_t uvp;
  memcpy(ptr(pre, u, a->vt_size), c, a->vt_size);
  incr_vt(c);
  uvp.u = u;
  uvp.vp = a->vt_wts[uvp.u]->elts;
  uvp.vp_end = uvp.vp + a->vt_wts[uvp.u]->num_elts * a->pair_size;
  stack_push(s, &uvp);
  while (s->num_elts > 0){
    stack_pop(s, &uvp);
    p = uvp.vp;
    while (p != uvp.vp_end && cmpat_vt(pre, p, nr) != 0){
      p += a->pair_size;
    }
    uvp.vp = p;
    if (uvp.vp == uvp.vp_end){
      memcpy(ptr(post, uvp.u, a->vt_size), c, a->vt_size);
      incr_vt(c);
    }else{
      stack_push(s, &uvp); /* push the unfinished vertex */
      memcpy(ptr(pre, a->read_vt(uvp.vp), a->vt_size), c, a->vt_size);
      incr_vt(c);
      uvp.u = a->read_vt(uvp.vp);
      uvp.vp = a->vt_wts[uvp.u]->elts;
      uvp.vp_end = uvp.vp + a->vt_wts[uvp.u]->num_elts * a->pair_size;
      stack_push(s, &uvp); /* then push an unexplored vertex */
    }
  }
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}
