/**
   dfs.h

   Declarations of accessible functions for running the DFS algorithm.

   The implementation emulates the recursion in DFS on a dynamically 
   allocated stack data structure to avoid an overflow of the memory stack.
*/

#ifndef DFS_H  
#define DFS_H

#include "graph.h"

/**
   Computes and copies pre and postvisit values to pre and post arrays.
*/
void dfs(adj_lst_t *a, int *pre, int *post);

#endif
