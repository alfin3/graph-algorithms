/**
   bfs-uint64.h

   Declarations of accessible functions for running the BFS algorithm on 
   graphs with the number of vertices > 0 and bounded by 1 + (2^64 - 1) / 
   sizeof(uint64_t), and vertices indexed from 0. The unused upper values are
   reserved for special values.
*/

#ifndef BFS_UINT64_H  
#define BFS_UINT64_H

#include <stdint.h>
#include "graph-uint64.h"

/**
   Computes and copies to dist the lowest # of edges from start to each 
   reached vertex, and provides the previous vertex in prev, with nr in 
   prev for unreached vertices. Assumes immutability of an adjacency list 
   during execution. 
*/
void bfs_uint64(adj_lst_uint64_t *a,
		uint64_t start,
		uint64_t *dist,
		uint64_t *prev);

#endif
