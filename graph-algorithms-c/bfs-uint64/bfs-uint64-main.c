/**
   bfs-uint64-main.c

   Examples of running the BFS algorithm. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "bfs-uint64.h"
#include "graph-uint64.h"
#include "stack-uint64.h"
#include "utilities-ds.h"

static int cmp_uint64_arrs(uint64_t a[], uint64_t b[], uint64_t n);
void print_test_result(int result);

static const uint64_t nr = 0xffffffffffffffff; //not reached

/**  
   Test bfs_uint64 on small graphs.
*/

/**
   Initializes the first graph with five vertices.
*/
void first_vfive_graph_init(graph_uint64_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t num_vts = 5;
  graph_uint64_base_init(g, num_vts);
  g->num_es = 4;
  g->u = malloc(g->num_es * sizeof(uint64_t));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(uint64_t));
  assert(g->v != NULL);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Initializes the second graph with five vertices.
*/
void second_vfive_graph_init(graph_uint64_t *g){
  uint64_t u[] = {0, 1, 2, 3};
  uint64_t v[] = {1, 2, 3, 4};
  uint64_t num_vts = 5;
  graph_uint64_base_init(g, num_vts);
  g->num_es = 4;
  g->u = malloc(g->num_es * sizeof(uint64_t));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(uint64_t));
  assert(g->v != NULL);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Run bfs tests on small graphs.
*/
static void vfive_graph_test_helper(graph_uint64_t *g,
				    uint64_t ret_dist[][5],
				    uint64_t ret_prev[][5],
				    void (*build_fn)(adj_lst_uint64_t *,
						     graph_uint64_t *),
				    int *result);

void run_first_vfive_graph_test(){
  graph_uint64_t g;
  int result = 1;
  uint64_t dir_dist[5][5] = {{0, 1, 1, 1, 0},
			     {0, 0, 0, 1, 0},
			     {0, 0, 0, 0, 0},
			     {0, 0, 0, 0, 0},
			     {0, 0, 0, 0, 0}};
  uint64_t dir_prev[5][5] = {{0, 0, 0, 0, nr},
			     {nr, 1, nr, 1, nr},
			     {nr, nr, 2, nr, nr},
			     {nr, nr, nr, 3, nr},
			     {nr, nr, nr, nr, 4}};
  uint64_t undir_dist[5][5] = {{0, 1, 1, 1, 0},
			       {1, 0, 2, 1, 0},
			       {1, 2, 0, 2, 0},
			       {1, 1, 2, 0, 0},
			       {0, 0, 0, 0, 0}};
  uint64_t undir_prev[5][5] = {{0, 0, 0, 0, nr},
			       {1, 1, 0, 1, nr},
			       {2, 0, 2, 0, nr},
			       {3, 3, 0, 3, nr},
			       {nr, nr, nr, nr, 4}};
  printf("Run a bfs_uint64 test on the first small graph instance --> ");
  first_vfive_graph_init(&g);
  vfive_graph_test_helper(&g,
			  dir_dist,
			  dir_prev,
			  adj_lst_uint64_dir_build,
			  &result);
  vfive_graph_test_helper(&g,
			  undir_dist,
			  undir_prev,
			  adj_lst_uint64_undir_build,
			  &result);
  graph_uint64_free(&g);
  print_test_result(result);
}

void run_second_vfive_graph_test(){
  graph_uint64_t g;
  int result = 1;
  uint64_t dir_dist[5][5] = {{0, 1, 2, 3, 4},
			     {0, 0, 1, 2, 3},
			     {0, 0, 0, 1, 2},
			     {0, 0, 0, 0, 1},
			     {0, 0, 0, 0, 0}};
  uint64_t dir_prev[5][5] = {{0, 0, 1, 2, 3},
			     {nr, 1, 1, 2, 3},
			     {nr, nr, 2, 2, 3},
			     {nr, nr, nr, 3, 3},
			     {nr, nr, nr, nr, 4}};
  uint64_t undir_dist[5][5] = {{0, 1, 2, 3, 4},
			       {1, 0, 1, 2, 3},
			       {2, 1, 0, 1, 2},
			       {3, 2, 1, 0, 1},
			       {4, 3, 2, 1, 0}};
  uint64_t undir_prev[5][5] = {{0, 0, 1, 2, 3},
			       {1, 1, 1, 2, 3},
			       {1, 2, 2, 2, 3},
			       {1, 2, 3, 3, 3},
			       {1, 2, 3, 4, 4}};
  printf("Run a bfs_uint64 test on the second small graph instance --> ");
  second_vfive_graph_init(&g);
  vfive_graph_test_helper(&g,
			  dir_dist,
			  dir_prev,
			  adj_lst_uint64_dir_build,
			  &result);
  vfive_graph_test_helper(&g,
			  undir_dist,
			  undir_prev,
			  adj_lst_uint64_undir_build,
			  &result);
  graph_uint64_free(&g);
  print_test_result(result);
}

