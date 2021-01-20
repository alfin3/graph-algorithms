/**
   tsp-uint64-main.c

   Examples of running a dynamic programming version of an exact solution 
   of TSP with generic weights, including negative weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "tsp-uint64.h"
#include "graph-uint64.h"
#include "stack.h"
#include "utilities-rand-mod.h"

void print_test_result(int result);

static const uint64_t nr = 0xffffffffffffffff; //not reached

/** 
    Graphs with uint64_t weights.
*/

/**
   Initialize graphs with uint64_t weights.
*/
void graph_uint64_wts_init(graph_uint64_t *g){
  uint64_t u[] = {0, 1, 2, 3, 1, 2, 3, 0, 0, 2, 1, 3};
  uint64_t v[] = {1, 2, 3, 0, 0, 1, 2, 3, 2, 0, 3, 1};
  uint64_t wts[] = {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2};
  graph_uint64_base_init(g, 4, sizeof(uint64_t));
  g->num_es = 12;
  g->u = malloc(g->num_es * sizeof(uint64_t));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(uint64_t));
  assert(g->v != NULL);
  g->wts = malloc(g->num_es * g->wt_size);
  assert(g->wts != NULL);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((uint64_t *)g->wts + i) = wts[i];
  }
}

void graph_uint64_single_vt_init(graph_uint64_t *g){
  graph_uint64_base_init(g, 1, sizeof(uint64_t));
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
  
void print_adj_lst(adj_lst_uint64_t *a,
		   void (*print_wts_fn)(stack_t *)){
  printf("\tvertices: \n");
  for (uint64_t i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", i);
    print_uint64_elts(a->vts[i]);
  }
  if (print_wts_fn != NULL){
    printf("\tweights: \n");
    for (uint64_t i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", i);
      print_wts_fn(a->wts[i]);
    }
  }
  printf("\n");
}

void print_uint64_arr(uint64_t *arr, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    if (arr[i] == nr){
      printf("nr ");
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
   Run a test on graphs with uint64_t weights.
*/
void init_uint64_fn(void *wt){
  *(uint64_t *)wt = 0;
}

void add_uint64_fn(void *sum, void *wt_a, void *wt_b){
  *(uint64_t *)sum = *(uint64_t *)wt_a + *(uint64_t *)wt_b;
}
  
int cmp_uint64_fn(const void *wt_a, const void *wt_b){
  if (*(uint64_t *)wt_a > *(uint64_t *)wt_b){
    return 1;
  }else if (*(uint64_t *)wt_a < *(uint64_t *)wt_b){
    return -1;
  }else{
    return 0;
  }
}

void run_uint64_tsp(adj_lst_uint64_t *a){
  int ret = -1;
  uint64_t *dist = malloc(sizeof(uint64_t));
  assert(dist != NULL);
  for (uint64_t i = 0; i < a->num_vts; i++){
    ret = tsp_uint64(a,
		     i,
		     dist,
		     init_uint64_fn,
		     add_uint64_fn,
		     cmp_uint64_fn);
    printf("tsp_uint64 ret: %d, tour length with %lu as start: ", ret, i);
    print_uint64_arr(dist, 1);
  }
  printf("\n");
  free(dist);
  dist = NULL;
}

void run_uint64_graph_test(){
  graph_uint64_t g;
  adj_lst_uint64_t a;
  //graph with > 1 vertices
  graph_uint64_wts_init(&g);
  printf("Running uint64_t graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_tsp(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
  //graph with a single vertex
  graph_uint64_single_vt_init(&g);
  printf("Running uint64_t graph with a single vertex test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_tsp(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}

/**
    Graphs with double weights.
*/

void graph_double_wts_init(graph_uint64_t *g){
  uint64_t u[] = {0, 1, 2, 3, 1, 2, 3, 0, 0, 2, 1, 3};
  uint64_t v[] = {1, 2, 3, 0, 0, 1, 2, 3, 2, 0, 3, 1};
  double wts[] = {1.0, 1.0, 1.0, 1.0,
		  2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0};
  graph_uint64_base_init(g, 4, sizeof(double));
  g->num_es = 12;
  g->u = malloc(g->num_es * sizeof(uint64_t));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(uint64_t));
  assert(g->v != NULL);
  g->wts = malloc(g->num_es * g->wt_size);
  assert(g->wts != NULL);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((double *)g->wts + i) = wts[i];
  }
}

void graph_double_single_vt_init(graph_uint64_t *g){
  graph_uint64_base_init(g, 1, sizeof(double));
}

/**
   Run a test on graphs with double weights.
*/
void init_double_fn(void *wt){
  *(double *)wt = 0.0;
}

void add_double_fn(void *sum, void *wt_a, void *wt_b){
  *(double *)sum = *(double *)wt_a + *(double *)wt_b;
}
  
int cmp_double_fn(const void *wt_a, const void *wt_b){
  if (*(double *)wt_a > *(double *)wt_b){
    return 1;
  }else if (*(double *)wt_a < *(double *)wt_b){
    return -1;
  }else{
    return 0;
  } 
}

void run_double_tsp(adj_lst_uint64_t *a){
  int ret = -1;
  double *dist = malloc(sizeof(double));
  assert(dist != NULL);
  for (uint64_t i = 0; i < a->num_vts; i++){
    ret = tsp_uint64(a,
		     i,
		     dist,
		     init_double_fn,
		     add_double_fn,
		     cmp_double_fn);
    printf("tsp_uint64 ret: %d, tour length with %lu as start: ", ret, i);
    print_double_arr(dist, 1);
  }
  printf("\n");
  free(dist);
  dist = NULL;
}

void run_double_graph_test(){
  graph_uint64_t g;
  adj_lst_uint64_t a;
  //graph with > 1 vertices
  graph_double_wts_init(&g);
  printf("Running double graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_tsp(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
  //graph with a single vertex
  graph_double_single_vt_init(&g);
  printf("Running double graph with a single vertex test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_tsp(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}

/** 
    Construct adjacency lists of random directed graphs with random 
    non-tour weights and a known tour.
*/

void add_dir_uint64_edge(adj_lst_uint64_t *a,
			 uint64_t u,
			 uint64_t v,
			 uint32_t num,
			 uint32_t denom,
			 uint64_t wt_l,
			 uint64_t wt_h){
  uint64_t rand_val;
  uint64_t prev_num_es = a->num_es;
  adj_lst_uint64_add_dir_edge(a, u, v, num, denom);
  if (prev_num_es < a->num_es){
    rand_val = wt_l + random_range_uint64(wt_h - wt_l);
    stack_push(a->wts[u], &rand_val);
  }
}

void add_dir_double_edge(adj_lst_uint64_t *a,
			 uint64_t u,
			 uint64_t v,
			 uint32_t num,
			 uint32_t denom,
			 uint64_t wt_l,
			 uint64_t wt_h){
  double rand_val;
  uint64_t prev_num_es = a->num_es;
  adj_lst_uint64_add_dir_edge(a, u, v, num, denom);
  if (prev_num_es < a->num_es){
    rand_val = (double)(wt_l + random_range_uint64(wt_h - wt_l));
    stack_push(a->wts[u], &rand_val);
  }
}

void adj_lst_rand_dir_wts(adj_lst_uint64_t *a,
			  uint64_t n,
			  int wt_size,
			  uint32_t num,
			  uint32_t denom,
			  uint64_t wt_l,
			  uint64_t wt_h,
			  void (*add_dir_edge_fn)(adj_lst_uint64_t *,
						  uint64_t,
						  uint64_t,
						  uint32_t,
						  uint32_t,
						  uint64_t,
						  uint64_t)){
  assert(n > 0 && num <= denom && denom > 0);
  graph_uint64_t g;
  graph_uint64_base_init(&g, n, wt_size);
  adj_lst_uint64_init(a, &g);
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      if (j - i == 1){
	add_dir_edge_fn(a, i, j, 1, 1, 1, 1);
	add_dir_edge_fn(a, j, i, num, denom, wt_l, wt_h);
      }else if (i == 0 && j == n - 1){
	add_dir_edge_fn(a, i, j, num, denom, wt_l, wt_h);
	add_dir_edge_fn(a, j, i, 1, 1, 1, 1);
      }else{
	add_dir_edge_fn(a, i, j, num, denom, wt_l, wt_h);
	add_dir_edge_fn(a, j, i, num, denom, wt_l, wt_h);
      }
    }
  }
  graph_uint64_free(&g);
}

/**
   Test tsp_uint64 on random directed graphs with random uint64_t non-tour 
   weights and a known tour.
*/
void run_rand_uint64_wts_graph_test(){
  adj_lst_uint64_t a;
  int pow_two_start = 2, pow_two_end = 5;
  int num_nums = 5;
  int ret = -1;
  uint64_t n;
  uint64_t wt_l = 2, wt_h = pow_two_uint64(32) - 1;
  uint64_t tsp_dist;
  uint32_t nums[] = {0, 1, 2, 4, 8};
  uint32_t denom = 8;
  clock_t t;
  printf("Run a tsp_uint64 test on random directed graphs with random "
	 "uint64_t non-tour weights in [%lu, %lu]\n", wt_l, wt_h);
  fflush(stdout);
  srandom(time(0));
  for (int num_ix = 0; num_ix < num_nums; num_ix++){
    printf("\tP[a non-tour edge is in a graph] = %.4f\n",
	   (float)nums[num_ix] / denom);
    for (int i = pow_two_start; i <  pow_two_end; i++){
      n = pow_two_uint64(i); //1 < n
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(uint64_t),
			   nums[num_ix],
			   denom,
			   wt_l,
			   wt_h,
			   add_dir_uint64_edge);
      t = clock();
      ret = tsp_uint64(&a,
		       random_range_uint64(n - 1),
		       &tsp_dist,
		       init_uint64_fn,
		       add_uint64_fn,
		       cmp_uint64_fn);
      t = clock() - t;
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     a.num_vts, a.num_es);
      printf("\t\t\truntime:     %.6f seconds\n",
	     (float)t / CLOCKS_PER_SEC);
      fflush(stdout);
      printf("\t\t\tcorrectness: ");
      print_test_result((tsp_dist == n && ret == 0));
      adj_lst_uint64_free(&a);
    }
  }
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_uint64_graph_test();
  run_double_graph_test();
  run_rand_uint64_wts_graph_test();
  return 0;
}
