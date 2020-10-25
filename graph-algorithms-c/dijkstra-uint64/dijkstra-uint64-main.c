/**
   dijkstra-uint64-main.c

   Examples of running Dijkstra's algorithm on a graph with generic 
   non-negative weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "dijkstra-uint64.h"
#include "graph-uint64.h"
#include "stack-uint64.h"
#include "utilities-ds.h"

static const uint64_t nr = 0xffffffffffffffff; //not reached

/** 
    Graph with uint64_t weights.
*/

/**
   Initializes an instance of a graph with uint64_t weights.
*/
void graph_uint64_wts_init(graph_uint64_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t wts[] = {4, 3, 2, 1};
  g->num_vts = 5;
  g->num_es = 4;
  g->wt_size = sizeof(uint64_t);
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
  printf("\tweights: \n");
  for (uint64_t i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", i);
    print_wts_fn(a->wts[i]);
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
   Run a test example of a graph with uint64_t weights.
*/
void init_uint64_fn(void *wt){
  *(uint64_t *)wt = 0;
}

void add_uint64_fn(void *sum, void *wt_a, void *wt_b){
  *(uint64_t *)sum = *(uint64_t *)wt_a + *(uint64_t *)wt_b;
}
  
int cmp_uint64_fn(void *wt_a, void *wt_b){
  return *(uint64_t *)wt_a - *(uint64_t *)wt_b;
}

void run_uint64_dijkstra(adj_lst_uint64_t *a){
  uint64_t *dist = malloc(a->num_vts * sizeof(uint64_t));
  uint64_t *prev = malloc(a->num_vts * sizeof(uint64_t));
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
  graph_uint64_t g;
  adj_lst_uint64_t a;
  graph_uint64_wts_init(&g);
  printf("Running directed uint64_t graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_dijkstra(&a);
  adj_lst_uint64_free(&a);
  printf("Running undirected uint64_t graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_dijkstra(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}

/**
    Graph with double weights.
*/

/**
   Initializes an instance of a graph with double weights.
*/
void double_graph_init(graph_uint64_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  g->num_vts = 5;
  g->num_es = 4;
  g->wt_size = sizeof(double);
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

/**
   Run a test example of a graph with double weights.
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

void run_double_dijkstra(adj_lst_uint64_t *a){
  double *dist = malloc(a->num_vts * sizeof(double));
  uint64_t *prev = malloc(a->num_vts * sizeof(uint64_t));
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
  graph_uint64_t g;
  adj_lst_uint64_t a;
  double_graph_init(&g);
  printf("Running directed double graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_dijkstra(&a);
  adj_lst_uint64_free(&a);
  printf("Running undirected double graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_double_dijkstra(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}
int main(){
  run_uint64_graph_test();
  run_double_graph_test();
}
