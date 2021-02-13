/**
   tsp-main.c

   Tests a dynamic programming version of an exact solution of TSP without
   revisiting and with generic weights, including negative weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "tsp.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"
#include "utilities-rand-mod.h"

#define RGENS_SEED() do{srandom(time(0)); srand48(random());}while (0)
#define RANDOM() (random())
#define DRAND48() (drand48())

static const size_t NR = SIZE_MAX; //not reached as index

uint64_t pow_two(int k);
void print_test_result(int res);

/**
   Initialize small graphs with uint64_t weights.
*/

void graph_uint64_wts_init(graph_t *g){
  uint64_t u[] = {0, 1, 2, 3, 1, 2, 3, 0, 0, 2, 1, 3};
  uint64_t v[] = {1, 2, 3, 0, 0, 1, 2, 3, 2, 0, 3, 1};
  uint64_t wts[] = {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2};
  graph_base_init(g, 4, sizeof(uint64_t));
  g->num_es = 12;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((uint64_t *)g->wts + i) = wts[i];
  }
}

void graph_uint64_single_vt_init(graph_t *g){
  graph_base_init(g, 1, sizeof(uint64_t));
}

/**
   Printing helper functions.
*/
void print_uint64_elts(stack_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%lu ", *((uint64_t *)s->elts + i));
  }
  printf("\n");
}

void print_double_elts(stack_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%.2lf ", *((double *)s->elts + i));
  }
  printf("\n");
}
  
void print_adj_lst(adj_lst_t *a, void (*print_wts)(stack_t *)){
  printf("\tvertices: \n");
  for (uint64_t i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", i);
    print_uint64_elts(a->vts[i]);
  }
  if (print_wts != NULL){
    printf("\tweights: \n");
    for (uint64_t i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", i);
      print_wts(a->wts[i]);
    }
  }
  printf("\n");
}

void print_uint64_arr(uint64_t *arr, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    if (arr[i] == NR){
      printf("NR ");
    }else{
      printf("%lu ", arr[i]);
    }
  }
  printf("\n");
}

void print_double_arr(double *arr, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    printf("%.2lf ", arr[i]);
  }
  printf("\n");
}

/**
   Run a test on small graphs with uint64_t weights.
*/

void add_uint64(void *sum, const void *wt_a, const void *wt_b){
  *(uint64_t *)sum = *(uint64_t *)wt_a + *(uint64_t *)wt_b;
}
  
int cmp_uint64(const void *wt_a, const void *wt_b){
  if (*(uint64_t *)wt_a > *(uint64_t *)wt_b){
    return 1;
  }else if (*(uint64_t *)wt_a < *(uint64_t *)wt_b){
    return -1;
  }else{
    return 0;
  }
}

void run_uint64_tsp(adj_lst_t *a){
  int ret = -1;
  uint64_t dist;
  for (uint64_t i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, add_uint64, cmp_uint64);
    printf("tsp_uint64 ret: %d, tour length with %lu as start: ", ret, i);
    print_uint64_arr(&dist, 1);
  }
  printf("\n");
}

void run_uint64_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_uint64_wts_init(&g);
  printf("Running uint64_t graph test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_uint64_single_vt_init(&g);
  printf("Running uint64_t graph with a single vertex test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
   Initialize small graphs with double weights.
*/

void graph_double_wts_init(graph_t *g){
  uint64_t u[] = {0, 1, 2, 3, 1, 2, 3, 0, 0, 2, 1, 3};
  uint64_t v[] = {1, 2, 3, 0, 0, 1, 2, 3, 2, 0, 3, 1};
  double wts[] = {1.0, 1.0, 1.0, 1.0,
		  2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0};
  graph_base_init(g, 4, sizeof(double));
  g->num_es = 12;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((double *)g->wts + i) = wts[i];
  }
}

void graph_double_single_vt_init(graph_t *g){
  graph_base_init(g, 1, sizeof(double));
}

/**
   Run a test on small graphs with double weights.
*/

void add_double(void *sum, const void *wt_a, const void *wt_b){
  *(double *)sum = *(double *)wt_a + *(double *)wt_b;
}
  
int cmp_double(const void *wt_a, const void *wt_b){
  if (*(double *)wt_a > *(double *)wt_b){
    return 1;
  }else if (*(double *)wt_a < *(double *)wt_b){
    return -1;
  }else{
    return 0;
  } 
}

void run_double_tsp(adj_lst_t *a){
  int ret = -1;
  double *dist = malloc_perror(sizeof(double));
  for (uint64_t i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, dist, add_double, cmp_double);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, i);
    print_double_arr(dist, 1);
  }
  printf("\n");
  free(dist);
  dist = NULL;
}

