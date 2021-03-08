/**
   dfs-main.c

   Tests of the DFS algorithm.
 
   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "dfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

/**
   Generate random numbers in a portable way for test purposes only; rand()
   in the Linux C Library uses the same generator as random(), which may not
   be the case on older rand() implementations, and on current
   implementations on different systems.
*/
#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand()) /* [0, RAND_MAX] */
#define DRAND() ((double)rand() / RAND_MAX) /* [0.0, 1.0] */

int cmp_arr(const uint64_t *a, const uint64_t *b, uint64_t n);
uint64_t pow_two(int k);
void print_test_result(int res);

/**  
   Test dfs on small graphs.
*/

/**
   Initializes the first graph with six vertices.
*/
void first_vsix_graph_init(graph_t *g){
  uint64_t u[] = {0, 1, 2, 3, 0, 4, 4};
  uint64_t v[] = {1, 2, 3, 1, 3, 2, 5};
  uint64_t num_vts = 6;
  uint64_t i;
  graph_base_init(g, num_vts, 0);
  g->num_es = 7;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  for (i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Initializes the second graph with six vertices.
*/
void second_vsix_graph_init(graph_t *g){
  uint64_t u[] = {0, 1, 2, 3, 4};
  uint64_t v[] = {1, 2, 3, 4, 5};
  uint64_t num_vts = 6;
  uint64_t i;
  graph_base_init(g, num_vts, 0);
  g->num_es = 5;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  for (i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Run dfs tests on small graphs.
*/

void small_graph_helper(const graph_t *g,
			uint64_t start,
			uint64_t ret_pre[],
			uint64_t ret_post[],
			void (*build)(adj_lst_t *, const graph_t *),
			int *res);

void run_first_vsix_graph_test(){
  int res = 1;
  uint64_t start = 0;
  uint64_t dir_pre[6] = {0, 1, 2, 3, 8, 9};
  uint64_t dir_post[6] = {7, 6, 5, 4, 11, 10};
  uint64_t undir_pre[6] = {0, 1, 2, 3, 5, 6};
  uint64_t undir_post[6] = {11, 10, 9, 4, 8, 7};
  graph_t g;
  printf("Run a dfs test on the first small graph instance --> ");
  first_vsix_graph_init(&g);
  small_graph_helper(&g, start, dir_pre, dir_post, adj_lst_dir_build, &res);
  small_graph_helper(&g,
		     start,
		     undir_pre,
		     undir_post,
		     adj_lst_undir_build,
		     &res);
  graph_free(&g);
  print_test_result(res);
}

void run_second_vsix_graph_test(){
  int res = 1;
  uint64_t start = 0;
  uint64_t dir_pre[6] = {0, 1, 2, 3, 4, 5};
  uint64_t dir_post[6] = {11, 10, 9, 8, 7, 6};
  uint64_t undir_pre[6] = {0, 1, 2, 3, 4, 5};
  uint64_t undir_post[6] = {11, 10, 9, 8, 7, 6};
  graph_t g;
  printf("Run a dfs test on the second small graph instance --> ");
  second_vsix_graph_init(&g);
  small_graph_helper(&g, start, dir_pre, dir_post, adj_lst_dir_build, &res);
  small_graph_helper(&g,
		     start,
		     undir_pre,
		     undir_post,
		     adj_lst_undir_build,
		     &res);
  graph_free(&g);
  print_test_result(res);
}

void small_graph_helper(const graph_t *g,
			uint64_t start,
			uint64_t ret_pre[],
			uint64_t ret_post[],
			void (*build)(adj_lst_t *, const graph_t *),
			int *res){
  uint64_t i;
  uint64_t *pre = NULL, *post = NULL;
  adj_lst_t a;
  adj_lst_init(&a, g);
  build(&a, g);
  pre = malloc_perror(a.num_vts * sizeof(uint64_t));
  post = malloc_perror(a.num_vts * sizeof(uint64_t));
  for (i = 0; i < a.num_vts; i++){
    dfs(&a, start, pre, post);
    *res *= cmp_arr(pre, ret_pre, a.num_vts);
    *res *= cmp_arr(post, ret_post, a.num_vts);
  }
  adj_lst_free(&a);
  free(pre);
  free(post);
  pre = NULL;
  post = NULL;
}

/**  
   Test dfs on directed graphs with n(n - 1) edges.
*/

typedef struct{
  double p;
} bern_arg_t;

int bern_fn(void *arg){
  bern_arg_t *b = arg;
  if (b->p >= 1.00) return 1;
  if (b->p <= 0.00) return 0;
  if (b->p > DRAND()) return 1;
  return 0;
}

/**
   Runs a dfs test on directed graphs with n(n - 1) edges. The test relies
   on the construction order in adj_lst_rand_dir.
*/
void run_max_edges_graph_test(){
  int res = 1;
  int i, pow_end = 15;
  uint64_t j, n, start;
  uint64_t *pre = NULL, *post = NULL;
  bern_arg_t b;
  adj_lst_t a;
  b.p = 1.00;
  printf("Run a dfs test on graphs with n vertices, where "
	 "0 < n <= 2^%d, and n(n - 1) edges --> ", pow_end - 1);
  fflush(stdout);
  for (i = 0; i < pow_end; i++){
    n = pow_two(i); /* n > 0 */
    pre = malloc_perror(n * sizeof(uint64_t));
    post = malloc_perror(n * sizeof(uint64_t));
    adj_lst_rand_dir(&a, n, bern_fn, &b);
    start =  RANDOM() % n;
    dfs(&a, start, pre, post);
    for (j = 0; j < n; j++){
      if (j == start){
	res *= (pre[j] == 0);
	res *= (post[j] == 2 * n - 1);
      }else if (j < start){
	/* n >= 2 */
	res *= (pre[j] == j + 1);
	res *= (post[j] == 2 * n - 2 - j);
      }else{
	res *= (pre[j] == j);
	res *= (post[j] == 2 * n - 1 - j);
      }
    }
    adj_lst_free(&a);
    free(pre);
    free(post);
    pre = NULL;
    post = NULL;
  }
  print_test_result(res);
}

/**
   Runs a dfs test on graphs with no edges.
*/
void run_no_edges_graph_test(){
  int res = 1;
  int i, pow_end = 15;
  uint64_t j, n, start;
  uint64_t *pre = NULL, *post = NULL;
  bern_arg_t b;
  adj_lst_t a;
  b.p = 0.00;
  printf("Run a dfs test on graphs with n vertices, where "
	 "0 < n <= 2^%d, and no edges --> ", pow_end - 1);
  fflush(stdout);
  for (i = 0; i < pow_end; i++){
    n = pow_two(i);
    pre = malloc_perror(n * sizeof(uint64_t));
    post = malloc_perror(n * sizeof(uint64_t));
    adj_lst_rand_dir(&a, n, bern_fn, &b);
    start =  RANDOM() % n;
    dfs(&a, start, pre, post);
    for (j = 0; j < n; j++){
      res *= (post[j] - pre[j] == 1);
    }
    adj_lst_free(&a);
    free(pre);
    free(post);
    pre = NULL;
    post = NULL;
  }
  print_test_result(res);
}

/**
   Test dfs on random directed graphs.
*/

/**
   Runs a dfs test on random directed graphs.
*/
void run_random_dir_graph_test(){
  int i, num_p = 5;
  int j, k, pow_end = 15, ave_iter = 10;
  uint64_t n, start[10];
  uint64_t *pre = NULL, *post = NULL;
  double p[5] = {1.00, 0.75, 0.50, 0.25, 0.00};
  bern_arg_t b;
  adj_lst_t a;
  clock_t t;
  printf("Run a dfs test on random directed graphs from %d random "
	 "start vertices in each graph \n", ave_iter);
  fflush(stdout);
  for (i = 0; i < num_p; i++){
    b.p = p[i];
    printf("\tP[an edge is in a graph] = %.2f\n", b.p);
    for (j = 0; j <  pow_end; j++){
      n = pow_two(j);
      pre = malloc_perror(n * sizeof(uint64_t));
      post = malloc_perror(n * sizeof(uint64_t));
      adj_lst_rand_dir(&a, n, bern_fn, &b);
      for (k = 0; k < ave_iter; k++){
	start[k] =  RANDOM() % n;
      }
      t = clock();
      for (k = 0; k < ave_iter; k++){
	dfs(&a, start[k], pre, post);
      }
      t = clock() - t;
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f, "
	     "average runtime: %.6f seconds\n",
	     n, b.p * n * (n - 1), (float)t / ave_iter / CLOCKS_PER_SEC);
      fflush(stdout);
      adj_lst_free(&a);
      free(pre);
      free(post);
      pre = NULL;
      post = NULL;
    }
  }
}

/**
   Compares the elements of two uint64_t arrays.
*/
int cmp_arr(const uint64_t *a, const uint64_t *b, uint64_t n){
  int res = 1;
  uint64_t i;
  for (i = 0; i < n; i++){
    res *= (a[i] == b[i]);
  }
  return res;
}

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
uint64_t pow_two(int k){
  uint64_t ret = 1;
  return ret << k;
}

/**
   Prints a test result.
*/
void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  RGENS_SEED();
  run_first_vsix_graph_test();
  run_second_vsix_graph_test();
  run_max_edges_graph_test();
  run_no_edges_graph_test();
  run_random_dir_graph_test();
  return 0;
}
