/**
   dfs-uint64.h

   Declarations of accessible functions for running the DFS algorithm on 
   graphs with the number of vertices bounded by 1 + (2^64 - 1) / 
   sizeof(uint64_t) and vertices indexed from 0. The unused upper values are
   reserved for special values. 

   The implementation emulates the recursion in DFS on a dynamically 
   allocated stack data structure to avoid an overflow of the memory stack.
*/

#ifndef DFS_UINT64_H  
#define DFS_UINT64_H

#include <stdint.h>
#include "graph-uint64.h"

/**
   Computes and copies pre and postvisit values to pre and post arrays.
   Assumes immutability of an adjacency list during execution.
*/
void dfs_uint64(adj_lst_uint64_t *a, uint64_t *pre, uint64_t *post);

#endif
