/**
   bfs.c

   Functions for running the BFS algorithm on graphs with vertices
   indexed from 0.

   Distance only changed if reached. Otherwise the value is unchanged.
   Therefore, it is guaranteed that if prev is not num_vts the
   corresponding distance is not a trap representation.

   Under C99 and C11, dist can be allocated with calloc and for any
   integer the representation with all zero bits is guaranteed to
   be 0 integer value. 

   Optimization notes: 
   - using a bit array for testing if a vertex was visited in order to
   decrease cache misses did not result in a speed up in tests.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfs.h"
#include "graph.h"
#include "queue.h"
#include "stack.h"
#include "utilities-mem.h"

static const size_t QUEUE_INIT_COUNT = 1;

static void *ptr(const void *block, size_t i, size_t size);

void bfs_incr_ushort(void *a){
  (*(unsigned short *)a)++;
}

void bfs_incr_uint(void *a){
  (*(unsigned int *)a)++;
}

void bfs_incr_ulong(void *a){
  (*(unsigned long *)a)++;
}

void bfs_incr_sz(void *a){
  (*(size_t *)a)++;
}

int bfs_cmpat_ushort(const void *a, const void *i, const void *v){
  return ((const unsigned short *)a)[*(const unsigned short *)i] !=
    *(const unsigned short *)v;
}

int bfs_cmpat_uint(const void *a, const void *i, const void *v){
  return ((const unsigned int *)a)[*(const unsigned int *)i] !=
    *(const unsigned int *)v;
}

int bfs_cmpat_ulong(const void *a, const void *i, const void *v){
  return ((const unsigned long *)a)[*(const unsigned long *)i] !=
    *(const unsigned long *)v;
}

int bfs_cmpat_sz(const void *a, const void *i, const void *v){
  return ((const size_t *)a)[*(const size_t *)i] != *(const size_t *)v;
}

/**
   Computes and copies to an array pointed to by dist the lowest # of edges
   from start to each reached vertex, and provides the previous vertex in the
   array pointed to by prev, with the maximal value of size_t in the prev
   array for unreached vertices. Assumes start is valid and there is at least
   one vertex.
   a           : pointer to an adjacency list with at least one vertex
   start       : a start vertex for running bfs
   dist        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list
   prev        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list
*/
void bfs(const adj_lst_t *a,
	 size_t start,
	 void *dist,
	 void *prev,
	 int (*cmpat_vt)(const void *, const void *, const void *),
	 void (*incr_vt)(void *)){
  char *dp = NULL, *pp = NULL;
  const char *du = NULL;
  const char *p = NULL, *p_start = NULL, *p_end = NULL;
  void *unr = NULL; /* vertex, not reached vertex value */
  void *u = NULL, *nr = NULL;
  queue_t q;
  /* single block for cache-efficiency; same type */
  unr = malloc_perror(2, a->vt_size);
  u = unr;
  nr = ptr(unr, 1, a->vt_size);
  a->write_vt(u, start);
  a->write_vt(nr, a->num_vts);
  a->write_vt(ptr(dist, start, a->vt_size), 0);
  for (pp = prev; pp != ptr(prev, a->num_vts, a->vt_size); pp += a->vt_size){
    memcpy(pp, nr, a->vt_size);
  }
  queue_init(&q, QUEUE_INIT_COUNT, a->vt_size, NULL);
  memcpy(ptr(prev, a->read_vt(u), a->vt_size), u, a->vt_size);
  queue_push(&q, u);
  while (q.num_elts > 0){
    queue_pop(&q, u);
    du = ptr(dist, a->read_vt(u), a->vt_size);
    p_start = a->vt_wts[a->read_vt(u)]->elts;
    p_end = p_start + a->vt_wts[a->read_vt(u)]->num_elts * a->pair_size;
    for (p = p_start; p != p_end; p += a->pair_size){
      if (cmpat_vt(prev, p, nr) == 0){
        dp = ptr(dist, a->read_vt(p), a->vt_size);
        pp = ptr(prev, a->read_vt(p), a->vt_size);
	memcpy(dp, du, a->vt_size);
        incr_vt(dp);
	memcpy(pp, u, a->vt_size);
	queue_push(&q, p);
      }
    }
  }
  queue_free(&q);
  free(unr);
  unr = NULL;
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}
