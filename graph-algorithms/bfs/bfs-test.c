/**
   bfs-test.c

   Tests of the BFS algorithm.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bfs.h"
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

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* small graph test */
const size_t C_NUM_VTS_FIRST = 5;
const size_t C_NUM_ES_FIRST = 4;
const size_t C_U_FIRST[4] = {0, 0, 0, 1};
const size_t C_V_FIRST[4] = {1, 2, 3, 3};
const size_t C_DIR_DIST_FIRST[5][5] =
  {{0, 1, 1, 1, 0},
   {0, 0, 0, 1, 0},
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0}};
const size_t C_DIR_PREV_FIRST[5][5] =
  {{0, 0, 0, 0, (size_t)-1},
   {(size_t)-1, 1, (size_t)-1, 1, (size_t)-1},
   {(size_t)-1, (size_t)-1, 2, (size_t)-1, (size_t)-1},
   {(size_t)-1, (size_t)-1, (size_t)-1, 3, (size_t)-1},
   {(size_t)-1, (size_t)-1, (size_t)-1, (size_t)-1, 4}};
const size_t C_UNDIR_DIST_FIRST[5][5] =
  {{0, 1, 1, 1, 0},
   {1, 0, 2, 1, 0},
   {1, 2, 0, 2, 0},
   {1, 1, 2, 0, 0},
   {0, 0, 0, 0, 0}};
const size_t C_UNDIR_PREV_FIRST[5][5] =
  {{0, 0, 0, 0, (size_t)-1},
   {1, 1, 0, 1, (size_t)-1},
   {2, 0, 2, 0, (size_t)-1},
   {3, 3, 0, 3, (size_t)-1},
   {(size_t)-1, (size_t)-1, (size_t)-1, (size_t)-1, 4}};

const size_t C_NUM_VTS_SECOND = 5;
const size_t C_NUM_ES_SECOND = 4;
const size_t C_U_SECOND[5] = {0, 1, 2, 3};
const size_t C_V_SECOND[5] = {1, 2, 3, 4};
const size_t C_DIR_DIST_SECOND[5][5] =
  {{0, 1, 2, 3, 4},
   {0, 0, 1, 2, 3},
   {0, 0, 0, 1, 2},
   {0, 0, 0, 0, 1},
   {0, 0, 0, 0, 0}};
const size_t C_DIR_PREV_SECOND[5][5] =
  {{0, 0, 1, 2, 3},
   {(size_t)-1, 1, 1, 2, 3},
   {(size_t)-1, (size_t)-1, 2, 2, 3},
   {(size_t)-1, (size_t)-1, (size_t)-1, 3, 3},
   {(size_t)-1, (size_t)-1, (size_t)-1, (size_t)-1, 4}};
const size_t C_UNDIR_DIST_SECOND[5][5] =
  {{0, 1, 2, 3, 4},
   {1, 0, 1, 2, 3},
   {2, 1, 0, 1, 2},
   {3, 2, 1, 0, 1},
   {4, 3, 2, 1, 0}};
const size_t C_UNDIR_PREV_SECOND[5][5] =
  {{0, 0, 1, 2, 3},
   {1, 1, 1, 2, 3},
   {1, 2, 2, 2, 3},
   {1, 2, 3, 3, 3},
   {1, 2, 3, 4, 4}};

/* large graph tests */
const int C_ITER = 10;
const int C_PROBS_COUNT = 5;
const double C_PROBS[5] = {1.00, 0.75, 0.50, 0.25, 0.00};
const double C_PROB_ONE = 1.0;
const double C_PROB_ZERO = 0.0;
const size_t C_SIZE_MAX = (size_t)-1;

int cmp_arr(const size_t *a, const size_t *b, size_t n);
size_t pow_two(int k);
void print_test_result(int res);

/**  
   Test bfs on small graphs.
*/

