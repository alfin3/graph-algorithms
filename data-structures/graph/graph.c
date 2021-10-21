/**
   graph.c

   Functions for representing a graph with generic integer vertices and
   generic weights. 

   Each list in an adjacency list is represented by a dynamically growing 
   stack. A vertex is of any integer type with values starting from 0. If
   a graph is weighted, an edge weight is an object within a contiguous 
   memory block, such as an object of basic type (e.g. char, int, double)
   or a struct (e.g. two unsigned integers).
  
   A single stack of adjacent vertex weight pairs with adjustable alignment
   in memory is used to achieve cache efficiency in graph algorithms.
   Depending on the problem size and a given system, the choice of an integer
   type with a smaller size for vertices  may provide additional
   cache efficiency in addition to reducing space requirements.

   The implementation only uses integer and pointer operations (any non-
   integer operations on weights are defined by the user). Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient
   resources. The behavior outside the specified parameter ranges is
   undefined.

   The implementation does not use stdint.h and is portable under
   C89/C90 and C99.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

const size_t STACK_INIT_COUNT = 1;

/**
   Initializes a weighted or unweighted graph with num_vts vertices and
   no edges, providing a basis for graph construction.
   g           : pointer to a preallocated block of size sizeof(graph_t)
   num_vts     : number of vertices
   vt_size     : > 0 size of the integer type used to represent a vertex
   wt_size     : > 0 size of object used to represent a weight, if a graph
                 is weighted; 0 if the graph is not weighted
   read_vt     : non-NULL pointer to a function for reading a vertex, which
                 may be one of the provided functions
   write_vt    : non-NULL pointer to a function for writing a vertex, which
                 may be one of the provided functions
*/
void graph_base_init(graph_t *g,
		     size_t num_vts,
		     size_t vt_size,
		     size_t wt_size,
		     const graph_vto_t *vto){
  g->num_vts = num_vts;
  g->num_es = 0;
  g->vt_size = vt_size;
  g->wt_size = wt_size;
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
  g->vto = vto;
}

