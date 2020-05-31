/**
   graph-main.c

   Examples of graphs with generic weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "graph.h"

/** 
    Graph with integer weights.
*/

/**
   Initializes a graph with integer weights.
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
   Prints adjacency list of a graph with integer weights.
*/
void print_all_int_elts(stack_t *s);
  
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
  for (int i = 0; i < s->num_elts; i++){
    printf("%d ", *((int *)s->elts + i));
  }
  printf("\n");
}

/**
   Runs a test example of a graph with integer weights.
*/
void run_int_graph_test(){
  graph_t g;
  adj_lst_t a;
  int_graph_init(&g);
  printf("Running directed int graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  print_int_adj_lst(&a);
  adj_lst_free(&a);
  printf("Running undirected int graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_int_adj_lst(&a);
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
   Prints adjacency list of a long double graph.
*/
void print_all_long_double_elts(stack_t *s);
  
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
  for (int i = 0; i < s->num_elts; i++){
    printf("%Lf ", *((long double *)s->elts + i));
  }
  printf("\n");
}

/**
   Runs a test example of a graph with long double weights.
*/
void run_long_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  long_double_graph_init(&g);
  printf("Running directed long double graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  print_long_double_adj_lst(&a);
  adj_lst_free(&a);
  printf("Running undirected long double graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_long_double_adj_lst(&a);
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
  g->wts = NULL;
  g->u = NULL;
  g->v = NULL;
}

/**
   Prints the adjacency list of a graph with no edges.
*/
void print_no_edges_adj_lst(adj_lst_t *a){
  printf("Print vertices... \n\n");
  for (int i = 0; i < a->num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a->vts[i]);
  }
  printf("\n");
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
  print_no_edges_adj_lst(&a);
  adj_lst_free(&a);
  printf("Running undirected graph with no edges test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  print_no_edges_adj_lst(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

int main(){
  run_int_graph_test();
  run_long_double_graph_test();
  run_no_edges_graph_test();
  return 0;
}