void run_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_double_wts_init(&g);
  printf("Running double graph test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_double_single_vt_init(&g);
  printf("Running double graph with a single vertex test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/** 
    Construct adjacency lists of random directed graphs with random 
    weights.
*/

typedef struct{
  double p;
} bern_arg_t;

int bern(void *arg){
  bern_arg_t *b = arg;
  if (b->p >= 1.000000) return 1;
  if (b->p <= 0.000000) return 0;
  if (b->p > DRAND48()) return 1;
  return 0;
}

void add_dir_uint64_edge(adj_lst_t *a,
			 uint64_t u,
			 uint64_t v,
			 uint64_t wt_l,
			 uint64_t wt_h,
			 int (*bern)(void *),
			 void *arg){
  uint64_t rand_val = wt_l + DRAND48() * (wt_h - wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, bern, arg);
}

void add_dir_double_edge(adj_lst_t *a,
			 uint64_t u,
			 uint64_t v,
			 uint64_t wt_l,
			 uint64_t wt_h,
			 int (*bern)(void *),
			 void *arg){
  double rand_val = wt_l + DRAND48() * (wt_h - wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, bern, arg);
}

void adj_lst_rand_dir_wts(adj_lst_t *a,
			  uint64_t n,
			  int wt_size,
			  uint64_t wt_l,
			  uint64_t wt_h,
			  int (*bern)(void *),
			  void *arg,
			  void (*add_dir_edge)(adj_lst_t *,
					       uint64_t,
					       uint64_t,
					       uint64_t,
					       uint64_t,
					       int (*)(void *),
					       void *)){
  graph_t g;
  bern_arg_t arg_true;
  graph_base_init(&g, n, wt_size);
  adj_lst_init(a, &g);
  arg_true.p = 2.0;
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      if (n == 2){
	add_dir_edge(a, i, j, 1, 1, bern, &arg_true);
	add_dir_edge(a, j, i, 1, 1, bern, &arg_true);
      }else if (j - i == 1){
	add_dir_edge(a, i, j, 1, 1, bern, &arg_true);
	add_dir_edge(a, j, i, wt_l, wt_h, bern, arg);
      }else if (i == 0 && j == n - 1){
	add_dir_edge(a, i, j, wt_l, wt_h, bern, arg);
	add_dir_edge(a, j, i, 1, 1, bern, &arg_true);
      }else{
	add_dir_edge(a, i, j, wt_l, wt_h, bern, arg);
	add_dir_edge(a, j, i, wt_l, wt_h, bern, arg);
      }
    }
  }
  graph_free(&g);
}

/**
   Test tsp_uint64 on random directed graphs with random uint64_t non-tour 
   weights and a known tour.
*/
void run_rand_uint64_test(){
  int num_vts_max = 21;
  int iter = 1;
  int res = 1, ret;
  int num_p = 7;
  uint64_t n, rand_start[iter];
  uint64_t wt_l = 0, wt_h = pow_two(32) - 1;
  uint64_t dist;
  double p[7] = {1.000000, 0.250000, 0.062500,
		 0.015625, 0.003906, 0.000977,
		 0.000000};
  adj_lst_t a;
  bern_arg_t b;
  clock_t t;
  printf("Run a tsp test on random directed graphs with random "
	 "uint64_t non-tour weights in [%lu, %lu]\n", wt_l, wt_h);
  fflush(stdout);
  for (int pi = 0; pi < num_p; pi++){
    b.p = p[pi];
    printf("\tP[an edge is in a graph] = %.4f\n", p[pi]);
    for (int i = 1; i < num_vts_max; i++){
      n = i;
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(uint64_t),
			   wt_l,
			   wt_h,
			   bern,
			   &b,
			   add_dir_uint64_edge);
      for(int j = 0; j < iter; j++){
	rand_start[j] = DRAND48() * (n - 1);
      }
      t = clock();
      for(int j = 0; j < iter; j++){
	ret = tsp(&a,
		  rand_start[j],
		  &dist,
		  add_uint64,
		  cmp_uint64);
      }
      t = clock() - t;
      if (n == 1){
	res *= (dist == 0 && ret == 0);
      }else{
	res *= (dist == n && ret == 0);
      }
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     a.num_vts, a.num_es);
      printf("\t\t\truntime:     %.6f seconds\n",
	     (float)t / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness: ");
      print_test_result(res);
      res = 1;
      adj_lst_free(&a);
    }
  }
}

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
uint64_t pow_two(int k){
  uint64_t ret = 1;
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
  run_uint64_graph_test();
  run_double_graph_test();
  run_rand_uint64_test();
  return 0;
}