/**
   Initializes the first graph with five vertices.
*/
void first_vfive_graph_init(graph_t *g){
  size_t i;
  graph_base_init(g, C_NUM_VTS_FIRST, 0);
  g->num_es = C_NUM_ES_FIRST;
  g->u = malloc_perror(g->num_es * sizeof(size_t));
  g->v = malloc_perror(g->num_es * sizeof(size_t));
  for (i = 0; i < g->num_es; i++){
    g->u[i] = C_U_FIRST[i];
    g->v[i] = C_V_FIRST[i];
  }
}

/**
   Initializes the second graph with five vertices.
*/
void second_vfive_graph_init(graph_t *g){
  size_t i;
  graph_base_init(g, C_NUM_VTS_SECOND, 0);
  g->num_es = C_NUM_ES_SECOND;
  g->u = malloc_perror(g->num_es * sizeof(size_t));
  g->v = malloc_perror(g->num_es * sizeof(size_t));
  for (i = 0; i < g->num_es; i++){
    g->u[i] = C_U_SECOND[i];
    g->v[i] = C_V_SECOND[i];
  }
}

/**
   Run bfs tests on small graphs.
*/
void vfive_graph_helper(const graph_t *g,
			const size_t ret_dist[][5],
			const size_t ret_prev[][5],
			void (*build)(adj_lst_t *, const graph_t *),
			int *res);

void run_first_vfive_graph_test(){
  int res = 1;
  graph_t g;
  printf("Run a bfs test on the first small graph instance --> ");
  first_vfive_graph_init(&g);
  vfive_graph_helper(&g, C_DIR_DIST_FIRST, C_DIR_PREV_FIRST, adj_lst_dir_build, &res);
  vfive_graph_helper(&g, C_UNDIR_DIST_FIRST, C_UNDIR_PREV_FIRST, adj_lst_undir_build, &res);
  graph_free(&g);
  print_test_result(res);
}

void run_second_vfive_graph_test(){
  int res = 1;
  graph_t g;
  printf("Run a bfs test on the second small graph instance --> ");
  second_vfive_graph_init(&g);
  vfive_graph_helper(&g, C_DIR_DIST_SECOND, C_DIR_PREV_SECOND, adj_lst_dir_build, &res);
  vfive_graph_helper(&g, C_UNDIR_DIST_SECOND, C_UNDIR_PREV_SECOND, adj_lst_undir_build, &res);
  graph_free(&g);
  print_test_result(res);
}

