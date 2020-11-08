/**
   tsp-uint64-main.c

   Examples of running Dijkstra's algorithm on a graph with generic 
   non-negative weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "tsp-uint64.h"
#include "dijkstra-uint64.h"
#include "graph-uint64.h"
#include "stack-uint64.h"
#include "utilities-ds.h"

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
void graph_uint64_wts_init(graph_uint64_t *g){
  uint64_t u[] = {0, 1, 2, 3};
  uint64_t v[] = {1, 2, 3, 0};
  uint64_t wts[] = {1, 1, 1, 1};
  graph_uint64_base_init(g, 4, sizeof(uint64_t));
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
  graph_uint64_base_init(g, 4, sizeof(uint64_t));
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

void run_uint64_tsp(adj_lst_uint64_t *a){
  uint64_t *dist = malloc(sizeof(uint64_t));
  assert(dist != NULL);
  for (uint64_t i = 0; i < a->num_vts; i++){
    tsp_uint64(a,
	       i,
	       dist,
	       init_uint64_fn,
	       add_uint64_fn,
	       cmp_uint64_fn);
    printf("tour lenghts with %lu as start \n", i);
    print_uint64_arr(dist, 1);
  }
  free(dist);
  dist = NULL;
}

void run_uint64_graph_test(){
  graph_uint64_t g;
  adj_lst_uint64_t a;
  //graph with edges
  graph_uint64_wts_init(&g);
  printf("Running directed uint64_t graph test... \n\n");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_uint64_tsp(&a);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}

int main(){
  run_uint64_graph_test();
  return 0;
}
