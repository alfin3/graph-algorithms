/**
   dfs-main.c

   Examples of running the DFS algorithm. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "dfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-ds.h"

static int cmp_int_arrs(int a[], int b[], int n);
void print_test_result(int result);

/**  
   Test dfs on small graphs.
*/

/**
   Initializes the first graph with six vertices.
*/
void first_vsix_graph_init(graph_t *g){
  int u[] = {0, 1, 2, 3, 0, 4, 4};
  int v[] = {1, 2, 3, 1, 3, 2, 5};
  int num_vts = 6;
  graph_base_init(g, num_vts);
  g->num_es = 7;
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
   Initializes the second graph with six vertices.
*/
void second_vsix_graph_init(graph_t *g){
  int u[] = {0, 1, 2, 3, 4};
  int v[] = {1, 2, 3, 4, 5};
  int num_vts = 6;
  graph_base_init(g, num_vts);
  g->num_es = 5;
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
   Run dfs tests on small graphs.
*/
static void graph_test_helper(graph_t *g,
			      int pre[],
			      int post[],
			      void (*build_fn)(graph_t *, adj_lst_t *),
			      int *result);

void run_first_vsix_graph_test(){
  graph_t g;
  int result = 1;
  int dir_pre[6] = {0, 1, 2, 3, 8, 9};
  int dir_post[6] = {7, 6, 5, 4, 11, 10};
  int undir_pre[6] = {0, 1, 2, 3, 5, 6};
  int undir_post[6] = {11, 10, 9, 4, 8, 7};
  printf("Run a dfs test on the first small graph instance --> ");
  first_vsix_graph_init(&g);
  graph_test_helper(&g,
		    dir_pre,
		    dir_post,
		    adj_lst_dir_build,
		    &result);
  graph_test_helper(&g,
		    undir_pre,
		    undir_post,
		    adj_lst_undir_build,
		    &result);
  graph_free(&g);
  print_test_result(result);
}

void run_second_vsix_graph_test(){
  graph_t g;
  int result = 1;
  int dir_pre[6] = {0, 1, 2, 3, 4, 5};
  int dir_post[6] = {11, 10, 9, 8, 7, 6};
  int undir_pre[6] = {0, 1, 2, 3, 4, 5};
  int undir_post[6] = {11, 10, 9, 8, 7, 6};
  printf("Run a dfs test on the second small graph instance --> ");
  second_vsix_graph_init(&g);
  graph_test_helper(&g,
		    dir_pre,
		    dir_post,
		    adj_lst_dir_build,
		    &result);
  graph_test_helper(&g,
		    undir_pre,
		    undir_post,
		    adj_lst_undir_build,
		    &result);
  graph_free(&g);
  print_test_result(result);
}

static void graph_test_helper(graph_t *g,
			      int ret_pre[],
			      int ret_post[],
			      void (*build_fn)(graph_t *, adj_lst_t *),
			      int *result){
  adj_lst_t a;
  int *pre, *post;
  adj_lst_init(g, &a);
  build_fn(g, &a);
  pre = malloc(a.num_vts * sizeof(int));
  post = malloc(a.num_vts * sizeof(int));
  for (int i = 0; i < a.num_vts; i++){
    dfs(&a, pre, post);
    *result *= cmp_int_arrs(pre, ret_pre, a.num_vts);
    *result *= cmp_int_arrs(post, ret_post, a.num_vts);
  }
  adj_lst_free(&a);
  free(pre);
  free(post);
}

/**  
   Test dfs on directed graphs with n(n - 1) edges.
*/

/**
   Runs a dfs test on directed graphs with n(n - 1) edges. The test relies
   on the construction order in adj_lst_rand_dir.
*/
void run_max_edges_graph_test(){
  adj_lst_t a;
  int n;
  int pow_two_start = 0;
  int pow_two_end = 14;
  int result = 1;
  int *pre, *post;
  uint32_t nom = 1;
  uint32_t denom = 1;
  printf("Run a dfs test on graphs with n vertices, where 0 < n <= 2^%d, "
	 "and n(n - 1) edges --> ", pow_two_end);
  fflush(stdout);
  for (int i = pow_two_start; i <  pow_two_end; i++){
    n = (int)pow_two_uint64(i); //0 < n
    pre = malloc(n * sizeof(int));
    post = malloc(n * sizeof(int));
    adj_lst_rand_dir(&a, n, nom, denom); //nom/denom = 1
    dfs(&a, pre, post);
    for (int i = 0; i < n; i++){
      result *= (pre[i] == i);
      result *= (post[i] == 2 * n - 1 - i);
    }
    adj_lst_free(&a);
    free(pre);
    free(post);
  }
  print_test_result(result);
}

/**
   Runs a dfs test on graphs with no edges.
*/
void run_no_edges_graph_test(){
  adj_lst_t a;
  int n;
  int pow_two_start = 0;
  int pow_two_end = 14;
  int result = 1;
  int *pre, *post;
  uint32_t nom = 0;
  uint32_t denom = 1;
  printf("Run a dfs test on graphs with n vertices, where 0 <= n <= 2^%d, "
	 "and no edges --> ", pow_two_end);
  fflush(stdout);
  //no vertices
  n = 0;
  pre = NULL;
  post = NULL;
  adj_lst_rand_dir(&a, n, nom, denom); //nom/denom = 0
  dfs(&a, pre, post);
  result *= (pre == NULL && post == NULL);
  //one or more vertices
  for (int i = pow_two_start; i <  pow_two_end; i++){
    n = (int)pow_two_uint64(i); //0 < n
    pre = malloc(n * sizeof(int));
    post = malloc(n * sizeof(int));
    adj_lst_rand_dir(&a, n, nom, denom); //nom/denom = 0
    dfs(&a, pre, post);
    for (int i = 0; i < n; i++){
      result *= (post[i] - pre[i] == 1);
    }
    adj_lst_free(&a);
    free(pre);
    free(post);
  }
  print_test_result(result);
}

/** 
   Test dfs on random directed graphs.
*/

/**
   Runs a dfs test on random directed graphs.
*/
void run_random_dir_graph_test(){
  adj_lst_t a;
  int n;
  int pow_two_start = 10;
  int pow_two_end = 15;
  int num_numers = 5;
  int *pre, *post;
  uint32_t numers[] = {4, 3, 2, 1, 0};
  uint32_t denom = 4;
  clock_t t;
  printf("Run a dfs test on random directed graphs \n");
  fflush(stdout);
  srandom(time(0));
  for (int numer_ix = 0; numer_ix < num_numers; numer_ix++){
    printf("\tP[an edge is in a graph] = %.4f\n",
	   (float)numers[numer_ix] / (float)denom);
    for (int exp = pow_two_start; exp <  pow_two_end; exp++){
      n = (int)pow_two_uint64(exp); // 0 < n
      pre = malloc(n * sizeof(int));
      post = malloc(n * sizeof(int));
      adj_lst_rand_dir(&a, n, numers[numer_ix], denom);
      t = clock();
      dfs(&a, pre, post);
      t = clock() - t;
      printf("\t\tvertices: %d, E[# of directed edges]: %.1f, "
	     "runtime: %.6f seconds\n",
	     n, (float)numers[numer_ix] / (float)denom * (float)(n * (n - 1)),
	     (float)t / CLOCKS_PER_SEC);
      fflush(stdout);
      adj_lst_free(&a);
      free(pre);
      free(post);
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
  run_first_vsix_graph_test();
  run_second_vsix_graph_test();
  run_max_edges_graph_test();
  run_no_edges_graph_test();
  run_random_dir_graph_test();
}
