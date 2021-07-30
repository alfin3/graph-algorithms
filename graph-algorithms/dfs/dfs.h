/**
   dfs.h

   Declarations of accessible functions for running the DFS algorithm on 
   graphs with vertices indexed from 0.

   The implementation emulates the recursion in DFS on a dynamically 
   allocated stack data structure to avoid an overflow of the memory stack.
*/

#ifndef DFS_H  
#define DFS_H

#include <stddef.h>
#include "graph.h"

void dfs_incr_ushort(void *a);
void dfs_incr_uint(void *a);
void dfs_incr_ulong(void *a);
void dfs_incr_sz(void *a);

int dfs_cmpat_ushort(const void *a, const void *i, const void *v);
int dfs_cmpat_uint(const void *a, const void *i, const void *v);
int dfs_cmpat_ulong(const void *a, const void *i, const void *v);
int dfs_cmpat_sz(const void *a, const void *i, const void *v);

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
void dfs(const adj_lst_t *a, size_t start, void *pre, void *post,
int (*cmpat_vt)(const void *, const void *, const void *), 
void (*incr_vt)(void *));

#endif
