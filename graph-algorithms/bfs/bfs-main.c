/**
   bfs-main.c

   Tests of the BFS algorithm.
   
   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "bfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"


#define SEED(s) do{srandom(s); srand48(s);} while (0)
#define RANDOM() (random())
#define DRAND48() (drand48())

int cmp_arr(const uint64_t *a, const uint64_t *b, uint64_t n);
uint64_t pow_two(int k);
void print_test_result(int res);

const uint64_t NR = SIZE_MAX; //not reached as index

/**  
   Test bfs on small graphs.
*/

/**
   Initializes the first graph with five vertices.
*/
void first_vfive_graph_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t num_vts = 5;
  graph_base_init(g, num_vts, 0);
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Initializes the second graph with five vertices.
*/
void second_vfive_graph_init(graph_t *g){
  uint64_t u[] = {0, 1, 2, 3};
  uint64_t v[] = {1, 2, 3, 4};
  uint64_t num_vts = 5;
  graph_base_init(g, num_vts, 0);
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Run bfs tests on small graphs.
*/
void vfive_graph_helper(const graph_t *g,
			uint64_t ret_dist[][5],
			uint64_t ret_prev[][5],
			void (*build)(adj_lst_t *, const graph_t *),
			int *res);

void run_first_vfive_graph_test(){
  int res = 1;
  uint64_t dir_dist[5][5] = {{0, 1, 1, 1, 0},
			     {0, 0, 0, 1, 0},
			     {0, 0, 0, 0, 0},
			     {0, 0, 0, 0, 0},
			     {0, 0, 0, 0, 0}};
  uint64_t dir_prev[5][5] = {{0, 0, 0, 0, NR},
			     {NR, 1, NR, 1, NR},
			     {NR, NR, 2, NR, NR},
			     {NR, NR, NR, 3, NR},
			     {NR, NR, NR, NR, 4}};
  uint64_t undir_dist[5][5] = {{0, 1, 1, 1, 0},
			       {1, 0, 2, 1, 0},
			       {1, 2, 0, 2, 0},
			       {1, 1, 2, 0, 0},
			       {0, 0, 0, 0, 0}};
  uint64_t undir_prev[5][5] = {{0, 0, 0, 0, NR},
			       {1, 1, 0, 1, NR},
			       {2, 0, 2, 0, NR},
			       {3, 3, 0, 3, NR},
			       {NR, NR, NR, NR, 4}};
  graph_t g;
  printf("Run a bfs test on the first small graph instance --> ");
  first_vfive_graph_init(&g);
  vfive_graph_helper(&g, dir_dist, dir_prev, adj_lst_dir_build, &res);
  vfive_graph_helper(&g, undir_dist, undir_prev, adj_lst_undir_build, &res);
  graph_free(&g);
  print_test_result(res);
}

void run_second_vfive_graph_test(){
  int res = 1;
  uint64_t dir_dist[5][5] = {{0, 1, 2, 3, 4},
			     {0, 0, 1, 2, 3},
			     {0, 0, 0, 1, 2},
			     {0, 0, 0, 0, 1},
			     {0, 0, 0, 0, 0}};
  uint64_t dir_prev[5][5] = {{0, 0, 1, 2, 3},
			     {NR, 1, 1, 2, 3},
			     {NR, NR, 2, 2, 3},
			     {NR, NR, NR, 3, 3},
			     {NR, NR, NR, NR, 4}};
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
  graph_t g;
  printf("Run a bfs test on the second small graph instance --> ");
  second_vfive_graph_init(&g);
  vfive_graph_helper(&g, dir_dist, dir_prev, adj_lst_dir_build, &res);
  vfive_graph_helper(&g, undir_dist, undir_prev, adj_lst_undir_build, &res);
  graph_free(&g);
  print_test_result(res);
}

void vfive_graph_helper(const graph_t *g,
			uint64_t ret_dist[][5],
			uint64_t ret_prev[][5],
			void (*build)(adj_lst_t *, const graph_t *),
			int *res){
  uint64_t *dist = NULL, *prev = NULL;
  adj_lst_t a;
  adj_lst_init(&a, g);
  build(&a, g);
  dist = malloc_perror(a.num_vts * sizeof(uint64_t));
  prev = malloc_perror(a.num_vts * sizeof(uint64_t));
  for (uint64_t i = 0; i < a.num_vts; i++){
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

int bern_fn(void *arg){
  bern_arg_t *b = arg;
  if (b->p >= 1.00) return 1;
  if (b->p <= 0.00) return 0;
  if (b->p > DRAND48()) return 1;
  return 0;
}

/**
   Runs a bfs test on directed graphs with n(n - 1) edges.
*/
void run_max_edges_graph_test(){
  int res = 1, pow_end = 15;
  uint64_t n, start;
  uint64_t *dist = NULL, *prev = NULL;
  bern_arg_t b;
  adj_lst_t a;
  b.p = 1.00;
  SEED(time(0));
  printf("Run a bfs test on graphs with n vertices, where "
	 "0 < n <= 2^%d, and n(n - 1) edges --> ", pow_end - 1);
  fflush(stdout);
  for (int i = 0; i < pow_end; i++){
    n = pow_two(i); // 0 < n
    dist = malloc_perror(n * sizeof(uint64_t));
    prev = malloc_perror(n * sizeof(uint64_t));
    adj_lst_rand_dir(&a, n, bern_fn, &b);
    start =  RANDOM() % n;
    bfs(&a, start, dist, prev);
    for (uint64_t j = 0; j < n; j++){
      if (j == start){
	res *= (dist[j] == 0);
      }else{
	res *= (dist[j] == 1);
      }
      res *= (prev[j] == start);
    }
    adj_lst_free(&a);
    free(dist);
    free(prev);
    dist = NULL;
    prev = NULL;
  }
  print_test_result(res);
}

/** 
   Test bfs on graphs with no edges.
*/

/**
   Runs a bfs test on directed graphs with no edges.
*/
void run_no_edges_graph_test(){
  int res = 1, pow_end = 15;
  uint64_t n, start;
  uint64_t *dist = NULL, *prev = NULL;
  bern_arg_t b;
  adj_lst_t a;
  b.p = 0.00;
  SEED(time(0));
  printf("Run a bfs test on graphs with n vertices, where "
	 "0 < n <= 2^%d, and no edges --> ", pow_end - 1);
  fflush(stdout);
  for (int i = 0; i < pow_end; i++){
    n = pow_two(i); // 0 < n
    dist = malloc_perror(n * sizeof(uint64_t));
    prev = malloc_perror(n * sizeof(uint64_t));
    adj_lst_rand_dir(&a, n, bern_fn, &b);
    start =  RANDOM() % n;
    bfs(&a, start, dist, prev);
    for (uint64_t j = 0; j < n; j++){
      if (j == start){
	res *= (prev[j] == start);
      }else{
	res *= (prev[j] == NR);
      }
      res *= (dist[j] == 0);
    }
    adj_lst_free(&a);
    free(dist);
    free(prev);
    dist = NULL;
    prev = NULL;
  }
  print_test_result(res);
}

/** 
   Test bfs on random directed graphs.
*/

/**
   Runs a bfs test on random directed graphs.
*/
void run_random_dir_graph_test(){
  int pow_end = 15, ave_iter = 10;
  int num_p = 5;
  uint64_t n, start[10];
  uint64_t *dist = NULL, *prev = NULL;
  double p[5] = {1.00, 0.75, 0.50, 0.25, 0.00};
  bern_arg_t b;
  adj_lst_t a;
  clock_t t;
  printf("Run a bfs test on random directed graphs, from %d random "
	 "start vertices in each graph \n", ave_iter);
  fflush(stdout);
  SEED(time(0));
  for (int i = 0; i < num_p; i++){
    b.p = p[i];
    printf("\tP[an edge is in a graph] = %.2f\n", b.p);
    for (int j = 0; j <  pow_end; j++){
      n = pow_two(j); // 0 < n
      dist = malloc_perror(n * sizeof(uint64_t));
      prev = malloc_perror(n * sizeof(uint64_t));
      adj_lst_rand_dir(&a, n, bern_fn, &b);
      for (int k = 0; k < ave_iter; k++){
	start[k] =  RANDOM() % n;
      }
      t = clock();
      for (int k = 0; k < ave_iter; k++){
	bfs(&a, start[k], dist, prev);
      }
      t = clock() - t;
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f, "
	     "average runtime: %.6f seconds\n",
	     n, b.p * n * (n - 1), (float)t / ave_iter / CLOCKS_PER_SEC);
      fflush(stdout);
      adj_lst_free(&a);
      free(dist);
      free(prev);
      dist = NULL;
      prev = NULL;
    }
  }
}

/**
   Compares elements of two uint64_t arrays.
*/
int cmp_arr(const uint64_t *a, const uint64_t *b, uint64_t n){
  int res = 1;
  for (uint64_t i = 0; i < n; i++){
    res *= (a[i] == b[i]);
  }
  return res;
}

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
uint64_t pow_two(int k){
  uint64_t ret = 1 << k;
  return ret;
}

void print_test_result(int res){
  if (res){
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
