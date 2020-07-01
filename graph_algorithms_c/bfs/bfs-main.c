/**
   bfs-main.c

   Examples of running the BFS algorithm. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "bfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-ds.h"

static int cmp_int_arrs(int a[], int b[], int n);
void print_test_result(int result);

/**  
   Test bfs on small graphs.
*/

/**
   Initializes the first graph with five vertices.
*/
void first_vfive_graph_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  int num_vts = 5;
  graph_base_init(g, num_vts);
  g->num_es = 4;
  g->u = malloc(g->num_es * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(int));
  assert(g->v != NULL);
  for (int i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Initializes the second graph with five vertices.
*/
void second_vfive_graph_init(graph_t *g){
  int u[] = {0, 1, 2, 3};
  int v[] = {1, 2, 3, 4};
  int num_vts = 5;
  graph_base_init(g, num_vts);
  g->num_es = 4;
  g->u = malloc(g->num_es * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(int));
  assert(g->v != NULL);
  for (int i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Run bfs tests on small graphs.
*/
static void vfive_graph_test_helper(graph_t *g,
				    int ret_dist[][5],
				    int ret_prev[][5],
				    void (*build_fn)(graph_t *, adj_lst_t *),
				    int *result);

void run_first_vfive_graph_test(){
  graph_t g;
  int result = 1;
  int dir_dist[5][5] = {{0, 1, 1, 1, 0},
			{0, 0, 0, 1, 0},
			{0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0}};
  int dir_prev[5][5] = {{0, 0, 0, 0, -1},
			{-1, 1, -1, 1, -1},
			{-1, -1, 2, -1, -1},
			{-1, -1, -1, 3, -1},
			{-1, -1, -1, -1, 4}};
  int undir_dist[5][5] = {{0, 1, 1, 1, 0},
			  {1, 0, 2, 1, 0},
			  {1, 2, 0, 2, 0},
			  {1, 1, 2, 0, 0},
			  {0, 0, 0, 0, 0}};
  int undir_prev[5][5] = {{0, 0, 0, 0, -1},
			  {1, 1, 0, 1, -1},
			  {2, 0, 2, 0, -1},
			  {3, 3, 0, 3, -1},
			  {-1, -1, -1, -1, 4}};
  printf("Run a bfs test on the first small graph instance --> ");
  first_vfive_graph_init(&g);
  vfive_graph_test_helper(&g,
			  dir_dist,
			  dir_prev,
			  adj_lst_dir_build,
			  &result);
  vfive_graph_test_helper(&g,
			  undir_dist,
			  undir_prev,
			  adj_lst_undir_build,
			  &result);
  graph_free(&g);
  print_test_result(result);
}

void run_second_vfive_graph_test(){
  graph_t g;
  int result = 1;
  int dir_dist[5][5] = {{0, 1, 2, 3, 4},
			{0, 0, 1, 2, 3},
			{0, 0, 0, 1, 2},
			{0, 0, 0, 0, 1},
			{0, 0, 0, 0, 0}};
  int dir_prev[5][5] = {{0, 0, 1, 2, 3},
			{-1, 1, 1, 2, 3},
			{-1, -1, 2, 2, 3},
			{-1, -1, -1, 3, 3},
			{-1, -1, -1, -1, 4}};
  int undir_dist[5][5] = {{0, 1, 2, 3, 4},
			  {1, 0, 1, 2, 3},
			  {2, 1, 0, 1, 2},
			  {3, 2, 1, 0, 1},
			  {4, 3, 2, 1, 0}};
  int undir_prev[5][5] = {{0, 0, 1, 2, 3},
			  {1, 1, 1, 2, 3},
			  {1, 2, 2, 2, 3},
			  {1, 2, 3, 3, 3},
			  {1, 2, 3, 4, 4}};
  printf("Run a bfs test on the second small graph instance --> ");
  second_vfive_graph_init(&g);
  vfive_graph_test_helper(&g,
			  dir_dist,
			  dir_prev,
			  adj_lst_dir_build,
			  &result);
  vfive_graph_test_helper(&g,
			  undir_dist,
			  undir_prev,
			  adj_lst_undir_build,
			  &result);
  graph_free(&g);
  print_test_result(result);
}

static void vfive_graph_test_helper(graph_t *g,
				    int ret_dist[][5],
				    int ret_prev[][5],
				    void (*build_fn)(graph_t *, adj_lst_t *),
				    int *result){
  adj_lst_t a;
  int *dist, *prev;
  adj_lst_init(g, &a);
  build_fn(g, &a);
  dist = malloc(a.num_vts * sizeof(int));
  prev = malloc(a.num_vts * sizeof(int));
  for (int i = 0; i < a.num_vts; i++){
    bfs(&a, i, dist, prev);
    *result *= cmp_int_arrs(dist, ret_dist[i], a.num_vts);
    *result *= cmp_int_arrs(prev, ret_prev[i], a.num_vts);
  }
  adj_lst_free(&a);
  free(dist);
  free(prev);
}

/**  
   Test bfs on directed graphs with n(n - 1) edges.
*/

/**
   Runs a bfs test on directed graphs with n(n - 1) edges.
*/
void run_max_edges_graph_test(){
  adj_lst_t a;
  int n, s;
  int pow_two_start = 0;
  int pow_two_end = 14;
  int result = 1;
  int *dist, *prev;
  uint32_t nom = 1;
  uint32_t denom = 1;
  printf("Run a bfs test on graphs with n vertices, where 0 < n <= 2^%d, "
	 "and n(n - 1) edges --> ", pow_two_end);
  fflush(stdout);
  srandom(time(0));
  for (int i = pow_two_start; i <  pow_two_end; i++){
    n = (int)pow_two_uint64(i); // 0 < n
    dist = malloc(n * sizeof(int));
    prev = malloc(n * sizeof(int));
    adj_lst_rand_dir(&a, n, nom, denom); //nom/denom = 1
    s = (int)random_range_uint32((uint32_t)(n - 1)); //if n = 1, s = 0
    bfs(&a, s, dist, prev);
    for (int i = 0; i < n; i++){
      if (i == s){
	result *= (dist[i] == 0);
      }else{
	result *= (dist[i] == 1);
      }
      result *= (prev[i] == s);
    }
    adj_lst_free(&a);
    free(dist);
    free(prev);
  }
  print_test_result(result);
}

/** 
   Test bfs on graphs with no edges.
*/

/**
   Runs a bfs test on directed graphs with no edges.
*/
void run_no_edges_graph_test(){
  adj_lst_t a;
  int n, s;
  int pow_two_start = 0;
  int pow_two_end = 14;
  int result = 1;
  int *dist, *prev;
  uint32_t nom = 0;
  uint32_t denom = 1;
  printf("Run a bfs test on graphs with n vertices, where 0 < n <= 2^%d, "
	 "and no edges --> ", pow_two_end);
  fflush(stdout);
  srandom(time(0));
  for (int i = pow_two_start; i <  pow_two_end; i++){
    n = (int)pow_two_uint64(i); // 0 < n
    dist = malloc(n * sizeof(int));
    prev = malloc(n * sizeof(int));
    adj_lst_rand_dir(&a, n, nom, denom); //nom/denom = 0
    s = (int)random_range_uint32((uint32_t)(n - 1)); //if n = 1, s = 0
    bfs(&a, s, dist, prev);
    for (int i = 0; i < n; i++){
      if (i == s){
	result *= (prev[i] == s);
      }else{
	result *= (prev[i] == -1);
      }
      result *= (dist[i] == 0);
    }
    adj_lst_free(&a);
    free(dist);
    free(prev);
  }
  print_test_result(result);
}

/** 
   Test bfs on random directed graphs.
*/

/**
   Runs a bfs test on random directed graphs.
*/
void run_random_dir_graph_test(){
  adj_lst_t a;
  int n, s;
  int pow_two_start = 10;
  int pow_two_end = 15;
  int num_noms = 3;
  int ave_iter = 10;
  int *dist, *prev;
  uint32_t noms[] = {3, 2, 1};
  uint32_t denom = 4;
  clock_t t;
  printf("Run a bfs test on random directed graphs, from %d random start "
	 "vertices in each graph \n", ave_iter);
  fflush(stdout);
  srandom(time(0));
  for (int nom_ix = 0; nom_ix < num_noms; nom_ix++){
    printf("\tP[an edge is in a graph] = %.4f\n",
	   (float)noms[nom_ix] / (float)denom);
    for (int exp = pow_two_start; exp <  pow_two_end; exp++){
      n = (int)pow_two_uint64(exp); // 0 < n
      dist = malloc(n * sizeof(int));
      prev = malloc(n * sizeof(int));
      adj_lst_rand_dir(&a, n, noms[nom_ix], denom);
      t = clock();
      for (int i = 0; i < ave_iter; i++){
	s = (int)random_range_uint32((uint32_t)(n - 1));
	bfs(&a, s, dist, prev);
      }
      t = clock() - t;
      printf("\t\tvertices: %d, E[# of directed edges]: %.1f, "
	     "average runtime: %.6f seconds\n",
	     n, (float)noms[nom_ix] / (float)denom * (float)(n * (n - 1)),
	     (float)t / ((float)ave_iter * CLOCKS_PER_SEC));
      fflush(stdout);
      adj_lst_free(&a);
      free(dist);
      free(prev);
    }
  }
}

static int cmp_int_arrs(int a[], int b[], int n){
  int result = 1;
  for (int i = 0; i < n; i++){
    result *= (a[i] == b[i]);
  }
  return result;
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_first_vfive_graph_test();
  run_second_vfive_graph_test();
  run_max_edges_graph_test();
  run_no_edges_graph_test();
  run_random_dir_graph_test();
}
