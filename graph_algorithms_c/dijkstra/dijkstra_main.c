/**
   dijkstra_main.c

   Examples of running Dijkstra's algorithm on a graph with generic 
   non-negative weights.

   Edge weights are of any basic type (e.g. char, int, double, long double).
   Edge weight initialization and operations are defined in init_(type)_fn, 
   add_(type)_fn, and cmp_(type)_fn functions. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dijkstra.h"

/** 
    Graph with integer weights.
*/

/**
   Initializes an instance of a graph with integer weights.
*/
void int_graph_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  int wts[] = {4, 3, 2, 1};
  g->num_vts = 5;
  g->num_e = 4;
  g->wt_size = sizeof(int);
  g->u = malloc(g->num_e * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_e * sizeof(int));
  assert(g->v != NULL);
  g->wts = malloc(g->num_e * g->wt_size);
  assert(g->wts != NULL);
  for (int i = 0; i < g->num_e; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((int *)(g->wts) + i) = wts[i];
  }
}

/**
   Frees the dynamically allocated arrays of a graph.
*/
void graph_free(graph_t *g){
  free(g->u);
  free(g->v);
  free(g->wts);
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Print the adjacency list of a graph with integer weights.
*/
void print_all_int_elts(stack_t *s);
void print_int_arr(int *arr, int n);
  
void print_int_adj_lst(adj_lst_t *a){
  printf("Print vertices... \n\n");
  for (int i = 0; i < a->num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a->vts[i]);
  }
  printf("\n");
  printf("Print weights... \n\n");
  for (int i = 0; i < a->num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a->wts[i]);
  }
  printf("\n");
}

void print_all_int_elts(stack_t *s){
  int *arr = s->elts;
  print_int_arr(arr, s->num_elts);
}

void print_int_arr(int *arr, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", arr[i]);
  }
  printf("\n");
} 

/**
   Run a test example of a graph with integer weights.
*/
void init_int_fn(int s, int i, void *dist_wt){
  if (s == i){
    *(int *)dist_wt = 0;
  }else{
    *(int *)dist_wt = -1; //-1 as infinity
  }
}

void add_int_fn(void *sum, void *u_dist, void *u_v_wt){
  *(int *)sum = *(int *)u_dist + *(int *)u_v_wt;
}
  
int cmp_int_fn(void *v_dist, void *sum){
  if (*(int *)v_dist == -1){return 1;}
  return *(int *)v_dist - *(int *)sum;
}

void run_int_dijkstra(adj_lst_t *a){
  int *dist = malloc(a->num_vts * sizeof(int));
  for (int i = 0; i < a->num_vts; i++){
    printf("start vertex: %d \n", i);
    dijkstra(a, i, dist, init_int_fn, add_int_fn, cmp_int_fn);
    print_int_arr(dist, a->num_vts);
  }
  printf("\n");
  free(dist);
  dist = NULL;
}
  
void run_int_graph_test(){
  graph_t g;
  adj_lst_t a;
  int_graph_init(&g);
  printf("Running directed int graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  print_int_adj_lst(&a);
  run_int_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running undirected int graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_int_adj_lst(&a);
  run_int_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/** 
    Graph with long double weights.
*/

/**
   Initializes an instance of a graph with long double weights.
*/
void long_double_graph_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  long double wts[] = {4.0, 3.0, 2.0, 1.0};
  g->num_vts = 5;
  g->num_e = 4;
  g->wt_size = sizeof(long double);
  g->u = malloc(g->num_e * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_e * sizeof(int));
  assert(g->v != NULL);
  g->wts = malloc(g->num_e * g->wt_size);
  assert(g->wts != NULL);
  for (int i = 0; i < g->num_e; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((long double *)(g->wts) + i) = wts[i];
  }
}

/**
   Print adjacency list of a long double graph.
*/
void print_all_long_double_elts(stack_t *s);
void print_long_double_arr(long double *arr, int n);
  
void print_long_double_adj_lst(adj_lst_t *a){
  printf("Print vertices... \n\n");
  for (int i = 0; i < a->num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a->vts[i]);
  }
  printf("\n");
  printf("Print weights... \n\n");
  for (int i = 0; i < a->num_vts; i++){
    printf("%d : ", i);
    print_all_long_double_elts(a->wts[i]);
  }
  printf("\n");
}

void print_all_long_double_elts(stack_t *s){
  long double *arr = s->elts;
  print_long_double_arr(arr, s->num_elts);
}

void print_long_double_arr(long double *arr, int n){
  for (int i = 0; i < n; i++){
    printf("%.1Lf ", arr[i]);
  }
  printf("\n");
}

/**
   Run a test example of a graph with long double weights.
*/
void init_long_double_fn(int s, int i, void *dist_wt){
  if (s == i){
    *(long double *)dist_wt = 0.0;
  }else{
    *(long double *)dist_wt = -1.0; //-1.0 as infinity
  }
}

void add_long_double_fn(void *sum, void *u_dist, void *u_v_wt){
  *(long double *)sum = *(long double *)u_dist + *(long double *)u_v_wt;
}
  
int cmp_long_double_fn(void *v_dist, void *sum){
  long double v_dist_val = *(long double *)v_dist;
  long double sum_val = *(long double *)sum;
  if (v_dist_val < 0.0 || v_dist_val > sum_val){
    return 1;
  }else if (v_dist_val < sum_val){
    return -1;
  }else{
    return 0;
  } 
}

void run_long_double_dijkstra(adj_lst_t *a){
  long double *dist = malloc(a->num_vts * sizeof(long double));
  for (int i = 0; i < a->num_vts; i++){
    printf("start vertex: %d \n", i);
    dijkstra(a,
	     i,
	     dist,
	     init_long_double_fn,
	     add_long_double_fn,
	     cmp_long_double_fn);
    print_long_double_arr(dist, a->num_vts);
  }
  printf("\n");
  free(dist);
  dist = NULL;
}

void run_long_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  long_double_graph_init(&g);
  printf("Running directed long double graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  print_long_double_adj_lst(&a);
  run_long_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running undirected long double graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_long_double_adj_lst(&a);
  run_long_double_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/** 
    Graph with integer weights but no edges.
*/

/**
   Initializes an instance of a graph with integer weights but no edges.
*/
void int_no_edges_graph_init(graph_t *g){
  g->num_vts = 5;
  g->num_e = 0;
  g->wt_size = sizeof(int);
  g->wts = NULL;
  g->u = NULL;
  g->v = NULL;
}

/**
   Prints adjacency list of a graph with integer weights but no edges.
*/
void print_int_no_edges_adj_lst(adj_lst_t *a){
  printf("Print vertices... \n\n");
  for (int i = 0; i < a->num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a->vts[i]);
  }
  printf("\n");
}

/**
   Run a test example of a graph with integer weights but no edges.
*/
void run_int_no_edges_graph_test(){
  graph_t g;
  adj_lst_t a;
  int_no_edges_graph_init(&g);
  printf("Running directed int graph with no edges test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  print_int_no_edges_adj_lst(&a);
  run_int_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running undirected int graph with no edges test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_int_no_edges_adj_lst(&a);
  run_int_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

int main(){
  run_int_graph_test();
  run_long_double_graph_test();
  run_int_no_edges_graph_test();
}
  
