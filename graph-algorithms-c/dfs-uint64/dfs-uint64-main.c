/**
   dfs-uint64-main.c

   Examples of running the DFS algorithm. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "dfs-uint64.h"
#include "graph-uint64.h"
#include "stack-uint64.h"
#include "utilities-ds.h"

static int cmp_uint64_arrs(uint64_t a[], uint64_t b[], uint64_t n);
void print_test_result(int result);

/**  
   Test dfs_uint64 on small graphs.
*/

/**
   Initializes the first graph with six vertices.
*/
void first_vsix_graph_init(graph_uint64_t *g){
  uint64_t u[] = {0, 1, 2, 3, 0, 4, 4};
  uint64_t v[] = {1, 2, 3, 1, 3, 2, 5};
  uint64_t num_vts = 6;
  graph_uint64_base_init(g, num_vts, 0);
  g->num_es = 7;
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
   Initializes the second graph with six vertices.
*/
void second_vsix_graph_init(graph_uint64_t *g){
  uint64_t u[] = {0, 1, 2, 3, 4};
  uint64_t v[] = {1, 2, 3, 4, 5};
  uint64_t num_vts = 6;
  graph_uint64_base_init(g, num_vts, 0);
  g->num_es = 5;
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
   Run dfs_uint64 tests on small graphs.
*/
static void graph_test_helper(graph_uint64_t *g,
			      uint64_t pre[],
			      uint64_t post[],
			      void (*build_fn)(adj_lst_uint64_t *,
					       graph_uint64_t *),
			      int *result);

void run_first_vsix_graph_test(){
  graph_uint64_t g;
  int result = 1;
  uint64_t dir_pre[6] = {0, 1, 2, 3, 8, 9};
  uint64_t dir_post[6] = {7, 6, 5, 4, 11, 10};
  uint64_t undir_pre[6] = {0, 1, 2, 3, 5, 6};
  uint64_t undir_post[6] = {11, 10, 9, 4, 8, 7};
  printf("Run a dfs_uint64 test on the first small graph instance --> ");
  first_vsix_graph_init(&g);
  graph_test_helper(&g,
		    dir_pre,
		    dir_post,
		    adj_lst_uint64_dir_build,
		    &result);
  graph_test_helper(&g,
		    undir_pre,
		    undir_post,
		    adj_lst_uint64_undir_build,
		    &result);
  graph_uint64_free(&g);
  print_test_result(result);
}

void run_second_vsix_graph_test(){
  graph_uint64_t g;
  int result = 1;
  uint64_t dir_pre[6] = {0, 1, 2, 3, 4, 5};
  uint64_t dir_post[6] = {11, 10, 9, 8, 7, 6};
  uint64_t undir_pre[6] = {0, 1, 2, 3, 4, 5};
  uint64_t undir_post[6] = {11, 10, 9, 8, 7, 6};
  printf("Run a dfs_uint64 test on the second small graph instance --> ");
  second_vsix_graph_init(&g);
  graph_test_helper(&g,
		    dir_pre,
		    dir_post,
		    adj_lst_uint64_dir_build,
		    &result);
  graph_test_helper(&g,
		    undir_pre,
		    undir_post,
		    adj_lst_uint64_undir_build,
		    &result);
  graph_uint64_free(&g);
  print_test_result(result);
}

static void graph_test_helper(graph_uint64_t *g,
			      uint64_t ret_pre[],
			      uint64_t ret_post[],
			      void (*build_fn)(adj_lst_uint64_t *,
					       graph_uint64_t *),
			      int *result){
  adj_lst_uint64_t a;
  uint64_t *pre = NULL, *post = NULL;
  adj_lst_uint64_init(&a, g);
  build_fn(&a, g);
  pre = malloc(a.num_vts * sizeof(uint64_t));
  assert(pre != NULL);
  post = malloc(a.num_vts * sizeof(uint64_t));
  assert(post != NULL);
  for (uint64_t i = 0; i < a.num_vts; i++){
    dfs_uint64(&a, pre, post);
    *result *= cmp_uint64_arrs(pre, ret_pre, a.num_vts);
    *result *= cmp_uint64_arrs(post, ret_post, a.num_vts);
  }
  adj_lst_uint64_free(&a);
  free(pre);
  free(post);
  pre = NULL;
  post = NULL;
}

/**  
   Test dfs_uint64 on directed graphs with n(n - 1) edges.
*/

/**
   Runs a dfs_uint64 test on directed graphs with n(n - 1) edges. The test 
   relies on the construction order in adj_lst_uint64_rand_dir.
*/
void run_max_edges_graph_test(){
  adj_lst_uint64_t a;
  int pow_two_start = 0;
  int pow_two_end = 15;
  int result = 1;
  uint64_t n;
  uint64_t *pre = NULL, *post = NULL;
  uint32_t num = 1;
  uint32_t denom = 1;
  printf("Run a dfs_uint64 test on graphs with n vertices, where "
	 "0 < n <= 2^%d, and n(n - 1) edges --> ", pow_two_end - 1);
  fflush(stdout);
  for (int i = pow_two_start; i <  pow_two_end; i++){
    n = pow_two_uint64(i); //0 < n
    pre = malloc(n * sizeof(uint64_t));
    assert(pre != NULL);
    post = malloc(n * sizeof(uint64_t));
    assert(post != NULL);
    adj_lst_uint64_rand_dir(&a, n, num, denom); //num/denom = 1
    dfs_uint64(&a, pre, post);
    for (uint64_t i = 0; i < n; i++){
      result *= (pre[i] == i);
      result *= (post[i] == 2 * n - 1 - i);
    }
    adj_lst_uint64_free(&a);
    free(pre);
    free(post);
    pre = NULL;
    post = NULL;
  }
  print_test_result(result);
}

/**
   Runs a dfs_uint64 test on graphs with no edges.
*/
void run_no_edges_graph_test(){
  adj_lst_uint64_t a;
  int pow_two_start = 0;
  int pow_two_end = 15;
  int result = 1;
  uint64_t n;
  uint64_t *pre = NULL, *post = NULL;
  uint32_t num = 0;
  uint32_t denom = 1;
  printf("Run a dfs_uint64 test on graphs with n vertices, where "
	 "0 <= n <= 2^%d, and no edges --> ", pow_two_end - 1);
  fflush(stdout);
  //no vertices
  n = 0;
  pre = NULL;
  post = NULL;
  adj_lst_uint64_rand_dir(&a, n, num, denom); //num/denom = 0
  dfs_uint64(&a, pre, post);
  result *= (pre == NULL && post == NULL);
  //one or more vertices
  for (int i = pow_two_start; i <  pow_two_end; i++){
    n = pow_two_uint64(i); //0 < n
    pre = malloc(n * sizeof(uint64_t));
    assert(pre != NULL);
    post = malloc(n * sizeof(uint64_t));
    assert(post != NULL);
    adj_lst_uint64_rand_dir(&a, n, num, denom); //num/denom = 0
    dfs_uint64(&a, pre, post);
    for (uint64_t i = 0; i < n; i++){
      result *= (post[i] - pre[i] == 1);
    }
    adj_lst_uint64_free(&a);
    free(pre);
    free(post);
    pre = NULL;
    post = NULL;
  }
  print_test_result(result);
}

/**
   Test dfs_uint64 on random directed graphs.
*/

/**
   Runs a dfs_uint64 test on random directed graphs.
*/
void run_random_dir_graph_test(){
  adj_lst_uint64_t a;
  int pow_two_start = 10;
  int pow_two_end = 15;
  int num_numers = 5;
  uint64_t n;
  uint64_t *pre = NULL, *post = NULL;
  uint32_t numers[] = {4, 3, 2, 1, 0};
  uint32_t denom = 4;
  clock_t t;
  printf("Run a dfs_uint64 test on random directed graphs \n");
  fflush(stdout);
  srandom(time(0));
  for (int numer_ix = 0; numer_ix < num_numers; numer_ix++){
    printf("\tP[an edge is in a graph] = %.4f\n",
	   (float)numers[numer_ix] / denom);
    for (int exp = pow_two_start; exp <  pow_two_end; exp++){
      n = pow_two_uint64(exp); // 0 < n
      pre = malloc(n * sizeof(uint64_t));
      assert(pre != NULL);
      post = malloc(n * sizeof(uint64_t));
      assert(post != NULL);
      adj_lst_uint64_rand_dir(&a, n, numers[numer_ix], denom);
      t = clock();
      dfs_uint64(&a, pre, post);
      t = clock() - t;
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f, "
	     "runtime: %.6f seconds\n",
	     a.num_vts,
	     (float)numers[numer_ix] / denom * a.num_vts * (a.num_vts - 1),
	     (float)t / CLOCKS_PER_SEC);
      fflush(stdout);
      adj_lst_uint64_free(&a);
      free(pre);
      free(post);
      pre = NULL;
      post = NULL;
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
  run_first_vsix_graph_test();
  run_second_vsix_graph_test();
  run_max_edges_graph_test();
  run_no_edges_graph_test();
  run_random_dir_graph_test();
}
