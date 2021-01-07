/**
   dijkstra-uint64-main.c

   Examples of running Dijkstra's algorithm on a graph with generic 
   non-negative weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "dijkstra-uint64.h"
#include "bfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"
#include "utilities-rand-mod.h"

#define SEED(s) do{srandom(s); srand48(s);} while (0)
#define RANDOM() (random())
#define DRAND48() (drand48())

int cmp_uint64_arrs(uint64_t *a, uint64_t *b, uint64_t n);
void norm_uint64_arr(uint64_t *a, uint64_t norm, uint64_t n);
void print_test_result(int result);

static const uint64_t nr = 0xffffffffffffffff; //not reached

/** 
    Graphs with uint64_t weights.
*/

/**
   Initialize graphs with uint64_t weights.
*/
void graph_uint64_wts_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t wts[] = {4, 3, 2, 1};
  graph_base_init(g, 5, sizeof(uint64_t));
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((uint64_t *)g->wts + i) = wts[i];
  }
}

void graph_uint64_wts_no_edges_init(graph_t *g){
  graph_base_init(g, 5, sizeof(uint64_t));
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
  
void print_adj_lst(adj_lst_t *a,
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
  
int cmp_uint64_fn(void *wt_a, void *wt_b){
  uint64_t wt_a_val = *(uint64_t *)wt_a;
  uint64_t wt_b_val = *(uint64_t *)wt_b;
  if (wt_a_val > wt_b_val){
    return 1;
  }else if (wt_a_val < wt_b_val){
    return -1;
  }else{
    return 0;
  }
}

void run_uint64_dijkstra(adj_lst_t *a){
  uint64_t *dist = malloc_perror(a->num_vts * sizeof(uint64_t));
  uint64_t *prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  for (uint64_t i = 0; i < a->num_vts; i++){
    dijkstra_uint64(a,
		    i,
		    dist,
		    prev,
		    init_uint64_fn,
		    add_uint64_fn,
		    cmp_uint64_fn);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint64_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}
  
void run_uint64_graph_test(){
  graph_t g;
  adj_lst_t a;
  //graph with edges
  graph_uint64_wts_init(&g);
  printf("Running directed uint64_t graph test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running undirected uint64_t graph test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
  //graph with no edges
  graph_uint64_wts_no_edges_init(&g);
  printf("Running directed uint64_t graph with no edges test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running undirected uint64_t graph with no edges test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
    Graphs with double weights.
*/

/**
   Initialize graphs with double weights.
*/
void graph_double_wts_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  graph_base_init(g, 5, sizeof(double));
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((double *)g->wts + i) = wts[i];
  }
}

void graph_double_wts_no_edges_init(graph_t *g){
  graph_base_init(g, 5, sizeof(double));
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
  
int cmp_double_fn(void *wt_a, void *wt_b){
  double wt_a_val = *(double *)wt_a;
  double wt_b_val = *(double *)wt_b;
  if (wt_a_val > wt_b_val){
    return 1;
  }else if (wt_a_val < wt_b_val){
    return -1;
  }else{
    return 0;
  } 
}

void run_double_dijkstra(adj_lst_t *a){
  double *dist = malloc_perror(a->num_vts * sizeof(double));
  uint64_t *prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  for (uint64_t i = 0; i < a->num_vts; i++){
    dijkstra_uint64(a,
		    i,
		    dist,
		    prev,
		    init_double_fn,
		    add_double_fn,
		    cmp_double_fn);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_double_wts_init(&g);
  printf("Running directed double graph test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running undirected double graph test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
  //graph with no edges
  graph_double_wts_no_edges_init(&g);
  printf("Running directed double graph with no edges test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running undirected double graph with no edges test... \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_dijkstra(&a);
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

int bern_fn(void *arg){
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
  uint64_t rand_val = wt_l + random_range_uint64(wt_h - wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, bern, arg);
}

void add_dir_double_edge(adj_lst_t *a,
			 uint64_t u,
			 uint64_t v,
			 uint64_t wt_l,
			 uint64_t wt_h,
			 int (*bern)(void *),
			 void *arg){
  double rand_val = (double)(wt_l + random_range_uint64(wt_h - wt_l));
  adj_lst_add_dir_edge(a, u, v, &rand_val, bern, arg);
}

void adj_lst_rand_dir_wts(adj_lst_t *a,
			  uint64_t n,
			  int wt_size,
			  uint64_t wt_l,
			  uint64_t wt_h,
			  int (*bern)(void *),
			  void *arg,
			  void (*add_dir_edge_fn)(adj_lst_t *,
						  uint64_t,
						  uint64_t,
						  uint64_t,
						  uint64_t,
						  int (*)(void *),
						  void *arg)){
  graph_t g;
  graph_base_init(&g, n, wt_size);
  adj_lst_init(a, &g);
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      add_dir_edge_fn(a, i, j, wt_l, wt_h, bern, arg);
      add_dir_edge_fn(a, j, i, wt_l, wt_h, bern, arg);
    }
  }
  graph_free(&g);
}

/**
   Test the distance equivalence of bfs and dijkstra on random directed 
   graphs with the same uint64_t weight across edges.
*/
void run_bfs_dijkstra_graph_test(){
  adj_lst_t a;
  int pow_two_start = 10, pow_two_end = 15;
  int iter = 10;
  int result = 1;
  int num_p = 12;
  uint64_t n;
  uint64_t rand_start[iter];
  uint64_t *bfs_dist = NULL, *bfs_prev = NULL;
  uint64_t *dijkstra_dist = NULL, *dijkstra_prev = NULL;
  double p[12] = {1.000000, 0.500000, 0.250000, 0.125000,
		  0.062500, 0.031250, 0.015625, 0.007812,
		  0.003906, 0.001953, 0.000977, 0.000000};
  bern_arg_t b;
  clock_t b_t, d_t;
  printf("Run a bfs and dijkstra_uint64 test on random directed "
	 "graphs with the same weight across edges\n");
  fflush(stdout);
  srandom(time(0));
  for (int pi = 0; pi < num_p; pi++){
    b.p = p[pi];
    printf("\tP[an edge is in a graph] = %.4f\n", p[pi]);
    for (int i = pow_two_start; i <  pow_two_end; i++){
      n = pow_two_uint64(i); //0 < n
      bfs_dist = malloc_perror(n * sizeof(uint64_t));
      bfs_prev = malloc_perror(n * sizeof(uint64_t));
      dijkstra_dist = malloc_perror(n * sizeof(uint64_t));
      dijkstra_prev = malloc_perror(n * sizeof(uint64_t));
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(uint64_t),
			   i,
			   i,
			   bern_fn,
			   &b,
			   add_dir_uint64_edge);
      for(int j = 0; j < iter; j++){
	rand_start[j] = random_range_uint64(n - 1);
      }
      b_t = clock();
      for(int j = 0; j < iter; j++){
	bfs(&a, rand_start[j], bfs_dist, bfs_prev);
      }
      b_t = clock() - b_t;
      d_t = clock();
      for(int j = 0; j < iter; j++){
	dijkstra_uint64(&a,
			rand_start[j],
			dijkstra_dist,
			dijkstra_prev,
			init_uint64_fn,
			add_uint64_fn,
			cmp_uint64_fn);
      }
      d_t = clock() - d_t;
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     a.num_vts, a.num_es);
      printf("\t\t\tbfs ave runtime:         %.8f seconds\n"
	     "\t\t\tdijkstra ave runtime:    %.8f seconds\n",
	     (float)b_t / iter / CLOCKS_PER_SEC,
	     (float)d_t / iter / CLOCKS_PER_SEC);
      fflush(stdout);
      norm_uint64_arr(dijkstra_dist, i, n);
      result *= (cmp_uint64_arrs(bfs_dist, dijkstra_dist, n) == 0);
      printf("\t\t\tcorrectness:             ");
      print_test_result(result);
      adj_lst_free(&a);
      free(bfs_dist);
      free(bfs_prev);
      free(dijkstra_dist);
      free(dijkstra_prev);
      bfs_dist = NULL;
      bfs_prev = NULL;
      dijkstra_dist = NULL;
      dijkstra_prev = NULL;
    }
  }
}

/**
   Test dijkstra_uint64 on random directed graphs with random uint64_t 
   weights.
*/
void run_rand_uint64_wts_graph_test(){
  adj_lst_t a;
  int pow_two_start = 10, pow_two_end = 14;
  int iter = 10;
  int num_p = 12;
  uint64_t n;
  uint64_t all_paths_wt = 0, num_paths = 0;
  uint64_t rand_start[iter];
  uint64_t wt_l = 0, wt_h = pow_two_uint64(32) - 1;
  uint64_t *dijkstra_dist = NULL, *dijkstra_prev = NULL;
  double p[12] = {1.000000, 0.500000, 0.250000, 0.125000,
		  0.062500, 0.031250, 0.015625, 0.007812,
		  0.003906, 0.001953, 0.000977, 0.000000};
  bern_arg_t b;
  clock_t t;
  printf("Run a dijkstra_uint64 test on random directed graphs with random "
	 "uint64_t weights in [%lu, %lu]\n", wt_l, wt_h);
  fflush(stdout);
  srandom(time(0));
  for (int pi = 0; pi < num_p; pi++){
    b.p = p[pi];
    printf("\tP[an edge is in a graph] = %.4f\n", p[pi]);
    for (int i = pow_two_start; i <  pow_two_end; i++){
      n = pow_two_uint64(i); //0 < n
      dijkstra_dist = malloc_perror(n * sizeof(uint64_t));
      dijkstra_prev = malloc_perror(n * sizeof(uint64_t));
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(uint64_t),
			   wt_l,
			   wt_h,
			   bern_fn,
			   &b,
			   add_dir_uint64_edge);
      for(int j = 0; j < iter; j++){
	rand_start[j] = random_range_uint64(n - 1);
      }
      t = clock();
      for(int j = 0; j < iter; j++){
	dijkstra_uint64(&a,
			rand_start[j],
			dijkstra_dist,
			dijkstra_prev,
			init_uint64_fn,
			add_uint64_fn,
			cmp_uint64_fn);
      }
      t = clock() - t;
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     a.num_vts, a.num_es);
      printf("\t\t\tave runtime:                %.8f seconds\n",
	     (float)t / iter / CLOCKS_PER_SEC);
      fflush(stdout);
      for(uint64_t v = 0; v < a.num_vts; v++){
	if (dijkstra_prev[v] != nr){
	  all_paths_wt += dijkstra_dist[v];
	  num_paths++; //< 0
	}
      }
      printf("\t\t\tlast run # paths:           %lu\n", num_paths - 1);
      if (num_paths > 1){
	printf("\t\t\tlast run ave path weight:   %.1lf\n",
	       (double)all_paths_wt / (num_paths - 1));
      }else{
	printf("\t\t\tlast run ave path weight:   none\n");
      }	      
      all_paths_wt = 0;
      num_paths  = 0;
      adj_lst_free(&a);
      free(dijkstra_dist);
      free(dijkstra_prev);
      dijkstra_dist = NULL;
      dijkstra_prev = NULL;
    }
  }
}

/**
   Returns O if two uint64_t arrays are equal.
*/
int cmp_uint64_arrs(uint64_t *a, uint64_t *b, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    if (a[i] > b[i]){return 1;}
    if (a[i] < b[i]){return -1;}
  }
  return 0;
}

/**
   Normalizes a uint64_t array.
*/
void norm_uint64_arr(uint64_t *a, uint64_t norm, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    a[i] = a[i] / norm;
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
  run_bfs_dijkstra_graph_test();
  run_rand_uint64_wts_graph_test();
  return 0;
}