static void vfive_graph_test_helper(graph_uint64_t *g,
				    uint64_t ret_dist[][5],
				    uint64_t ret_prev[][5],
				    void (*build_fn)(adj_lst_uint64_t *,
						     graph_uint64_t *),
				    int *result){
  adj_lst_uint64_t a;
  uint64_t *dist, *prev;
  adj_lst_uint64_init(&a, g);
  build_fn(&a, g);
  dist = malloc(a.num_vts * sizeof(uint64_t));
  prev = malloc(a.num_vts * sizeof(uint64_t));
  for (uint64_t i = 0; i < a.num_vts; i++){
    bfs_uint64(&a, i, dist, prev);
    *result *= cmp_uint64_arrs(dist, ret_dist[i], a.num_vts);
    *result *= cmp_uint64_arrs(prev, ret_prev[i], a.num_vts);
  }
  adj_lst_uint64_free(&a);
  free(dist);
  free(prev);
}

/**  
   Test bfs_uint64 on directed graphs with n(n - 1) edges.
*/

/**
   Runs a bfs_uint64 test on directed graphs with n(n - 1) edges.
*/
void run_max_edges_graph_test(){
  adj_lst_uint64_t a;
  int pow_two_start = 0;
  int pow_two_end = 15;
  int result = 1;
  uint64_t n, start;
  uint64_t *dist, *prev;
  uint32_t num = 1;
  uint32_t denom = 1;
  printf("Run a bfs_uint64 test on graphs with n vertices, where "
	 "0 < n <= 2^%d, and n(n - 1) edges --> ", pow_two_end - 1);
  fflush(stdout);
  srandom(time(0));
  for (int i = pow_two_start; i <  pow_two_end; i++){
    n = pow_two_uint64(i); // 0 < n
    dist = malloc(n * sizeof(uint64_t));
    prev = malloc(n * sizeof(uint64_t));
    adj_lst_uint64_rand_dir(&a, n, num, denom); //num/denom = 1
    start = (uint64_t)random_range_uint32((uint32_t)(n - 1));
    bfs_uint64(&a, start, dist, prev);
    for (uint64_t i = 0; i < n; i++){
      if (i == start){
	result *= (dist[i] == 0);
      }else{
	result *= (dist[i] == 1);
      }
      result *= (prev[i] == start);
    }
    adj_lst_uint64_free(&a);
    free(dist);
    free(prev);
  }
  print_test_result(result);
}

/** 
   Test bfs_uint64 on graphs with no edges.
*/

/**
   Runs a bfs_uint64 test on directed graphs with no edges.
*/
void run_no_edges_graph_test(){
  adj_lst_uint64_t a;
  int pow_two_start = 0;
  int pow_two_end = 15;
  int result = 1;
  uint64_t n, start;
  uint64_t *dist, *prev;
  uint32_t num = 0;
  uint32_t denom = 1;
  printf("Run a bfs_uint64 test on graphs with n vertices, where "
	 "0 < n <= 2^%d, and no edges --> ", pow_two_end - 1);
  fflush(stdout);
  srandom(time(0));
  for (int i = pow_two_start; i <  pow_two_end; i++){
    n = pow_two_uint64(i); // 0 < n
    dist = malloc(n * sizeof(uint64_t));
    prev = malloc(n * sizeof(uint64_t));
    adj_lst_uint64_rand_dir(&a, n, num, denom); //num/denom = 0
    start = (uint64_t)random_range_uint32((uint32_t)(n - 1));
    bfs_uint64(&a, start, dist, prev);
    for (uint64_t i = 0; i < n; i++){
      if (i == start){
	result *= (prev[i] == start);
      }else{
	result *= (prev[i] == nr);
      }
      result *= (dist[i] == 0);
    }
    adj_lst_uint64_free(&a);
    free(dist);
    free(prev);
  }
  print_test_result(result);
}

/** 
   Test bfs_uint64 on random directed graphs.
*/

/**
   Runs a bfs_uint64 test on random directed graphs.
*/
void run_random_dir_graph_test(){
  adj_lst_uint64_t a;
  int pow_two_start = 10;
  int pow_two_end = 15;
  int num_nums = 5;
  int ave_iter = 10;
  uint64_t n, start;
  uint64_t *dist, *prev;
  uint32_t nums[] = {4, 3, 2, 1, 0};
  uint32_t denom = 4;
  clock_t t;
  printf("Run a bfs_uint64 test on random directed graphs, from %d random "
	 "start vertices in each graph \n", ave_iter);
  fflush(stdout);
  srandom(time(0));
  for (int num_ix = 0; num_ix < num_nums; num_ix++){
    printf("\tP[an edge is in a graph] = %.4f\n",
	   (float)nums[num_ix] / denom);
    for (int i = pow_two_start; i <  pow_two_end; i++){
      n = pow_two_uint64(i); // 0 < n
      dist = malloc(n * sizeof(uint64_t));
      prev = malloc(n * sizeof(uint64_t));
      adj_lst_uint64_rand_dir(&a, n, nums[num_ix], denom);
      t = clock();
      for (int i = 0; i < ave_iter; i++){
	start = (uint64_t)random_range_uint32((uint32_t)(n - 1));
	bfs_uint64(&a, start, dist, prev);
      }
      t = clock() - t;
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f, "
	     "average runtime: %.6f seconds\n",
	     n, (float)nums[num_ix] / denom * (n * (n - 1)),
	     (float)t / (ave_iter * CLOCKS_PER_SEC));
      fflush(stdout);
      adj_lst_uint64_free(&a);
      free(dist);
      free(prev);
    }
  }
}

static int cmp_uint64_arrs(uint64_t a[], uint64_t b[], uint64_t n){
  int result = 1;
  for (uint64_t i = 0; i < n; i++){
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
