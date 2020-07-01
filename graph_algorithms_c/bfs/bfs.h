/**
   bfs.h

   Declarations of accessible functions for running the BFS algorithm.
*/

#ifndef BFS_H  
#define BFS_H

#include "graph.h"

/**
   Computes and copies to dist the lowest # of edges from s to each reached 
   vertex, and provides the previous vertex in prev, with -1 in prev 
   for unreached vertices.
*/
void bfs(adj_lst_t *a, int s, int *dist, int *prev);

#endif
