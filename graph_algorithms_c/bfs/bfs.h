/**
   bfs.h

   Declarations of accessible functions for running the BFS algorithm.
*/

#ifndef BFS_H  
#define BFS_H

#include "graph.h"

/**
   Computes and copies the lowest # of edges from s to dist array and 
   previous vertex to prev array, with -1 in prev for unreached vertices.
*/
void bfs(adj_lst_t *a, int s, int *dist, int *prev);

#endif