void vfive_graph_helper(const graph_t *g,
			const size_t ret_dist[][5],
			const size_t ret_prev[][5],
			void (*build)(adj_lst_t *, const graph_t *),
			int *res){
  size_t *dist = NULL, *prev = NULL;
  size_t i;
  adj_lst_t a;
  adj_lst_init(&a, g);
  build(&a, g);
  dist = malloc_perror(a.num_vts * sizeof(size_t));
  prev = malloc_perror(a.num_vts * sizeof(size_t));
  for (i = 0; i < a.num_vts; i++){
    bfs(&a, i, dist, prev);
    *res *= cmp_arr(dist, ret_dist[i], a.num_vts);
    *res *= cmp_arr(prev, ret_prev[i], a.num_vts);
  }
  adj_lst_free(&a);
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

/**  
   Test bfs on directed graphs with n(n - 1) edges.
*/

typedef struct{
  double p;
} bern_arg_t;

int bern(void *arg){
  bern_arg_t *b = arg;
  if (b->p >= C_PROB_ONE) return 1;
  if (b->p <= C_PROB_ZERO) return 0;
  if (b->p > DRAND()) return 1;
  return 0;
}

/**
   Runs a bfs test on directed graphs with n(n - 1) edges.
*/
void run_max_edges_graph_test(int pow_start, int pow_end){
  int res = 1;
  int i;
  size_t j, n, start;
  size_t *dist = NULL, *prev = NULL;
  bern_arg_t b;
  adj_lst_t a;
  dist = malloc_perror(pow_two(pow_end) * sizeof(size_t));
  prev = malloc_perror(pow_two(pow_end) * sizeof(size_t));
  b.p = C_PROB_ONE;
  printf("Run a bfs test on graphs with n vertices, where "
	 "2^%d <= n <= 2^%d, and n(n - 1) edges --> ", pow_start, pow_end);
  fflush(stdout);
  for (i = pow_start; i <= pow_end; i++){
    n = pow_two(i); /* 0 < n */
    adj_lst_rand_dir(&a, n, bern, &b);
    start =  RANDOM() % n;
    bfs(&a, start, dist, prev);
    for (j = 0; j < n; j++){
      if (j == start){
	res *= (dist[j] == 0);
      }else{
	res *= (dist[j] == 1);
      }
      res *= (prev[j] == start);
    }
    adj_lst_free(&a);
  }
  print_test_result(res);
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

/** 
   Test bfs on graphs with no edges.
*/

/**
   Runs a bfs test on directed graphs with no edges.
*/
void run_no_edges_graph_test(int pow_start, int pow_end){
  int res = 1;
  int i;
  size_t j, n, start;
  size_t *dist = NULL, *prev = NULL;
  bern_arg_t b;
  adj_lst_t a;
  dist = malloc_perror(pow_two(pow_end) * sizeof(size_t));
  prev = malloc_perror(pow_two(pow_end) * sizeof(size_t));
  b.p = C_PROB_ZERO;
  printf("Run a bfs test on graphs with n vertices, where "
	 "2^%d <= n <= 2^%d, and no edges --> ", pow_start, pow_end);
  fflush(stdout);
  for (i = pow_start; i <= pow_end; i++){
    n = pow_two(i); /* 0 < n */
    adj_lst_rand_dir(&a, n, bern, &b);
    start =  RANDOM() % n;
    bfs(&a, start, dist, prev);
    for (j = 0; j < n; j++){
      if (j == start){
	res *= (prev[j] == start);
      }else{
	res *= (prev[j] == C_SIZE_MAX);
      }
      res *= (dist[j] == 0);
    }
    adj_lst_free(&a);
  }
  print_test_result(res);
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

/** 
   Test bfs on random directed graphs.
*/

/**
   Runs a bfs test on random directed graphs.
*/
void run_random_dir_graph_test(int pow_start, int pow_end){
  int i, j, k;
  size_t n;
  size_t *start = NULL;
  size_t *dist = NULL, *prev = NULL;
  bern_arg_t b;
  adj_lst_t a;
  clock_t t;
  start = malloc_perror(C_ITER * sizeof(size_t));
  dist = malloc_perror(pow_two(pow_end) * sizeof(size_t));
  prev = malloc_perror(pow_two(pow_end) * sizeof(size_t));
  printf("Run a bfs test on random directed graphs, from %d random "
	 "start vertices in each graph \n", C_ITER);
  fflush(stdout);
  for (i = 0; i < C_PROBS_COUNT; i++){
    b.p = C_PROBS[i];
    printf("\tP[an edge is in a graph] = %.2f\n", b.p);
    for (j = pow_start; j <= pow_end; j++){
      n = pow_two(j); /* 0 < n */
      adj_lst_rand_dir(&a, n, bern, &b);
      for (k = 0; k < C_ITER; k++){
	start[k] =  RANDOM() % n;
      }
      t = clock();
      for (k = 0; k < C_ITER; k++){
	bfs(&a, start[k], dist, prev);
      }
      t = clock() - t;
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f, "
	     "average runtime: %.6f seconds\n",
	     TOLU(n), b.p * n * (n - 1), (float)t / C_ITER / CLOCKS_PER_SEC);
      fflush(stdout);
      adj_lst_free(&a);
    }
  }
  free(start);
  free(dist);
  free(prev);
  start = NULL;
  dist = NULL;
  prev = NULL;
}

/**
   Compares elements of two size_t arrays.
*/
int cmp_arr(const size_t *a, const size_t *b, size_t n){
  int res = 1;
  size_t i;
  for (i = 0; i < n; i++){
    res *= (a[i] == b[i]);
  }
  return res;
}

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
size_t pow_two(int k){
  size_t ret = 1;
  return ret << k;
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  RGENS_SEED();
  run_first_vfive_graph_test();
  run_second_vfive_graph_test();
  run_max_edges_graph_test(0, 14);
  run_no_edges_graph_test(0, 14);
  run_random_dir_graph_test(0, 14);
  return 0;
}
