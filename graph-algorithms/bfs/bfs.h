/**
   bfs.h

   Declarations of accessible functions for running the BFS algorithm on
   graphs with vertices indexed from 0.
*/

#ifndef BFS_H  
#define BFS_H

#include <stddef.h>
#include "graph.h"

/**
   Computes and copies to an array pointed to by dist the lowest # of edges
   from start to each reached vertex, and provides the previous vertex in the
   array pointed to by prev, with the maximal value of size_t in the prev
   array for unreached vertices. Assumes start is valid and there is at least
   one vertex.
   a           : pointer to an adjacency list with at least one vertex
   start       : a start vertex for running bfs
   dist        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list
   prev        : pointer to a preallocated array with the count equal to the
                 number of vertices in the adjacency list
*/
void bfs(const adj_lst_t *a, size_t start, size_t *dist, size_t *prev);

#endif
