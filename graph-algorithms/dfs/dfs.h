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
   Computes and copies pre and postvisit values to pre and post arrays.
*/
void dfs(const adj_lst_t *a, size_t start, size_t *pre, size_t *post);

#endif
