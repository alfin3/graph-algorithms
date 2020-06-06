/**
   bfs_main.c

   Examples of running the BFS algorithm. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"
#include "graph.h"
#include "bfs.h"

/**  
    First graph example.
*/

/**
   Initializes the first instance of a graph.
*/
void first_graph_test_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  g->num_vts = 5;
  g->num_e = 4;
  g->wt_size = 0;
  g->u = malloc(g->num_e * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_e * sizeof(int));
  assert(g->v != NULL);
  g->wts = NULL;
  for (int i = 0; i < g->num_e; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Frees the dynamically allocated arrays of a graph.
*/
void graph_free(graph_t *g){
  free(g->u);
  free(g->v);
  g->u = NULL;
  g->v = NULL;
}

/**
   Print the adjacency list of a graph.
*/
void print_all_int_elts(stack_t *s);
void print_int_arr(int *arr, int n);
  
void print_adj_lst(adj_lst_t *a){
  printf("Print vertices... \n\n");
  for (int i = 0; i < a->num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a->vts[i]);
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
   Run the first test example.
*/
void run_bfs(adj_lst_t *a){
  int *dist = malloc(a->num_vts * sizeof(int));
  int *prev = malloc(a->num_vts * sizeof(int));
  for (int i = 0; i < a->num_vts; i++){
    bfs(a, i, dist, prev);
    printf("distances and previous vertices with %d as start \n", i);
    print_int_arr(dist, a->num_vts);
    print_int_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}
  
void run_first_graph_test(){
  graph_t g;
  adj_lst_t a;
  first_graph_test_init(&g);
  printf("Running directed first graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  print_adj_lst(&a);
  run_bfs(&a);
  adj_lst_free(&a);
  printf("Running undirected first graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_adj_lst(&a);
  run_bfs(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
    Second graph example.
*/

/**
   Initializes the second instance of a graph.
*/
void second_graph_test_init(graph_t *g){
  int u[] = {0, 1, 2, 3};
  int v[] = {1, 2, 3, 4};
  g->num_vts = 5;
  g->num_e = 4;
  g->wt_size = 0;
  g->u = malloc(g->num_e * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_e * sizeof(int));
  assert(g->v != NULL);
  g->wts = NULL;
  for (int i = 0; i < g->num_e; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
  }
}

/**
   Runs the second test example.
*/  
void run_second_graph_test(){
  graph_t g;
  adj_lst_t a;
  second_graph_test_init(&g);
  printf("Running directed second graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  print_adj_lst(&a);
  run_bfs(&a);
  adj_lst_free(&a);
  printf("Running undirected second graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_adj_lst(&a);
  run_bfs(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/** 
    Graph with no edges.
*/

/**
   Initializes an instance of a graph with no edges.
*/
void no_edges_graph_init(graph_t *g){
  g->num_vts = 5;
  g->num_e = 0;
  g->wt_size = 0;
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Runs a test example of a graph with no edges.
*/
void run_no_edges_graph_test(){
  graph_t g;
  adj_lst_t a;
  no_edges_graph_init(&g);
  printf("Running directed graph with no edges test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  print_adj_lst(&a);
  run_bfs(&a);
  adj_lst_free(&a);
  printf("Running undirected graph with no edges test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_adj_lst(&a);
  run_bfs(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

int main(){
  run_first_graph_test();
  run_second_graph_test();
  run_no_edges_graph_test();
}
  
