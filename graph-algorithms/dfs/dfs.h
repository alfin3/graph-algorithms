/**
   dfs.h

   Declarations of accessible functions for running the DFS algorithm on 
   graphs with vertices indexed from 0.

   The implementation emulates the recursion in DFS on a dynamically 
   allocated stack data structure to avoid an overflow of the memory stack.
*/

#ifndef DFS_H  
#define DFS_H

#include <stdlib.h>
#include "graph.h"

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
void dfs(const adj_lst_t *a, size_t start, size_t *pre, size_t *post);

#endif