/**
   Frees a graph and leaves a block of size sizeof(graph_t) pointed to by 
   the g parameter.
*/
void graph_free(graph_t *g){
  free(g->u); /* free(NULL) performs no operation */
  free(g->v);
  free(g->wts);
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Initializes an empty adjacency list according to a graph. Aligns vertices
   and weights according to their sizes, which may result in overalignment.
   a           : pointer to a preallocated block of size sizeof(adj_lst_t)
   g           : pointer to a graph previously constructed with at least
                 graph_base_init
*/
void adj_lst_base_init(adj_lst_t *a, const graph_t *g){
  size_t i;
  size_t wt_rem, vt_rem;
  a->num_vts = g->num_vts;
  a->num_es = 0;
  a->vt_size = g->vt_size;
  a->wt_size = g->wt_size;
  /* align weight relative to a malloc's pointer and compute pair_size */
  if (a->wt_size == 0){
    a->wt_offset = a->vt_size;
  }else if (a->vt_size <= a->wt_size){
    a->wt_offset = a->wt_size;
  }else{
    wt_rem = a->vt_size % a->wt_size;
    a->wt_offset = add_sz_perror(a->vt_size,
				 (wt_rem > 0) * (a->wt_size - wt_rem)); 
  }
  vt_rem = add_sz_perror(a->wt_offset, a->wt_size) % a->vt_size;
  a->pair_size = add_sz_perror(a->wt_offset + a->wt_size,
			       (vt_rem > 0) * (a->vt_size - vt_rem));
  a->buf = calloc_perror(1, a->pair_size);
  a->vt_wts = NULL;
  if (a->num_vts > 0){
    a->vt_wts = malloc_perror(a->num_vts, sizeof(stack_t *));
  }
  /* initialize stacks */
  for (i = 0; i < a->num_vts; i++){
    a->vt_wts[i] = malloc_perror(1, sizeof(stack_t));
    stack_init(a->vt_wts[i], STACK_INIT_COUNT, a->pair_size, NULL);
  }
  a->vto = g->vto;
}

/**
   Aligns the vertices and weights of an adjacency list according to 
   the values of the alignment parameters. If the alignment requirement
   of only one type is known, then the size of the other type can be used
   as a value of the other alignment parameter because size of
   type >= alignment requirement of type (due to structure of arrays), 
   which may result in overalignment. The call to this operation may 
   result in reduction of space requirements as compared to adj_lst_base_init
   alone. The operation is optionally called after adj_lst_base_init is
   completed and before any other adj_list_ operation is called. 
   a            : pointer to a adj_lst_t struct initialized with
                  adj_lst_base_init
   vt_alignment : alignment requirement or size of the integer type of
                  a vertex
   wt_alignment : alignment requirement or size of the type of a weight
*/
void adj_lst_align(adj_lst_t *a,
		   size_t vt_alignment,
		   size_t wt_alignment){
  size_t i;
  size_t wt_rem, vt_rem;
  if (a->wt_size == 0){
    a->wt_offset = a->vt_size;
  }else if (a->vt_size <= wt_alignment){
    a->wt_offset = wt_alignment;
  }else{
    wt_rem = a->vt_size % wt_alignment;
    a->wt_offset = add_sz_perror(a->vt_size,
				 (wt_rem > 0) * (wt_alignment - wt_rem));
  }
  vt_rem = add_sz_perror(a->wt_offset, a->wt_size) % vt_alignment;
  a->pair_size = add_sz_perror(a->wt_offset + a->wt_size,
			       (vt_rem > 0) * (vt_alignment - vt_rem));
  a->buf = realloc_perror(a->buf, 1, a->pair_size);
  memset(a->buf, 0, a->pair_size);
  /* initialize stacks */
  for (i = 0; i < a->num_vts; i++){
    stack_free(a->vt_wts[i]);
    stack_init(a->vt_wts[i], STACK_INIT_COUNT, a->pair_size, NULL);
  }
}
   
/**
   Builds the adjacency list of a directed graph.
*/
void adj_lst_dir_build(adj_lst_t *a, const graph_t *g){
  size_t i;
  const char *u = g->u;
  const char *v = g->v;
  const char *wt = g->wts;
  char *buf_wt = (char *)a->buf + a->wt_offset;
  for (i = 0; i < g->num_es; i++){
    memcpy(a->buf, v, a->vt_size);
    if (a->wt_size > 0 && wt != NULL){
      memcpy(buf_wt, wt, a->wt_size);
      wt += a->wt_size;
    }
    stack_push(a->vto->at(a->vt_wts, u), a->buf);
    a->num_es++;
    u += a->vt_size;
    v += a->vt_size;
  }
}

/**
   Builds the adjacency list of an undirected graph.
*/
void adj_lst_undir_build(adj_lst_t *a, const graph_t *g){
  size_t i;
  const char *u = g->u;
  const char *v = g->v;
  const char *wt = g->wts;
  char *buf_wt = (char *)a->buf + a->wt_offset;
  for (i = 0; i < g->num_es; i++){
    memcpy(a->buf, v, a->vt_size);
    if (a->wt_size > 0 && wt != NULL){
      memcpy(buf_wt, wt, a->wt_size);
      wt += a->wt_size;
    }
    stack_push(a->vto->at(a->vt_wts, u), a->buf);
    memcpy(a->buf, u, a->vt_size);
    stack_push(a->vto->at(a->vt_wts, v), a->buf);
    a->num_es += 2;
    u += a->vt_size;
    v += a->vt_size;
  }
}

/**
   Adds a directed edge (u, v) according to the Bernoulli distribution
   provided by bern that takes arg as its parameter. The edge is added if
   bern returns nonzero. If a graph is weighted, wt points to a weight
   of size wt_size, otherwise wt is NULL.
*/
void adj_lst_add_dir_edge(adj_lst_t *a,
			  size_t u,
			  size_t v,
			  const void *wt,
			  int (*bern)(void *),
			  void *arg){
  if (bern(arg)){
    a->vto->write(a->buf, v);
    if (a->wt_size > 0 && wt != NULL){
      memcpy((char *)a->buf + a->wt_offset, wt, a->wt_size);
    }
    stack_push(a->vt_wts[u], a->buf);
    a->num_es++;
  }
}

/**
   Adds an undirected edge (u, v) according to the Bernoulli distribution
   provided by bern that takes arg as its parameter. The edge is added if
   bern returns nonzero. If a graph is weighted, wt points to a weight of
   size wt_size, otherwise wt is NULL.
*/
void adj_lst_add_undir_edge(adj_lst_t *a,
			    size_t u,
			    size_t v,
			    const void *wt,
			    int (*bern)(void *),
			    void *arg){
  if (bern(arg)){
    a->vto->write(a->buf, v);
    if (a->wt_size > 0 && wt != NULL){
      memcpy((char *)a->buf + a->wt_offset, wt, a->wt_size);
    }
    stack_push(a->vt_wts[u], a->buf);
    a->vto->write(a->buf, u);
    stack_push(a->vt_wts[v], a->buf);
    a->num_es += 2;
  }
}

/**
   Builds the adjacency list of a directed unweighted graph with num_vts
   vertices, where each of num_vts(num_vts - 1) possible edges is added
   according to the Bernoulli distribution provided by bern that takes arg
   as its parameter. An edge is added if bern returns nonzero. a is pointer
   to a preallocated block of size sizeof(adj_lst_t), not initialized.
   Please see the parameter specification in graph_base_init and
   adj_lst_base_init.
*/
void adj_lst_rand_dir(adj_lst_t *a,
		      size_t num_vts,
		      size_t vt_size,
		      const graph_vto_t *vto,
		      int (*bern)(void *),
		      void *arg){
  size_t i, j;
  graph_t g;
  graph_base_init(&g, num_vts, vt_size, 0, vto);
  adj_lst_base_init(a, &g);
  if (num_vts > 0){
    for (i = 0; i < num_vts - 1; i++){
      for (j = i + 1; j < num_vts; j++){
	adj_lst_add_dir_edge(a, i, j, NULL, bern, arg);
	adj_lst_add_dir_edge(a, j, i, NULL, bern, arg);
      }
    }
  }
}

/**
   Builds the adjacency list of an undirected unweighted graph with num_vts 
   vertices, where each of num_vts(num_vts - 1)/2 possible edges is added
   according to the Bernoulli distribution provided by bern that takes arg
   as its parameter. An edge is added if bern returns nonzero. a is pointer
   to a preallocated block of size sizeof(adj_lst_t), not initialized.
   Please see the parameter specification in graph_base_init and
   adj_lst_base_init.
*/
void adj_lst_rand_undir(adj_lst_t *a,
			size_t num_vts,
			size_t vt_size,
		        const graph_vto_t *vto,
			int (*bern)(void *),
			void *arg){
  size_t i, j;
  graph_t g;
  graph_base_init(&g, num_vts, vt_size, 0, vto);
  adj_lst_base_init(a, &g);
  if (num_vts > 0){
    for (i = 0; i < num_vts - 1; i++){
      for (j = i + 1; j < num_vts; j++){
	adj_lst_add_undir_edge(a, i, j, NULL, bern, arg);
      }
    }
  }
}

/**
   Frees an adjacency list and leaves a block of size sizeof(adj_lst_t)
   pointed to by the a parameter.
*/
void adj_lst_free(adj_lst_t *a){
  size_t i;
  for (i = 0; i < a->num_vts; i++){
    stack_free(a->vt_wts[i]);
    free(a->vt_wts[i]);
    a->vt_wts[i] = NULL;
  }
  free(a->buf);
  free(a->vt_wts); /* free(NULL) performs no operation */
  a->buf = NULL;
  a->vt_wts = NULL;
}

/**
   Read vertices of different integer types.
*/

size_t graph_read_uchar(const void *a){
  return (size_t)(*(const unsigned char *)a);
}
size_t graph_read_ushort(const void *a){
  return (size_t)(*(const unsigned short *)a);
}
size_t graph_read_uint(const void *a){
  return (size_t)(*(const unsigned int *)a);
}
size_t graph_read_ulong(const void *a){
  return (size_t)(*(const unsigned long *)a);
}
size_t graph_read_sz(const void *a){
  return *(const size_t *)a;
}

/**
   Write vertices of different integer types.
*/

void graph_write_uchar(void *a, size_t val){
  *(unsigned char *)a = val;
}
void graph_write_ushort(void *a, size_t val){
  *(unsigned short *)a = val;
}
void graph_write_uint(void *a, size_t val){
  *(unsigned int *)a = val;
}
void graph_write_ulong(void *a, size_t val){
  *(unsigned long *)a = val;
}
void graph_write_sz(void *a, size_t val){
  *(size_t *)a = val;
}

/**
   Increment values of integer type of vertices.
*/

void graph_incr_uchar(void *a){
  (*(unsigned char *)a)++;
}
void graph_incr_ushort(void *a){
  (*(unsigned short *)a)++;
}
void graph_incr_uint(void *a){
  (*(unsigned int *)a)++;
}
void graph_incr_ulong(void *a){
  (*(unsigned long *)a)++;
}
void graph_incr_sz(void *a){
  (*(size_t *)a)++;
}

/**
   Get pointer to an element in the array pointed by the first argument at
   the index pointed to by the second argument; each argument points to a
   value of the integer type used to represent vertices.
*/

stack_t *graph_at_uchar(stack_t * const *s, const void *i){
  return (stack_t *)s[*(const unsigned char *)i];
}
stack_t *graph_at_ushort(stack_t * const *s, const void *i){
  return (stack_t *)s[*(const unsigned short *)i];
}
stack_t *graph_at_uint(stack_t * const *s, const void *i){
  return (stack_t *)s[*(const unsigned int *)i];
}
stack_t *graph_at_ulong(stack_t * const *s, const void *i){
  return (stack_t *)s[*(const unsigned long *)i];
}
stack_t *graph_at_sz(stack_t * const *s, const void *i){
  return (stack_t *)s[*(const size_t *)i];
}

/**
   Comparing the element in the array pointed to by the first argument at
   the index pointed to by the second argument, to the value pointed to
   by the third argument; each argument points to a value of the integer
   type used to represent vertices.
*/

int graph_cmpat_uchar(const void *a, const void *i, const void *v){
  return ((const unsigned char *)a)[*(const unsigned char *)i] !=
    *(const unsigned char *)v;
}
int graph_cmpat_ushort(const void *a, const void *i, const void *v){
  return ((const unsigned short *)a)[*(const unsigned short *)i] !=
    *(const unsigned short *)v;
}
int graph_cmpat_uint(const void *a, const void *i, const void *v){
  return ((const unsigned int *)a)[*(const unsigned int *)i] !=
    *(const unsigned int *)v;
}
int graph_cmpat_ulong(const void *a, const void *i, const void *v){
  return ((const unsigned long *)a)[*(const unsigned long *)i] !=
    *(const unsigned long *)v;
}
int graph_cmpat_sz(const void *a, const void *i, const void *v){
  return ((const size_t *)a)[*(const size_t *)i] != *(const size_t *)v;
}
