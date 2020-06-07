/**
   dfs.h

   Declarations of accessible functions for running the DFS algorithm.
*/

#ifndef DFS_H  
#define DFS_H

#include "graph.h"

/**
   Computes and copies pre and postvisit values to pre and post arrays.
*/
void dfs(adj_lst_t *a, int *pre, int *post);

#endif
