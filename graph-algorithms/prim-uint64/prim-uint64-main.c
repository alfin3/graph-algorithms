/**
   prim-uint64-main.c

   Examples of running Prim's algorithm on an undirected graph with 
   generic weights, including negative weights.

   If there are vertices outside the connected component of start, an mst of 
   the connected component of start is computed.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "prim-uint64.h"
#include "graph-uint64.h"
#include "stack-uint64.h"
#include "utilities-rand-mod.h"

static const uint64_t nr = 0xffffffffffffffff; //not reached

/** 
    Graphs with uint64_t weights.
*/

/**
   Initialize graphs with uint64_t weights.
*/
void graph_uint64_wts_init(graph_uint64_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t wts[] = {4, 3, 2, 1};
  graph_uint64_base_init(g, 5, sizeof(uint64_t));
  g->num_es = 4;
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

void graph_uint64_wts_no_edges_init(graph_uint64_t *g){
  graph_uint64_base_init(g, 5, sizeof(uint64_t));
}

/**
   Printing helper functions.
*/
void print_uint64_elts(stack_uint64_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%lu ", *((uint64_t *)s->elts + i));
  }
  printf("\n");
}

void print_double_elts(stack_uint64_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%.2lf ", *((double *)s->elts + i));
  }
  printf("\n");
}
  
void print_adj_lst(adj_lst_uint64_t *a,
		   void (*print_wts_fn)(stack_uint64_t *)){
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
  
int cmp_uint64_fn(void *wt_a, void *wt_b){
  uint64_t wt_a_val = *(uint64_t *)wt_a;
  uint64_t wt_b_val  = *(uint64_t *)wt_b;
  if (wt_a_val > wt_b_val){
    return 1;
  }else if (wt_a_val < wt_b_val){
    return -1;
  }else{
    return 0;
  }
}

void run_uint64_prim(adj_lst_uint64_t *a){
  uint64_t *dist = malloc(a->num_vts * sizeof(uint64_t));
  assert(dist != NULL);
  uint64_t *prev = malloc(a->num_vts * sizeof(uint64_t));
  assert(prev != NULL);
  for (uint64_t i = 0; i < a->num_vts; i++){
    prim_uint64(a, i, dist, prev, init_uint64_fn, cmp_uint64_fn);
    printf("mst edge weights and previous vertices with %lu as start \n", i);
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
  graph_uint64_t g;
  adj_lst_uint64_t a;
  //graph with edges
  graph_uint64_wts_init(&g);
  printf("Running undirected uint64_t graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_prim(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
  //graph with no edges
  graph_uint64_wts_no_edges_init(&g);
  printf("Running undirected uint64_t graph with no edges test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_prim(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}

/**
    Graphs with double weights.
*/

/**
   Initialize graphs with double weights.
*/
void graph_double_wts_init(graph_uint64_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  graph_uint64_base_init(g, 5, sizeof(double));
  g->num_es = 4;
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

void graph_double_wts_no_edges_init(graph_uint64_t *g){
  graph_uint64_base_init(g, 5, sizeof(double));
}

/**
   Run a test on graphs with double weights.
*/
void init_double_fn(void *wt){
  *(double *)wt = 0.0;
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

void run_double_prim(adj_lst_uint64_t *a){
  double *dist = malloc(a->num_vts * sizeof(double));
  assert(dist != NULL);
  uint64_t *prev = malloc(a->num_vts * sizeof(uint64_t));
  assert(prev != NULL);
  for (uint64_t i = 0; i < a->num_vts; i++){
    prim_uint64(a, i, dist, prev, init_double_fn, cmp_double_fn);
    printf("mst edge weights and previous vertices with %lu as start \n", i);
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
  graph_uint64_t g;
  adj_lst_uint64_t a;
  //graph with edges
  graph_double_wts_init(&g);
  printf("Running undirected double graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_prim(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
  //graph with no edges
  graph_double_wts_no_edges_init(&g);
  printf("Running undirected double graph with no edges test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_prim(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}

/** 
    Construct adjacency lists of random undirected graphs with random 
    weights.
*/

void add_undir_uint64_edge(adj_lst_uint64_t *a,
			   uint64_t u,
			   uint64_t v,
			   uint32_t num,
			   uint32_t denom,
			   uint64_t wt_l,
			   uint64_t wt_h){
  uint64_t rand_val;
  uint64_t prev_num_es = a->num_es;
  adj_lst_uint64_add_undir_edge(a, u, v, num, denom);
  if (prev_num_es < a->num_es){
    rand_val = wt_l + random_range_uint64(wt_h - wt_l);
    stack_uint64_push(a->wts[u], &rand_val);
    stack_uint64_push(a->wts[v], &rand_val);
  }
}

void add_undir_double_edge(adj_lst_uint64_t *a,
			   uint64_t u,
			   uint64_t v,
			   uint32_t num,
			   uint32_t denom,
			   uint64_t wt_l,
			   uint64_t wt_h){
  double rand_val;
  uint64_t prev_num_es = a->num_es;
  adj_lst_uint64_add_undir_edge(a, u, v, num, denom);
  if (prev_num_es < a->num_es){
    rand_val = (double)(wt_l + random_range_uint64(wt_h - wt_l));
    stack_uint64_push(a->wts[u], &rand_val);
    stack_uint64_push(a->wts[v], &rand_val);
  }
}

void adj_lst_rand_undir_wts(adj_lst_uint64_t *a,
			    uint64_t n,
			    int wt_size,
			    uint32_t num,
			    uint32_t denom,
			    uint64_t wt_l,
			    uint64_t wt_h,
			    void (*add_undir_edge_fn)(adj_lst_uint64_t *,
						      uint64_t,
						      uint64_t,
						      uint32_t,
						      uint32_t,
						      uint64_t,
						      uint64_t)){
  assert(n > 0 && num <= denom && denom > 0);;
  graph_uint64_t g;
  graph_uint64_base_init(&g, n, wt_size);
  adj_lst_uint64_init(a, &g);
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      add_undir_edge_fn(a, i, j, num, denom, wt_l, wt_h);
    }
  }
  graph_uint64_free(&g);
}

/**
   Test prim_uint64 on random undirected graphs with random uint64_t weights.
*/
void run_rand_uint64_wts_graph_test(){
  adj_lst_uint64_t a;
  int pow_two_start = 10, pow_two_end = 14;
  int num_nums = 12;
  int iter = 10;
  uint64_t n;
  uint64_t mst_wt = 0, mst_num_vts = 0;
  uint64_t rand_start[iter];
  uint64_t wt_l = 0, wt_h = pow_two_uint64(32) - 1;
  uint64_t *prim_dist = NULL, *prim_prev = NULL;
  uint32_t nums[] = {1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1, 0};
  uint32_t denom = 1024;
  clock_t t;
  printf("Run a prim_uint64 test on random undirected graphs with random "
	 "uint64_t weights;\nan edge is represented by two directed edges "
	 "with a weight in [%lu, %lu]\n", wt_l, wt_h);
  fflush(stdout);
  srandom(time(0));
  for (int num_ix = 0; num_ix < num_nums; num_ix++){
    printf("\tP[an edge is in a graph] = %.4f\n",
	   (float)nums[num_ix] / denom);
    for (int i = pow_two_start; i <  pow_two_end; i++){
      n = pow_two_uint64(i); // 0 < n
      prim_dist = malloc(n * sizeof(uint64_t));
      assert(prim_dist != NULL);
      prim_prev = malloc(n * sizeof(uint64_t));
      assert(prim_prev != NULL);
      adj_lst_rand_undir_wts(&a,
			     n,
			     sizeof(uint64_t),
			     nums[num_ix],
			     denom,
			     wt_l,
			     wt_h,
			     add_undir_uint64_edge);
      for(int j = 0; j < iter; j++){
	rand_start[j] = random_range_uint64(n - 1);
      }
      t = clock();
      for(int j = 0; j < iter; j++){
	prim_uint64(&a,
		    rand_start[j],
		    prim_dist,
		    prim_prev,
		    init_uint64_fn,
		    cmp_uint64_fn);
      }
      t = clock() - t;
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     a.num_vts, a.num_es);
      printf("\t\t\tave runtime:               %.8f seconds\n",
	     (float)t / iter / CLOCKS_PER_SEC);
      fflush(stdout);
      for(uint64_t v = 0; v < a.num_vts; v++){
	if (prim_prev[v] != nr){
	  mst_wt += prim_dist[v];
	  mst_num_vts++; //< 0
	}
      }
      printf("\t\t\tlast mst # edges:          %lu\n", mst_num_vts - 1);
      if (mst_num_vts > 1){
	printf("\t\t\tlast mst ave edge weight:  %.1lf\n",
	       (double)mst_wt / (mst_num_vts - 1));
      }else{
	printf("\t\t\tlast mst ave edge weight:  none\n");
      }	      
      mst_wt = 0;
      mst_num_vts  = 0;
      adj_lst_uint64_free(&a);
      free(prim_dist);
      free(prim_prev);
      prim_dist = NULL;
      prim_prev = NULL;
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
}
